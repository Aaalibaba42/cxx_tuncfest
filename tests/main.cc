#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "./myheavycat";

bool compare_to_42(std::string_view maybe_42)
{
    return maybe_42 == "42";
}

constexpr auto FirstTestBuilder =
    TestBuilder<"FirstTest">()
        .with_stdinput<"42">()
        .with_stdout_validation<compare_to_42>()
        .with_stderr_validation<compare_to_42>()
        .with_exit_code_validation<[](int exit_code) -> bool {
            return exit_code >= 0;
        }>();

constexpr auto SecondTestBuilder =
    TestBuilder<"SecondTest">()
        .with_stdinput<"43\n">()
        .with_stdout_match<"43\n">()
        .with_stderr_validation<[](std::string_view got) -> bool {
            // Ooops I mispelled
            return got == std::string("42\n");
        }>()
        .with_exit_code_match<0>();

REGISTER_TEST(FirstTest, FirstTestBuilder);
REGISTER_TEST(SecondTest, SecondTestBuilder);

int main(void)
{
    TestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
