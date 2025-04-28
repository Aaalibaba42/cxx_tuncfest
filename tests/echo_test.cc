#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "/usr/bin/echo";

constexpr auto FirstTestBuilder = testBuilder("FirstTest")
                                      .with_expected_stdout<"test\n">()
                                      .with_command_line<"test">();

constexpr auto SecondTestBuilder =
    testBuilder("SecondTest")
        .with_expected_stdout<"multiple arguments\n">()
        .with_command_line<"multiple", "arguments", "again">();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
