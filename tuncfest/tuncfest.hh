#include <array>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

template <typename T>
concept HasConstexprName = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& test_name = T::test_name;
        static_assert(
            std::is_same_v<decltype(test_name), std::string_view const&>,
            "The test does contain a test_name");
    }();
};

template <typename T>
concept HasConstexprInput = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& stdinput = T::stdinput;
        static_assert(
            std::is_same_v<decltype(stdinput), std::string_view const&>,
            "The test does contain a stdinput");
    }();
};

template <typename T>
concept HasConstexprStdout = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& expected_stdout = T::expected_stdout;
        static_assert(
            std::is_same_v<decltype(expected_stdout), std::string_view const&>,
            "The test does contain an expected_stdout");
    }();
};

template <typename T>
concept HasConstexprStderr = requires {
    requires std::is_constant_evaluated();
    []() static constexpr {
        auto& expected_stderr = T::expected_stderr;
        static_assert(
            std::is_same_v<decltype(expected_stderr), std::string_view const&>,
            "The test does contain an expected_stderr");
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
concept TestCase =
    HasConstexprName<T> && HasConstexprInput<T> && HasConstexprStdout<T>
    && HasConstexprStderr<T> && HasConstexprExitCode<T>;

/*
    TODO Change this macro to a builder kind of pattern like:
    addTest(char *TestName)
        .with_input(char *input)
        .with_expected_output(expected_output)
    etc

    It makes fields optional and the framework less verbose
*/

// string_views are not directly usable in templates, making our own
template <size_t N>
struct sv
{
    char value[N];

    constexpr sv(char const (&str)[N])
    {
        for (size_t i = 0; i < N; ++i)
            value[i] = str[i];
    }

    constexpr operator std::string_view() const
    {
        return std::string_view(value, N - 1);
    }
};

// TODO Add std::string command[] to pass to the binary
// TODO Add timeout after which we kill the process
template <typename Name, sv StdInput = "", sv StdOut = "", sv StdErr = "",
          int ExitCode = 0>
struct TestBuilder
{
    using name = Name;

    template <sv NewInput>
    constexpr auto with_stdinput() const
    {
        return TestBuilder<Name, NewInput, StdOut, StdErr, ExitCode>{};
    }

    template <sv NewOut>
    constexpr auto with_expected_stdout() const
    {
        return TestBuilder<Name, StdInput, NewOut, StdErr, ExitCode>{};
    }

    template <sv NewErr>
    constexpr auto with_expected_stderr() const
    {
        return TestBuilder<Name, StdInput, StdOut, NewErr, ExitCode>{};
    }

    template <int NewExit>
    constexpr auto with_expected_exit_code() const
    {
        return TestBuilder<Name, StdInput, StdOut, StdErr, NewExit>{};
    }

    // Emit the actual struct
    struct Result
    {
        static constexpr std::string_view test_name = Name::value;
        static constexpr std::string_view stdinput = StdInput;
        static constexpr std::string_view expected_stdout = StdOut;
        static constexpr std::string_view expected_stderr = StdErr;
        static constexpr int expected_exit_code = ExitCode;
    };
};

#define WITH_INPUT(val) .with_stdinput<sv(val)>()
#define WITH_EXPECTED_STDOUT(val) .with_expected_stdout<sv(val)>()
#define WITH_EXPECTED_STDERR(val) .with_expected_stderr<sv(val)>()
#define WITH_EXPECTED_EXIT_CODE(code) .with_expected_exit_code<code>()

#define ADD_TEST(TEST_NAME, BUILDER)                                           \
    struct TEST_NAME##_Tag                                                     \
    {                                                                          \
        static constexpr std::string_view value = #TEST_NAME;                  \
    };                                                                         \
    using TEST_NAME = decltype(addTest<TEST_NAME##_Tag>() BUILDER)::Result

template <typename Name>
constexpr auto addTest()
{
    return TestBuilder<Name>{};
}

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
    static constexpr std::size_t value = 1 + parameter_pack_size<Ts...>::value;
};

// ostringstreams are notoriously heavy, this should be substancially
// quicker
struct OutputBuffer
{
    char data[4096];
    std::size_t size = 0;

    void append(char const* src, std::size_t len)
    {
        if (size + len < sizeof(data))
        {
            std::memcpy(data + size, src, len);
            size += len;
        }
    }

    std::string_view view() const
    {
        return { data, size };
    }
};

struct RuntimeProcess
{
    int stdout_fd;
    int stderr_fd;
    pid_t pid;
    OutputBuffer stdout_buff;
    OutputBuffer stderr_buff;
};

struct StaticProcessData
{
    std::string_view test_name;
    std::string_view stdinput;
    std::string_view expected_stdout;
    std::string_view expected_stderr;
    int expected_exit_code;
};

template <char const* BinaryPath, TestCase... Tests>
struct FunctionalTestRunner
{
    // Number of tests passed to this template instantiation
    static constexpr std::size_t NumTests =
        parameter_pack_size<Tests...>::value;

    // Prefill metadata for the tests in comptime, since these are available
    static constexpr std::array<StaticProcessData, NumTests> metadata = {
        { StaticProcessData{ Tests::test_name, Tests::stdinput,
                             Tests::expected_stdout, Tests::expected_stderr,
                             Tests::expected_exit_code }... }
    };

    // Main function for the runner
    static void run_all_tests()
    {
        constexpr int MAX_EVENTS = 64;
        // Epoll to let us run the tests in parallel while collecting I/O
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            perror("epoll_create1");
            return;
        }

        // Start every test in parallel
        std::array<RuntimeProcess, NumTests> processes;
        for (std::size_t i = 0; i < NumTests; ++i)
        {
            // Piping stdin and stdout (TODO stderr)
            int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
            pipe(stdin_pipe);
            pipe(stdout_pipe);
            pipe(stderr_pipe);

            // New process
            pid_t pid = fork();
            if (pid == 0)
            {
                // Link the pipes in the child
                dup2(stdin_pipe[0], STDIN_FILENO);
                dup2(stdout_pipe[1], STDOUT_FILENO);
                dup2(stderr_pipe[1], STDERR_FILENO);
                close(stdin_pipe[1]);
                close(stdout_pipe[0]);
                close(stderr_pipe[0]);
                // TODO Command arguments would go here once implemented
                execl(BinaryPath, BinaryPath, nullptr);
                perror("execl");
                _exit(127);
            }

            // In the parent, close our side of the pipe
            close(stdin_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);
            // Send the stdin data through the pipe
            write(stdin_pipe[1], metadata[i].stdinput.data(),
                  metadata[i].stdinput.size());
            // Close the stdin pipe
            close(stdin_pipe[1]);

            // Make the stdout nonblocking
            fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
            // Make the stderr nonblocking
            fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

            // Subscribe the process's stdout to the epoll
            epoll_event ev_out{ .events = EPOLLIN,
                                .data = { .fd = stdout_pipe[0] } };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdout_pipe[0], &ev_out);

            // Subscribe the process's stderr to the epoll
            epoll_event ev_err{ .events = EPOLLIN,
                                .data = { .fd = stderr_pipe[0] } };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stderr_pipe[0], &ev_err);

            // Fill the runtime struct
            processes[i] =
                RuntimeProcess{ stdout_pipe[0], stderr_pipe[0], pid, {}, {} };
        }

        // Let the tests run while collecting the data
        std::array<char, 1024> buffer;
        std::size_t remaining = NumTests * 2;
        // While tests are still running
        while (remaining > 0)
        {
            // Get everything that happened
            epoll_event events[MAX_EVENTS];
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

            for (int i = 0; i < n; ++i)
            {
                int fd = events[i].data.fd;
                for (std::size_t j = 0; j < NumTests; ++j)
                {
                    // For each event, find the corresponding process
                    if (processes[j].stdout_fd == fd)
                    {
                        // Either we collect new stdout, or the process has
                        // finished
                        ssize_t count = read(fd, buffer.data(), buffer.size());
                        if (count > 0)
                        {
                            processes[j].stdout_buff.append(buffer.data(),
                                                            count);
                        }
                        else if (count == 0)
                        {
                            close(fd);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                            --remaining;
                        }
                        break;
                    }
                    else if (processes[j].stderr_fd == fd)
                    {
                        // Either we collect new stderr, or the process has
                        // finished
                        ssize_t count = read(fd, buffer.data(), buffer.size());
                        if (count > 0)
                        {
                            processes[j].stderr_buff.append(buffer.data(),
                                                            count);
                        }
                        else if (count == 0)
                        {
                            close(fd);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                            --remaining;
                        }
                        break;
                    }
                }
            }
        }

        // Display The Tests Results
        // TODO More pretty and more info
        for (std::size_t i = 0; i < NumTests; ++i)
        {
            int status = 0;
            waitpid(processes[i].pid, &status, 0);
            int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

            bool passed = exit_code == metadata[i].expected_exit_code
                && processes[i].stdout_buff.view()
                    == metadata[i].expected_stdout
                && processes[i].stderr_buff.view()
                    == metadata[i].expected_stderr;

            std::cout << "[" << metadata[i].test_name << "] "
                      << (passed ? "✅ PASS" : "❌ FAIL") << "\n";

            if (!passed)
            {
                std::cout << "  Expected stdout: \""
                          << metadata[i].expected_stdout << "\"\n"
                          << "  Actual stdout:   \""
                          << processes[i].stdout_buff.view() << "\"\n"
                          << "  Expected stderr: \""
                          << metadata[i].expected_stderr << "\"\n"
                          << "  Actual stderr:   \""
                          << processes[i].stderr_buff.view() << "\"\n"
                          << "  Expected exit: "
                          << metadata[i].expected_exit_code
                          << ", Actual: " << exit_code << '\n';
            }
        }

        // We are done (Yay \o/)
        close(epoll_fd);
    }
};
