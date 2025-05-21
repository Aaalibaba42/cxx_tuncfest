#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <tuple>
#include <type_traits>
#include <unistd.h>

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
        []() static consteval {
            auto& test_name = T::test_name;
            static_assert(
                std::is_same_v<decltype(test_name), std::string_view const&>,
                "The test does contain a test_name");
        }();
    };

    template <typename T>
    concept HasConstexprInput = requires {
        requires std::is_constant_evaluated();
        []() static consteval {
            auto& stdinput = T::stdinput;
            static_assert(
                std::is_same_v<decltype(stdinput), std::string_view const&>,
                "The test does contain a stdinput");
        }();
    };

    /*
    template <typename T>
    concept HasConstexprStdout = requires {
        requires std::is_constant_evaluated();
        []() static consteval {
            auto& expected_stdout = T::expected_stdout;
            static_assert(std::is_same_v<decltype(expected_stdout),
                                         std::string_view const&>,
                          "The test does contain an expected_stdout");
        }();
    };

    template <typename T>
    concept HasConstexprStderr = requires {
        requires std::is_constant_evaluated();
        []() static consteval {
            auto& expected_stderr = T::expected_stderr;
            static_assert(std::is_same_v<decltype(expected_stderr),
                                         std::string_view const&>,
                          "The test does contain an expected_stderr");
        }();
    };

    template <typename T>
    concept HasConstexprExitCode = requires {
        requires std::is_constant_evaluated();
        []() static consteval {
            static_assert(
                std::is_same_v<
                    std::remove_cvref_t<decltype(T::expected_exit_code)>, int>,
                "The test does contain an expected exit code");
        }();
    };
    */

    template <typename T>
    concept HasConstexprCommandLineArgs = requires {
        std::is_constant_evaluated();
        []() static consteval {
            auto& argc = T::command_line_argc;
            auto& argv = T::command_line_argv;

            static_assert(
                std::is_integral_v<std::remove_cvref_t<decltype(argc)>>,
                "The test does not contain an expected command_line_argc");
            static_assert(
                std::is_same_v<std::remove_cvref_t<decltype(argv)>,
                               std::array<char const*, argc>>,
                "The test does not contain the expected command_line_argv");
        }();
    };

    template <typename T>
    concept TestCase = HasConstexprName<T>
        && HasConstexprInput<T>
        // && HasConstexprStdout<T> && HasConstexprStderr<T>
        // && HasConstexprExitCode<T>
        && HasConstexprCommandLineArgs<T>;
} // namespace TestFormValidation
// Concept that verifies something adheres to the prototype of a test.
using TestFormValidation::TestCase;

namespace HackyWrappers
{
    // string_views are not directly usable in templates, making our own
    template <std::size_t N>
    struct sv
    {
        char value[N];

        consteval sv(char const (&str)[N])
        {
            for (std::size_t i = 0; i < N; ++i)
                value[i] = str[i];
        }

        consteval operator std::string_view() const
        {
            return std::string_view(value, N - 1);
        }
    };

    // Deduction guide
    template <std::size_t N>
    sv(char const (&)[N]) -> sv<N>;

    // ostringstreams are notoriously heavy, this should be substancially
    // quicker
    //
    // FIXME:
    //     I think the best solution since we are going to lambdas for
    //     validation would be to map to a file if the expected output is more
    //     than 4096, and fail the test if the output buffer for an expected
    //     output < 4096 reaches 4096
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
    // TODO Add timeout after which we kill the process
    // TODO Add callable validator for stdout/stderr/exit_code *instead* of
    //      "literal" comparison (maybe when implementing, convert literal
    //      comparison to lambda callable that compares to the lit)
    template <sv Name = "", sv StdInput = "",
              bool (*StdOut)(std::string_view) = [](std::string_view) -> bool {
                  return true;
              },
              bool (*StdErr)(std::string_view) = [](std::string_view) -> bool {
                  return true;
              },
              bool (*ExitCode)(int) = [](int) -> bool { return true; },
              sv... CmdLineArgs>
    struct TestBuilder
    {
        template <sv NewName>
        consteval auto with_name() const
        {
            return TestBuilder<NewName, StdInput, StdOut, StdErr, ExitCode,
                               CmdLineArgs...>{};
        }

        template <sv NewInput>
        consteval auto with_stdinput() const
        {
            return TestBuilder<Name, NewInput, StdOut, StdErr, ExitCode,
                               CmdLineArgs...>{};
        }

        template <bool (*NewOut)(std::string_view)>
        consteval auto with_expected_stdout() const
        {
            return TestBuilder<Name, StdInput, NewOut, StdErr, ExitCode,
                               CmdLineArgs...>{};
        }

