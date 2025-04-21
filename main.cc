#include "tuncfest.hh"

static char const binPath[] = "./foo";

struct Test1
{
    static constexpr std::string_view name = "FirstTest";
    // TODO make this a std::optional
    static constexpr std::string_view input = "";

    // NOTE this shouldn't be an optional since every process has a
    // defined captured output, that might be empty
    static constexpr std::string_view expected_output = "42\n";
    static constexpr int expected_exit_code = 0;
};

struct Test2
{
    static constexpr std::string_view name = "SecondTest";
    // TODO make this a std::optional
    static constexpr std::string_view input = "";

    // NOTE this shouldn't be an optional since every process has a
    // defined captured output, that might be empty
    static constexpr std::string_view expected_output = "43\n";
    static constexpr int expected_exit_code = 0;
};

/*
VISION:

Instead of defining the types for the tests, having macros like this:

    ADD_TEST(name, input, expected_output, expected_exit_code)

Something like this:

    #define ADD_TEST(NAME, INPUT_STR, OUTPUT_STR, EXIT_CODE)                   \
        struct NAME {                                                          \
            static constexpr std::string_view name = #NAME;                    \
            static constexpr std::string_view input = INPUT_STR;               \
            static constexpr std::string_view expected_output = OUTPUT_STR;    \
            static constexpr int expected_exit_code = EXIT_CODE;               \
        };

+ Maybe with c++26 introspection, I can get back the name of the class directly
instead of making it a field.


Having TestSuites with names, so that you could have:
    Testsuite<TestSuiteName, binPath, TestStruct...>::run_all_tests();
    Testsuite<TestSuiteName, binPath, TestStruct...>::run_all_tests();
*/

int main(void)
{
    FunctionalTestRunner<binPath, Test1, Test2>::run_all_tests();
}
