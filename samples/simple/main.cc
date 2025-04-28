#include "tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

constexpr auto greatTestBuilder = TestBuilder<>()
                                      .with_name<"GreatTest">()
                                      .with_stdinput<"42\n">()
                                      .with_expected_stdout<"42\n">();

REGISTER_TEST(MyGreatTest, greatTestBuilder);

int main(void)
{
    TestRunner<binPath, MyGreatTest>::run_all_tests();
}
