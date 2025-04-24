#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "./myheavycat";

struct FirstTestTag
{
    static constexpr std::string_view value = "FirstTest";
};
using FirstTest = decltype(addTest<FirstTestTag>()
                               .with_stdinput<sv("42")>()
                               .with_expected_stdout<sv("42")>()
                               .with_expected_stderr<sv("42")>()
                               .with_expected_exit_code<0>())::Result;

struct SecondTestTag
{
    static constexpr std::string_view value = "SecondTest";
};
using SecondTest = decltype(addTest<SecondTestTag>()
                                .with_stdinput<sv("43\n")>()
                                .with_expected_stdout<sv("43\n")>()
                                .with_expected_stderr<sv("43\n")>()
                                .with_expected_exit_code<0>())::Result;

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
