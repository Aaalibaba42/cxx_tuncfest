#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "./myheavycat";

constexpr auto FirstTestBuilder = testBuilder("FirstTest")
                                      .with_stdinput<sv("42")>()
                                      .with_expected_stdout<sv("42")>()
                                      .with_expected_stderr<sv("42")>();

constexpr auto SecondTestBuilder = testBuilder("SecondTest")
                                       .with_stdinput<sv("43\n")>()
                                       .with_expected_stdout<sv("43\n")>()
                                       // Ooops I mispelled
                                       .with_expected_stderr<sv("42\n")>();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
