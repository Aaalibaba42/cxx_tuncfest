#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <cstring>

#define ADD_TEST(NAME, INPUT_STR, OUTPUT_STR, EXIT_CODE)                       \
    struct NAME                                                                \
    {                                                                          \
        static constexpr std::string_view name = #NAME;                        \
        static constexpr std::string_view input = INPUT_STR;                   \
        static constexpr std::string_view expected_output = OUTPUT_STR;        \
        static constexpr int expected_exit_code = EXIT_CODE;                   \
    };

// TODO instead of TestCases, make a concept for better checking and error
// messages
//
// TODO Lots of refactoring, this is very monolithic
//
template <char const* BinaryPath, typename... TestCases>
struct FunctionalTestRunner
{
    static void run_all_tests()
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
            std::string output;
            std::string expected_output;
            int expected_exit_code;
            std::string name;
        };

        std::vector<TestProcess> processes;

        // Lambda to fork and run a test
        auto spawn_test = [&](auto test) {
            int stdin_pipe[2];
            int stdout_pipe[2];
            pipe(stdin_pipe);
            pipe(stdout_pipe);

            pid_t pid = fork();
            if (pid == 0)
            {
                // Child
                dup2(stdin_pipe[0], STDIN_FILENO);
                dup2(stdout_pipe[1], STDOUT_FILENO);
                close(stdin_pipe[1]);
                close(stdout_pipe[0]);

                execl(BinaryPath, BinaryPath, nullptr);
                perror("execl");
                exit(127);
            }
            else
            {
                // Parent
                close(stdin_pipe[0]);
                close(stdout_pipe[1]);

                // Write input
                write(stdin_pipe[1], test.input.data(), test.input.size());
                close(stdin_pipe[1]);

                fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);

                epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = stdout_pipe[0];
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdout_pipe[0], &ev);

                processes.push_back(
                    { .stdout_fd = stdout_pipe[0],
                      .pid = pid,
                      .output = "",
                      .expected_output = std::string(test.expected_output),
                      .expected_exit_code = test.expected_exit_code,
                      .name = std::string(test.name) });
            }
        };

        // Spawn all tests
        (spawn_test(TestCases{}), ...);

        std::array<char, 1024> buf;
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
                        ssize_t count = read(fd, buf.data(), buf.size());
                        if (count > 0)
                        {
                            proc.output.append(buf.data(), count);
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

        // Collect and evaluate results
        for (auto& proc : processes)
        {
            int status = 0;
            waitpid(proc.pid, &status, 0);
            bool success = true;

            int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

            if (exit_code != proc.expected_exit_code)
            {
                success = false;
            }

            if (proc.output != proc.expected_output)
            {
                success = false;
            }

            std::cout << "[" << proc.name << "] "
                      << (success ? "✅ PASS" : "❌ FAIL") << "\n";

            if (!success)
            {
                std::cout << "  Expected output: \"" << proc.expected_output
                          << "\"\n";
                std::cout << "  Actual output:   \"" << proc.output << "\"\n";
                std::cout << "  Expected exit: " << proc.expected_exit_code
                          << ", Actual: " << exit_code << "\n";
            }
        }

        close(epoll_fd);
    }
};
