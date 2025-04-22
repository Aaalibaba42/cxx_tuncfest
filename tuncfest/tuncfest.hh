#include <array>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

template <typename T>
concept HasConstexprName = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& name = T::name;
        static_assert(std::is_same_v<decltype(name), std::string_view const&>,
                      "The test does contain a name");
    }();
};

template <typename T>
concept HasConstexprInput = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& input = T::input;
        static_assert(std::is_same_v<decltype(input), std::string_view const&>,
                      "The test does contain a stdin");
    }();
};

template <typename T>
concept HasConstexprOutput = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& output = T::expected_output;
        static_assert(std::is_same_v<decltype(output), std::string_view const&>,
                      "The test does contain an expected stdout");
    }();
};

template <typename T>
concept HasConstexprExitCode = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        static_assert(
            std::is_same_v<std::remove_cvref_t<decltype(T::expected_exit_code)>,
                           int>,
            "The test does contain an expected exit code");
    }();
};

template <typename T>
concept TestCase = HasConstexprName<T> && HasConstexprInput<T>
    && HasConstexprOutput<T> && HasConstexprExitCode<T>;

// TODO Is this useful ?
template <typename T>
concept ValidTestCase = requires {
    []() static constexpr {
        static_assert(HasConstexprName<T>,
                      "TestCase is missing a valid `name` member.");
        static_assert(HasConstexprInput<T>,
                      "TestCase is missing a valid `input` member.");
        static_assert(HasConstexprOutput<T>,
                      "TestCase is missing a valid `expected_output` member.");
        static_assert(
            HasConstexprExitCode<T>,
            "TestCase is missing a valid `expected_exit_code` member.");
    }();
};

// TODO Add expected std_err and std::string command[] to pass to the binary
#define ADD_TEST(NAME, INPUT_STR, OUTPUT_STR, EXIT_CODE)                       \
    struct NAME                                                                \
    {                                                                          \
        static constexpr std::string_view name = #NAME;                        \
        static constexpr std::string_view input = INPUT_STR;                   \
        static constexpr std::string_view expected_output = OUTPUT_STR;        \
        static constexpr int expected_exit_code = EXIT_CODE;                   \
    };                                                                         \
    static_assert(ValidTestCase<NAME>,                                         \
                  "#NAME does not fulfill the TestCase requirements");

template <typename... Ts>
struct parameter_pack_size;

template <>
struct parameter_pack_size<>
{
    static constexpr std::size_t value = 0;
};

template <typename T, typename... Ts>
struct parameter_pack_size<T, Ts...>
{
    static constexpr std::size_t value = parameter_pack_size<Ts...>::value + 1;
};

template <char const* BinaryPath, TestCase... TestCases>
class FunctionalTestRunner
{
public:
    static void run_all_tests();
};

template <char const* BinaryPath, TestCase... TestCases>
void FunctionalTestRunner<BinaryPath, TestCases...>::run_all_tests()
{
    constexpr int MAX_EVENTS = 64;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return;
    }

    struct TestProcess
    {
        int stdout_fd;
        pid_t pid;
        std::ostringstream output;
        std::string_view expected_output;
        int expected_exit_code;
        std::string_view name;
    };

    std::array<TestProcess, parameter_pack_size<TestCases...>::value> processes;
    std::size_t process_count = 0;

    auto create_process = [&](auto test) -> TestProcess {
        int stdin_pipe[2], stdout_pipe[2];
        pipe(stdin_pipe);
        pipe(stdout_pipe);

        pid_t pid = fork();
        if (pid == 0)
        {
            dup2(stdin_pipe[0], STDIN_FILENO);
            dup2(stdout_pipe[1], STDOUT_FILENO);

            close(stdin_pipe[1]);
            close(stdout_pipe[0]);

            execl(BinaryPath, BinaryPath, nullptr);
            perror("execl");
            exit(127);
        }

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        write(stdin_pipe[1], test.input.data(), test.input.size());
        close(stdin_pipe[1]);

        fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);

        epoll_event ev{ .events = EPOLLIN, .data = { .fd = stdout_pipe[0] } };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdout_pipe[0], &ev);

        return TestProcess{ .stdout_fd = stdout_pipe[0],
                            .pid = pid,
                            .output = {},
                            .expected_output = test.expected_output,
                            .expected_exit_code = test.expected_exit_code,
                            .name = test.name };
    };

    auto spawn_all_tests = [&]() {
        ((processes[process_count++] = create_process(TestCases{})), ...);
    };

    auto collect_outputs = [&]() {
        std::array<char, 1024> buffer;
        int remaining = processes.size();

        while (remaining > 0)
        {
            epoll_event events[MAX_EVENTS];
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

            for (int i = 0; i < n; ++i)
            {
                int fd = events[i].data.fd;
                for (auto& proc : processes)
                {
                    if (proc.stdout_fd == fd)
                    {
                        ssize_t count = read(fd, buffer.data(), buffer.size());
                        if (count > 0)
                        {
                            proc.output.write(buffer.data(), count);
                        }
                        else if (count == 0)
                        {
                            close(fd);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                            remaining--;
                        }
                    }
                }
            }
        }
    };

    auto evaluate_results = [&]() {
        for (auto& proc : processes)
        {
            int status = 0;
            waitpid(proc.pid, &status, 0);

            int actual_exit = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            bool passed = (actual_exit == proc.expected_exit_code)
                && (proc.output.str() == proc.expected_output);

            std::cout << "[" << proc.name << "] "
                      << (passed ? "✅ PASS" : "❌ FAIL") << "\n";

            if (!passed)
            {
                std::cout << "  Expected output: \"" << proc.expected_output
                          << "\"\n"
                          << "  Actual output:   \""
                          << std::move(proc.output).str() << "\"\n"
                          << "  Expected exit: " << proc.expected_exit_code
                          << ", Actual: " << actual_exit << '\n';
            }
        }
    };

    spawn_all_tests();
    collect_outputs();
    evaluate_results();
    close(epoll_fd);
}
