#include "tuncfest.hh"

#include <string_view>

static char const binPath[] = "/usr/bin/cat";

// #define ADD_TEST(NAME, INPUT_STR, OUTPUT_STR, EXIT_CODE)
ADD_TEST(FirstTest, "42\n", "42\n", 0);
ADD_TEST(SecondTest, "43", "43", 0);

/*
Notepad:

Having TestSuites with names, so that you could have:
    Testsuite<TestSuiteName, binPath, TestStruct...>::run_all_tests();
    Testsuite<TestSuiteName, binPath, TestStruct...>::run_all_tests();
*/

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
