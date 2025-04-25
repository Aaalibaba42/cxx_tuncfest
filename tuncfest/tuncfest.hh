#include <array>
#include <cstring>
#include <vector>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

// Pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

namespace VariadicTemplatedTypesCounting
{
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
        static constexpr std::size_t value =
            1 + parameter_pack_size<Ts...>::value;
    };
} // namespace VariadicTemplatedTypesCounting
using VariadicTemplatedTypesCounting::parameter_pack_size;

namespace TestFormValidation
{
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
            static_assert(std::is_same_v<decltype(expected_stdout),
                                         std::string_view const&>,
                          "The test does contain an expected_stdout");
        }();
    };

    template <typename T>
    concept HasConstexprStderr = requires {
        requires std::is_constant_evaluated();
        []() static constexpr {
            auto& expected_stderr = T::expected_stderr;
            static_assert(std::is_same_v<decltype(expected_stderr),
                                         std::string_view const&>,
                          "The test does contain an expected_stderr");
        }();
    };

    template <typename T>
    concept HasConstexprExitCode = requires {
        requires std::is_constant_evaluated();
        []() static constexpr {
            static_assert(
                std::is_same_v<
                    std::remove_cvref_t<decltype(T::expected_exit_code)>, int>,
                "The test does contain an expected exit code");
        }();
    };

    template <typename T>
    concept TestCase =
        HasConstexprName<T> && HasConstexprInput<T> && HasConstexprStdout<T>
        && HasConstexprStderr<T> && HasConstexprExitCode<T>;
} // namespace TestFormValidation
// Concept that verifies something adheres to the prototype of a test.
using TestFormValidation::TestCase;

namespace HackyWrappers
{
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

    // ostringstreams are notoriously heavy, this should be substancially
    // quicker
    // TODO 4096
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

} // namespace HackyWrappers
// std::string_views cannot directly be used in template instantiation
using HackyWrappers::sv;
// ostringstreams are heavy, this should be less so
using HackyWrappers::OutputBuffer;

namespace TestBuilderClass
{
    // TODO Add std::string command[] to pass to the binary
    // TODO Add timeout after which we kill the process
    template <typename Name, sv StdInput = "", sv StdOut = "", sv StdErr = "",
              int ExitCode = 0, sv... CmdLineArgs>
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

        template <sv... NewArgs>
        constexpr auto with_command_line() const
        {
            return TestBuilder<Name, StdInput, StdOut, StdErr, ExitCode,
                               NewArgs...>{};
        }

        // Emit the actual struct
        struct Result
        {
            static constexpr std::string_view test_name = Name::value;
            static constexpr std::string_view stdinput = StdInput;
            static constexpr std::string_view expected_stdout = StdOut;
            static constexpr std::string_view expected_stderr = StdErr;
            static constexpr int expected_exit_code = ExitCode;

            static constexpr std::size_t command_line_argc =
                parameter_pack_size<decltype(CmdLineArgs)...>::value;
            static constexpr std::array<std::string_view, command_line_argc>
                command_line_argv = { CmdLineArgs... };
        };
    };

    template <sv Name>
    struct MakeTagImpl
    {
        struct type
        {
            static constexpr sv value = Name;
        };
    };

    template <sv Str>
    using MakeTag = MakeTagImpl<Str>;

    template <typename Name>
    constexpr auto addTest()
    {
        return TestBuilder<Name>{};
    }

#define REGISTER_TEST(NAME, BUILDER) using NAME = decltype(BUILDER)::Result

#define testBuilder(TESTNAME_LITERAL)                                          \
    TestBuilder<MakeTag<TESTNAME_LITERAL>::type>()
} // namespace TestBuilderClass
using TestBuilderClass::TestBuilder;
using TestBuilderClass::MakeTag;
using TestBuilderClass::addTest;

namespace Runner
{
    // Not inferable in comptime
    struct RuntimeProcess
    {
        int stdout_fd;
        int stderr_fd;
        pid_t pid;
        OutputBuffer stdout_buff;
        OutputBuffer stderr_buff;
    };

    // Should be all filled at comptime
    struct StaticProcessData
    {
        std::string_view test_name;
        std::string_view stdinput;
        std::string_view expected_stdout;
        std::string_view expected_stderr;
        int expected_exit_code;

        std::size_t command_line_argc;
        std::string_view const* command_line_argv;
    };