        template <bool (*NewErr)(std::string_view)>
        consteval auto with_expected_stderr() const
        {
            return TestBuilder<Name, StdInput, StdOut, NewErr, ExitCode,
                               CmdLineArgs...>{};
        }

        template <bool (*NewExit)(int)>
        consteval auto with_expected_exit_code() const
        {
            return TestBuilder<Name, StdInput, StdOut, StdErr, NewExit,
                               CmdLineArgs...>{};
        }

        template <sv... NewArgs>
        consteval auto with_command_line() const
        {
            return TestBuilder<Name, StdInput, StdOut, StdErr, ExitCode,
                               NewArgs...>{};
        }

        // Emit the actual struct
        struct Result
        {
            static constexpr std::string_view test_name = Name;
            static constexpr std::string_view stdinput = StdInput;
            // TODO change name of these
            static constexpr bool (*expected_stdout)(std::string_view) = StdOut;
            static constexpr bool (*expected_stderr)(std::string_view) = StdErr;
            static constexpr bool (*expected_exit_code)(int) = ExitCode;

            static constexpr std::size_t command_line_argc =
                sizeof...(CmdLineArgs);
            static constexpr std::array<char const*, command_line_argc>
                command_line_argv = { CmdLineArgs.value... };
        };
    };

#define REGISTER_TEST(NAME, BUILDER) using NAME = decltype(BUILDER)::Result
} // namespace TestBuilderClass
using TestBuilderClass::TestBuilder;

namespace Runner
{

    namespace Output
    {
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

        static inline int get_terminal_width()
        {
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            return static_cast<int>(w.ws_col);
        }

        static inline void gradient_bar(
            std::size_t total, std::size_t left,
            std::tuple<unsigned char, unsigned char, unsigned char> const&
                start_color = { 227, 52, 0 },
            std::tuple<unsigned char, unsigned char, unsigned char> const&
                end_color = { 92, 204, 150 })
        {
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
                std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m"
                          << (i < filled ? "█"
                                         : (i < (filled * 1.15f) ? "░" : " "))
                          << "\033[0m";
            }
            std::cout << "] " << std::setw(3)
                      << static_cast<int>(progress * 100) << "%" << std::flush;
        };

