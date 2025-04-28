#include "../tuncfest/tuncfest.hh"

constexpr char const binPath[] = "./myheavycat";

constexpr auto default_builder = TestBuilder<>().with_expected_exit_code<0>();

constexpr auto tb1 = default_builder.with_name<"FirstTest">()
                         .with_stdinput<"42">()
                         .with_expected_stdout<"42">()
                         .with_expected_stderr<"42">();

constexpr auto tb2 = default_builder.with_name<"SecondTest">()
                         .with_stdinput<"43\n">()
                         .with_expected_stdout<"43\n">()
                         // Ooops I mispelled
                         .with_expected_stderr<"42\n">();

REGISTER_TEST(FirstTest, tb1);
REGISTER_TEST(SecondTest, tb2);

int main(void)
{
    TestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