    inline int get_terminal_width()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return static_cast<int>(w.ws_col);
    }

    template <char const* BinaryPath, TestCase... Tests>
    struct FunctionalTestRunner
    {
        // Number of tests passed to this template instantiation
        static constexpr std::size_t NumTests =
            parameter_pack_size<Tests...>::value;

        // Prefill metadata for the tests in comptime, since these are available
        static constexpr std::array<StaticProcessData, NumTests> metadata = {
            { StaticProcessData{
                Tests::test_name, Tests::stdinput, Tests::expected_stdout,
                Tests::expected_stderr, Tests::expected_exit_code,
                Tests::command_line_argc, Tests::command_line_argv.data() }... }
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

                    // TODO make constexpr
                    std::vector<char const*> argv;
                    argv.push_back(BinaryPath);
                    for (std::size_t j = 0; j < metadata[i].command_line_argc;
                         ++j)
                    {
                        argv.push_back(metadata[i].command_line_argv[j].data());
                    }
                    argv.push_back(nullptr);
                    // ODOT
                    execv(BinaryPath, const_cast<char* const*>(argv.data()));
                    perror("execv");
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
                processes[i] = RuntimeProcess{
                    stdout_pipe[0], stderr_pipe[0], pid, {}, {}
                };
            }

            // Let the tests run while collecting the data
            std::array<char, 1024> buffer;
            std::size_t remaining = NumTests * 2;
            auto gradient_bar =
                [](std::size_t total, std::size_t left,
                   std::tuple<int, int, int> start_color = { 227, 52, 0 },
                   std::tuple<int, int, int> end_color = { 92, 204, 150 }) {
                    int width = get_terminal_width();
                    int bar_width = width - 20;
                    float progress = 1.f - static_cast<float>(left) / total;
                    int filled = static_cast<int>(bar_width * progress);

                    auto [r1, g1, b1] = start_color;
                    auto [r2, g2, b2] = end_color;

                    std::cout << "\r[";
                    for (int i = 0; i < bar_width; ++i)
                    {
                        double ratio = static_cast<double>(i) / bar_width;
                        int r = static_cast<int>(r1 + (r2 - r1) * ratio);
                        int g = static_cast<int>(g1 + (g2 - g1) * ratio);
                        int b = static_cast<int>(b1 + (b2 - b1) * ratio);
                        std::cout
                            << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                            << (i < filled ? "█"
                                           : (i < (filled * 1.15f) ? "░" : " "))
                            << "\033[0m";
                    }
                    std::cout << "] " << std::setw(3)
                              << static_cast<int>(progress * 100) << "%"
                              << std::flush;
                };

            // While tests are still running
            while (remaining > 0)
            {
                gradient_bar(NumTests * 2, remaining - 1);
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
                            ssize_t count =
                                read(fd, buffer.data(), buffer.size());
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
                            ssize_t count =
                                read(fd, buffer.data(), buffer.size());
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

            for (std::size_t i = 0; i < NumTests; ++i)
            {
                int status = 0;
                waitpid(processes[i].pid, &status, 0);
                int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

                auto const& expected_stdout = metadata[i].expected_stdout;
                auto const& actual_stdout = processes[i].stdout_buff.view();
                auto const& expected_stderr = metadata[i].expected_stderr;
                auto const& actual_stderr = processes[i].stderr_buff.view();
                int expected_exit = metadata[i].expected_exit_code;

                bool passed = exit_code == expected_exit
                    && actual_stdout == expected_stdout
                    && actual_stderr == expected_stderr;

                std::cout << BOLD << "[" << metadata[i].test_name << "] "
                          << (passed ? GREEN "✅ PASS" : RED "❌ FAIL") << RESET
                          << "\n";

                if (!passed)
                {
                    auto print_diff = [](auto const& label,
                                         auto const& expected,
                                         auto const& actual) static {
                        std::cout << YELLOW << "  " << label << ":\n" << RESET;
                        if (expected != actual)
                        {
                            std::cout << "    - Expected: " << GREEN << "\""
                                      << expected << "\"" << RESET << "\n"
                                      << "    + Actual:   " << RED << "\""
                                      << actual << "\"" << RESET << "\n";
                        }
                        else
                        {
                            std::cout << "    " << GREEN << "(match)" << RESET
                                      << "\n";
                        }
                    };

                    print_diff("Stdout", expected_stdout, actual_stdout);
                    print_diff("Stderr", expected_stderr, actual_stderr);

                    std::cout << YELLOW << "  Exit Code:\n" << RESET;
                    if (exit_code != expected_exit)
                    {
                        std::cout << "    - Expected: " << GREEN
                                  << expected_exit << RESET << "\n"
                                  << "    + Actual:   " << RED << exit_code
                                  << RESET << "\n";
                    }
                    else
                    {
                        std::cout << "    " << GREEN << "(match)" << RESET
                                  << "\n";
                    }
                }

                std::cout << std::string(60, '-') << "\n";
            }

            // We are done (Yay \o/)
            close(epoll_fd);
        }
    };
} // namespace Runner
using Runner::FunctionalTestRunner;
