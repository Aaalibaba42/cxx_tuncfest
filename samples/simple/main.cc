#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

constexpr auto greatTestBuilder = testBuilder("GreatTest")
                                      .with_stdinput<"42\n">()
                                      .with_expected_stdout<"42\n">();

REGISTER_TEST(MyGreatTest, greatTestBuilder);

int main(void)
{
    FunctionalTestRunner<binPath, MyGreatTest>::run_all_tests();
}
