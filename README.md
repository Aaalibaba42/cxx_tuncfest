Tunctional Festing
==================

I was looking for a C++ native Functional Testing framework and found none that
I was happy with, so I decided to make my own.

There are plenty of Unit testing framework that are great, I won't try to do
better than them, but weirdly enough, functional tests of external binaries (C++
or otherwise) don't seem to exist in modern and simple C++.

Objective
---------

Basicly being able to functionally test external binaries in parallel and
comparing stdout/stderr and the exit code with simple C++ code.

As a bonus, I've leveraged most of what modern C++ can offer, so most things are
done very efficiently at compile time. Basicly the only thing left at run time
is to launch the pipes and subprocesses and compare the outputs.

Usage
-----

This is a single header library, you can just include the header "tuncfest.hh"
and you are good to go.

### Defining a Test

To add a Test, you first need to create a TestBuilder with the handy
`testBuilder("TestName")` macro. It is some kind of a builder pattern, you can
change the Test settings by chaining the following methods:
- with_stdinput<sv("Input")>()
- with_expected_stdout<sv("Output")>()
- with_expected_stderr<sv("Error output")>()
- with_expected_exit_code<0>()

Careful: sv("string") is an internal wrapper on std::string_view, since
string_views can't instantiate templates in c++23. They are necessary, and you
will have nasty errors if you don't use them.

You can then register a Test (basicly "realizing" the builder), with the
`REGISTER_TEST(TestName, TestBuilderName)` macro.

Partially specialized Builders should be able to be reused, but this isn't
tested yet so I won't say it is a feature.

### Launching the tests

Just create a main function, and run the FunctionalTestRunner `run_all_tests()`
static function, passing the path of the program to test, and as many tests as
you want (and have registered) as variadic template parameters.

### Example

```cpp
#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

constexpr auto FirstTestBuilder = testBuilder("FirstTest")
                                      .with_stdinput<sv("42")>()
                                      .with_expected_stdout<sv("42")>();

constexpr auto SecondTestBuilder = testBuilder("SecondTest")
                                       .with_stdinput<sv("43\n")>()
                                       .with_expected_stdout<sv("43\n")>();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
```

Pitfalls
--------

Functional tests in parallel can be perilous, if for example the program has
side effects that are in conflict with each others, but I'll ignore it for now.

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
