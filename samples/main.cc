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
