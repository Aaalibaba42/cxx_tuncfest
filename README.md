Tunctional Festing
==================

What ?
------

Tuncfest is a Functional Testing framework in modern C++. You can easiely define
tests with custom stdin and command line arguments, and define expected stdout,
stderr, and exit code. Everything but launching the tests and displaying the
results is done at compile time, so the overhead is minimal, and every test in a
Testsuite is launched in parallel.

Why ?
-----

I was looking for a C++ native Functional Testing framework and found none that
I was happy with, so I decided to make my own.

There are plenty of Unit testing framework that are great, I won't try to do
better than them, but weirdly enough, functional tests of external binaries
(C++ or otherwise) don't seem to exist in modern and simple C++. Plus, it is
very easy to [integrate to your projects!](#integration) In particular, easy
integration with CMake (and especially CMake's FetchContent) was of paramount
importance to me.

Usage
-----

### Glossary

- A `TestBuilder` is a reusable object that is able _actualize_ Tests based on
  how the Builder is setup at the moment of actualization.
- A `Test` is an immutable object that has been _actualized_ from a TestBuilder
  and that can be run by a TestRunner.
- A `TestRunner` is the class that is able to run any number of Tests in
  parallel on a given path to an executable program.
- To actualize a `TestBuilder` to a `Test`, you **Register** them.

### TestBuilder

A TestBuilder is a templated class with the following template parameters:

1. Test name: String (Default = "")
2. Stdin passed to the program: String (Default = "")
3. Stdout Validation: `bool (*)(std::string_view)` (Default =
`[](std::string_view) { return true; })`
4. Stderr Validation: `bool (*)(std::string_view)` (Default =
`[](std::string_view) { return true; })`
5. Exit Code Validation: `bool (*)(int)` (Default = `[](int) { return true; }`)
6. Variadic command line arguments: String...

The TestBuilder has a compile time template fluent interface builder pattern
that let you change any of these individually. The method to change the
parameters individually are:

Direct setters:
- with_name<"TestName">()
- with_stdinput<"Input">()
- with_command_line<"--optionName", "-o", "output.xml">()
- with_stdout_validation<funcptr>()
- with_expected_stderr<funcptr>()
- with_expected_exit_code<funcptr>()

Tip: don't forget these last 3 need *function pointers*, so you can either
declare functions and pass them, or use **captureless** lambdas (inlined or in
a variable) since captureless lambdas are implicitely convertible to function
pointers; you can also use the `+lambda` syntax to explicitely convert lambdas
to function pointers.

I have also devised helper methods to instantiate lambdas for the user for the most common cases:
- with_stdout_match<"Expected Stdout">
- with_stderr_match<"Expected Stderr">
- with_exit_code_match<0>

Thus, the *advised* way of declaring a Builder is:

```cpp
constexpr auto test_builder_1 = TestBuilder<"FirstTest">()
                                  .with_stdout_match<"test\n">()
                                  .with_command_line<"test">();
// Or to be even more explicit
constexpr auto test_builder_2 =
    TestBuilder<>()
        .with_name<"SecondTest">()
        .with_command_line<"much", "arguments">()
        .with_stdout_validation<[](std::string_view got) -> bool {
            auto nb_spaces = std::count_if(
                got.begin(), got.end(), [](char c) { return std::isspace(c); });
            return nb_spaces == 2;
        }>();
```

As you may notice, the order of the setter methods is free. You can also
factorize test attributes:

```cpp
// Maybe a better example would be pre-setting the command line argument and
// exit code corresponding to a feature you want a test but whatever, the
// point is to show that you are free to do stuff like this.

constexpr auto default_sucess = TestBuilder<>()
                                    .with_exit_code_match<0>();

constexpr auto default_failure = TestBuilder<>()
                                     .with_exit_code_match<1>()
                                     .with_stderr_match<"Failed\n">();

// "Inherit" the default_sucess parameters
constexpr auto success_test1 = default_sucess
                                   .with_name<"Newline">()
                                   .with_stdout_match<"38\n">();
constexpr auto success_test2 = default_sucess
                                   .with_name<"NoNewline">()
                                   .with_command_line<"--no_newline">()
                                   .with_stdout_match<"38">();

// "Inherits" the default_failure parameters
constexpr auto failing_test1 = default_failure
                                   .with_name<"UnrecognizedOption">()
                                   .with_command_line<"--doesnt_exist">();
```

NOTE: You can base a new TestBuilder from an existing one, but you **cannot**
use copy assignment operators between TestBuilders for 2 reasons:
- We aim for a constexpr context, and the copy assignment operator ought to be
  on a mutable *this*,
- TestBuilder methods actually creates a new type every time, since the tests
  are embedded in the template parameters of the class, so assigning a new
  TestBuilder to an existing variable would be changing its type, which is a
  big no-no.

### Registering a Test

You have a handy macro `REGISTER_TEST(TestName, Builder)` that will do
everything for you. The `TestName` here is the symbol corresponding to the Test,
how the test will be displayed by the testsuite is still the one parametrized
in the builder. Registering a Test has no influence on the builder, or the other
tests that were registered by the builder.

```cpp
constexpr auto test_builder1 = TestBuilder<>()./* with stuff */();
REGISTER_TEST(FirstTest, test_builder1);

constexpr auto test_builder2 = test_builder1./* with stuff */();
REGISTER_TEST(SecondTest, test_builder2);
```

Careful: The `TestName` is a typename, not an object.

### Launching a Testsuite

Once you have created the Tests you wanted, you can run them in parallel in
a TestRunner by using the `run_all_tests` static method of the specialized
TestRunner containing a path to an executable and a variadic number of Tests
to run:

```cpp
int main(void)
{
    constexpr char const binPath[] = "/usr/bin/echo";
    TestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
```

### Full Example

```cpp
#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

constexpr auto FirstTestBuilder = TestBuilder<"FirstTest">()
                                      .with_stdinput<"42">()
                                      .with_stdout_match<"42">();

constexpr auto SecondTestBuilder = TestBuilder<"SecondTest">()
                                       .with_stdinput<"43\n">()
                                       .with_stdout_match<"43\n">();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
```

You can find lots more examples in the [sample section.](samples)

Integration
-----------

It was thunk to be easy to integrate with whatever you are doing. You only
need a C++ compiler with c++23 support. No external library, no complex build
tools, nothing. Just compile, and you have a runnable testsuite. Literally just
a header.

CMake integration was also something important since the beginning of the
project since it is what lead me to look for a C++ functional testing framework.
You can integrate it to your project automagically with CMake's FetchContent,
like so:

```cmake
# Import the lib
include(FetchContent)
FetchContent_Declare(
    tuncfest
    GIT_REPOSITORY https://github.com/Aaalibaba42/cxx_tuncfest.git
    GIT_TAG main # Or select a specific hash/tag here for stability
)
FetchContent_MakeAvailable(tuncfest)

# Link it to your testsuite
add_executable(functional_tests functional_tests.cc)
target_link_libraries(functional_tests PRIVATE tuncfest)

# OPTIONAL launch the testsuite directly from CMake
add_custom_command(
    TARGET functional_tests
    POST_BUILD
    COMMAND $<TARGET_FILE:functional_tests>
)
# If you do, you may want to explicitely add the dependancy
add_dependencies(functional_tests your_target)
```

You may see it in action [here](samples/simple/CMakeLists.txt).

Performance
-----------

Since almost everything is done at compile time, the runtime overhead should
be negligeable. At runtime, almost nothing happens before starting to fork and
pipe to run the tests in parallel. I/O is synchronized with the kernel's epoll,
so runtime performance should also be excellent. Compilation time is *very*
reasonable considering everything that is happening. The compilation time of the
[heavy sample](samples/heavy/), which contains 60 different tests, takes about 2
seconds on boths clang and gcc on my laptop:

```
42sh$ time g++ -std=c++23 heavy.cc -I../../tuncfest/

real	0m2.182s
user	0m2.027s
sys	0m0.115s
42sh$ time clang++ -std=c++23 heavy.cc -I../../tuncfest/

real	0m1.995s
user	0m1.786s
sys	0m0.161s
```

Looks
-----

Looks was not paramount to me, but I still made an effort to make the output as
pretty as I can manage:

https://github.com/user-attachments/assets/960b2a86-9515-4325-af1e-127f99e9b6d4

Note: the time to compile is way quicker than I said above because the cache was
hot since I had compiled this just before (when cache was very very hot, I got
under 1s with this example).

Pitfalls
--------

Functional tests in parallel can be perilous, if for example the program has
side effects that are in conflict with each others (or well, itself), but I'll
ignore it for now.

I think I will leave it to the user to understand that every test in a testsuite
run in parallel. They can do several testsuites if this is not their expected
behavior.

Why C++ for Functional Tests ?
------------------------------

Short Answer: I like C++, and when I code, I want to enjoy it. I don't like
writing shell scripts; I enjoy writing C++.

Longer Answer: Modern C++ (and even more so when C++26's introspection features
arrive) has interesting tools to make writing a functional test suite fast and
safe. The type system and concepts with metaprogramming will allow this tool
(hopefully) to emit diagnostics when the instantiation of a test fails, and
sanitize input and output much better than a shell script could. The same goes
for parallelism; while it's doable in POSIX shells to have parallel command
execution with an internal state to report failures and successes, I honestly
wouldn't want to go through the trouble. In C++, it comes naturally; and that's
not even mentioning exporting the test suite results in XML or whatever other
fucked up format the industry can come up with.

The main thing that made me do this is that I wanted to have functional tests
for a C++ tool I'm developing which interacts with I/O, and I needed it to
integrate well with my C++ build system. It would have been ridiculous to have
CMake call an external build tool to build the test suite or something like
that. I did not found a solution so I'm making it.

This does not work on Windows ?
-------------------------------

I don't know the windows APIs, and don't plan on learning them anytime soon.
