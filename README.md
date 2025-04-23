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

Usage
-----

This is a single header library, you can just include the header "tuncfest.hh"
and you are good to go.

### Defining a Test

To add a Test, just invoke the ADD_TEST macro with the following arguments:
1. The test name
2. the stdin to pass to the test
3. the expected stdout
4. the expected return code

### Launching the tests

Just create a main function, and run the FunctionalTestRunner `run_all_tests()`
static function, passing the path of the program to test, and as many tests as
you want as variadic template parameters.

### Example

```cpp
#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

ADD_TEST(FirstTest, "42\n", "42\n", 0);
ADD_TEST(SecondTest, "43", "43", 0);

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
