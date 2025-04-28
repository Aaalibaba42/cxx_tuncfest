#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "/usr/bin/echo";

constexpr auto tb = TestBuilder<"FirstTest">()
                        .with_expected_stdout<"test\n">()
                        .with_command_line<"test">();

REGISTER_TEST(FirstTest, tb);

constexpr auto tb2 = TestBuilder<"SecondTest">()
                         .with_command_line<"multiple", "arguments">()
                         .with_expected_stdout<"multiple arguments\n">();

REGISTER_TEST(SecondTest, tb2);

int main(void)
{
    TestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
