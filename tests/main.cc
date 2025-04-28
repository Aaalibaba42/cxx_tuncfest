#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "./myheavycat";

constexpr auto FirstTestBuilder = TestBuilder<"FirstTest">()
                                      .with_stdinput<"42">()
                                      .with_expected_stdout<"42">()
                                      .with_expected_stderr<"42">();

constexpr auto SecondTestBuilder = TestBuilder<"SecondTest">()
                                       .with_stdinput<"43\n">()
                                       .with_expected_stdout<"43\n">()
                                       // Ooops I mispelled
                                       .with_expected_stderr<"42\n">();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    TestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
