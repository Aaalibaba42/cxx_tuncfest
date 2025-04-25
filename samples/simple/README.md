Simple
======

Here lies a simple example of how you can create a functional testsuite using
this tool, as well as how to integrate it to your projects via CMake.

Code walkthrough
----------------

### Bin Path

This defines the binary we want to test. You can add any valid path, relative or
absolute.

```cpp
static char const binPath[] = "/usr/bin/cat";
```

It doesn't need to be a global variable as I did here, you can put it wherever
you like, but it *should* be available at compile time, in order to instantiate
templates. For example, this would also work:

```cpp
// ...
int main(void)
{
    static constexpr char binPath[] = "/usr/bin/cat";
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
```

### Test Builder

Then we define a tests Builder. Let's examine one:

```cpp
constexpr auto greatTestBuilder = testBuilder("GreatTest")
                                      .with_stdinput<"42\n">()
                                      .with_expected_stdout<"42\n">();
```

First, the testBuilder macro takes a string. That will be the Test's name.
TestObject instances have a compile time fluent interface (not exactly since the
final Test will be another type entirely but that's beyond the point) which let
you change the final test's parameters, such as the command line arguments, the
expected stdout of the test, the exit code, etc.

#### Going further

Same as before, the builders can be registered and created in the main function,
and with other qualifiers, but they need to be realizable in compile time.

You **should** keep the auto and not use the exact type of this, but
in case you ever need it, it would be an instance of something like
`TestBuilder<MakeTagImpl<sv("FirstTest")>::type, /* other parameters */>`.

### Registration

You then need to "register" a test:

```cpp
REGISTER_TEST(MyGreatTest, greatTestBuilder);
```

Basicly it creates a "Test object" with the given name, from the given builder.

#### Going further

The way it works under the hood is that TestBuilder object contain a nested
`Result` struct which has the template parameters of the TestBuilder as
constexpr static attributes. So each time you call a `with_xxx<...>()` function,
it instantiates a new template TestBuilder with a different nested Result
struct.

Thus, the REGISTER_TEST macro is simply:
```cpp
#define REGISTER_TEST(NAME, BUILDER) using NAME = decltype(BUILDER)::Result
```

That is why a builder can be instantiate many different tests in between
modifications. Each time you modify a Builder, it really creates a new builder
type with a different nested Result struct; and if every parameter is the same,
it automagically fall back to the already created type with the same template
parameters, so nothing is wasted, and everything is compile time.

### Running the Testsuite

Inside a main function, you need to run the run_all_tests() static function of
the FunctionalTestRunner class, with everything as template parameters. The 1st
template parameter needs to be the binary to test, every other parameters must
be *registered* tests. You can put as much of them as you need.

```cpp
int main(void)
{
    FunctionalTestRunner<binPath, MyGreatTest>::run_all_tests();
}
```

Running the Testsuite
---------------------

Once you have a source file with the afore-mentioned requirements, you can
compile a runnable testsuite by compiling the source file. That's it.

```
$ g++ -std=c++23 tests.cc -o test
$ ./test
```

If you are so inclined, you may even integrate it to CMake automagically. You
have an example of that (here)[CMakeLists.txt]. If you want to do that, I expect
you know CMake enough to not require a walkthrough of this.
