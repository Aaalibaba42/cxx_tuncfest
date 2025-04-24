#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

static constexpr auto FirstTestBuilder = testBuilder("FirstTest")
                                             .with_stdinput<"42\n">()
                                             .with_expected_stdout<"42\n">();

constexpr auto SecondTestBuilder = testBuilder("SecondTest")
                                       .with_stdinput<"43\n">()
                                       .with_expected_stdout<"43\n">();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