        static inline void display_result(auto const& metadata,
                                          auto const& processes, std::size_t i)
        {
            int status = 0;
            waitpid(processes[i].pid, &status, 0);
            int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

            // TODO change name of the lambdas (things in metadata)
            auto& expected_stdout = metadata[i].expected_stdout;
            auto actual_stdout = processes[i].stdout_buff.view();
            auto& expected_stderr = metadata[i].expected_stderr;
            auto actual_stderr = processes[i].stderr_buff.view();
            auto& expected_exit = metadata[i].expected_exit_code;

            bool passed = expected_exit(exit_code)
                && expected_stdout(actual_stdout)
                && expected_stderr(actual_stderr);

            std::cout << BOLD << "[" << metadata[i].test_name << "] "
                      << (passed ? GREEN "✅ PASS" : RED "❌ FAIL") << RESET
                      << "\n";

            if (!passed)
            {
                auto print_diff = [](auto const& label, auto const& expected,
                                     auto const& actual) static {
                    std::cout << YELLOW << "  " << label << ":\n" << RESET;
                    if (!expected(actual))
                    {
                        std::cout << "    - Expected: " << GREEN << "\""
                                  << "<placeholder str>" << "\"" << RESET
                                  << "\n"
                                  << "    + Actual:   " << RED << "\"" << actual
                                  << "\"" << RESET << "\n";
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
                if (!expected_exit(exit_code))
                {
                    std::cout << "    - Expected: " << GREEN
                              << "<placeholder exit_code>" << RESET << "\n"
                              << "    + Actual:   " << RED << exit_code << RESET
                              << "\n";
                }
                else
                {
                    std::cout << "    " << GREEN << "(match)" << RESET << "\n";
                }
            }

            std::cout << std::string(60, '-') << "\n";
        }
    } // namespace Output
    using Output::get_terminal_width;
    using Output::gradient_bar;
    using Output::display_result;

    // Not inferable in comptime
    struct RuntimeProcess
    {
        OutputBuffer stdout_buff;
        OutputBuffer stderr_buff;

        pid_t pid;

        int stdout_fd;
        int stderr_fd;
    };

    template <char const* BinPath, TestCase Test>
    struct ArgvBuilder
    {
        static constexpr std::array<char const*, Test::command_line_argc + 2>
            value = []() static consteval {
                std::array<char const*, Test::command_line_argc + 2> r{};

                r[0] = BinPath;
                for (size_t i = 0; i < Test::command_line_argc; ++i)
                {
                    r[i + 1] = Test::command_line_argv[i];
                }
                r[r.size() - 1] = nullptr;

                return r;
            }();
    };

    // Should be all filled at comptime
    struct StaticProcessData
    {
        std::string_view test_name;
        std::string_view stdinput;
        bool (*expected_stdout)(std::string_view);
        bool (*expected_stderr)(std::string_view);

        char const* const* command_line_argv;
        std::size_t command_line_argc;

        bool (*expected_exit_code)(int);
    };

    template <char const* BinaryPath, TestCase... Tests>
    class TestRunner
    {
    private:
        // Number of tests passed to this template instantiation
        static constexpr std::size_t NumTests =
            parameter_pack_size<Tests...>::value;

        // Prefill metadata for the tests in comptime, since these are available
        static constexpr std::array<StaticProcessData, NumTests> metadata = {
            { StaticProcessData{ Tests::test_name, Tests::stdinput,
                                 Tests::expected_stdout, Tests::expected_stderr,
                                 ArgvBuilder<BinaryPath, Tests>::value.data(),
                                 Tests::command_line_argc,
                                 Tests::expected_exit_code }... }
        };

        // Function to set up pipes and fork a new process
        static inline void setup_process(int stdin_pipe[2], int stdout_pipe[2],
                                         int stderr_pipe[2], pid_t& pid,
                                         std::size_t i)
        {
            pipe(stdin_pipe);
            pipe(stdout_pipe);
            pipe(stderr_pipe);

            pid = fork();

            // New process
            if (pid == 0)
            {
                // Link the pipes in the child
                dup2(stdin_pipe[0], STDIN_FILENO);
                dup2(stdout_pipe[1], STDOUT_FILENO);
                dup2(stderr_pipe[1], STDERR_FILENO);
                close(stdin_pipe[1]);
                close(stdout_pipe[0]);
                close(stderr_pipe[0]);

                execv(BinaryPath,
                      const_cast<char* const*>(metadata[i].command_line_argv));
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
        }

        static inline void subscribe_to_epoll(int epoll_fd, int pipe[2])
        {
            epoll_event ev_out{ .events = EPOLLIN, .data = { .fd = pipe[0] } };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe[0], &ev_out);
        }

        static inline void handle_output(auto& buffer, int epoll_fd, int fd,
                                         auto& output_buff,
                                         std::size_t& remaining)
        {
            ssize_t count = read(fd, buffer.data(), buffer.size());
            if (count > 0)
            {
                output_buff.append(buffer.data(),
                                   static_cast<std::size_t>(count));
            }
            else if (count == 0)
            {
                close(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                --remaining;
            }
        }
        static inline void handle_event(int epoll_fd, auto& buffer,
                                        auto& processes, int fd,
                                        std::size_t& remaining)
        {
            for (auto& proc : processes)
            {
                if (proc.stdout_fd == fd)
                {
                    handle_output(buffer, epoll_fd, fd, proc.stdout_buff,
                                  remaining);
                    return;
                }
                else if (proc.stderr_fd == fd)
                {
                    handle_output(buffer, epoll_fd, fd, proc.stderr_buff,
                                  remaining);

                    return;
                }
            }
        };

        static inline void collect_processes(int epoll_fd, auto& processes)
        {
            constexpr int MAX_EVENTS = 64;
            std::array<char, 1024> buffer;
            std::size_t remaining = NumTests * 2;

            while (remaining > 0)
            {
                gradient_bar(NumTests * 2, remaining - 1);

                epoll_event events[MAX_EVENTS];
                int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

                for (int i = 0; i < n; ++i)
                {
                    handle_event(epoll_fd, buffer, processes, events[i].data.fd,
                                 remaining);
                }
            }

            gradient_bar(NumTests * 2, 0);
            std::cout << std::endl;
        }

    public:
        // Main function for the runner
        static void run_all_tests()
        {
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
                int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
                pid_t pid;

                setup_process(stdin_pipe, stdout_pipe, stderr_pipe, pid, i);

                subscribe_to_epoll(epoll_fd, stdout_pipe);
                subscribe_to_epoll(epoll_fd, stderr_pipe);

                // Fill the runtime struct
                processes[i] = RuntimeProcess{
                    {}, {}, pid, stdout_pipe[0], stderr_pipe[0]
                };
            }

            // Let the process run and collect the output
            collect_processes(epoll_fd, processes);

            std::cout << std::string(60, '-') << "\n";
            for (std::size_t i = 0; i < NumTests; ++i)
                display_result(metadata, processes, i);

            // We are done (Yay \o/)
            close(epoll_fd);
        }
    };
} // namespace Runner
using Runner::TestRunner;
