#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "./myheavycat";

// clang-format off
ADD_TEST(FirstTest,
    WITH_INPUT("42")
    WITH_EXPECTED_STDOUT("42")
    WITH_EXPECTED_STDERR("42")
);

ADD_TEST(SecondTest,
    WITH_INPUT("43\n")
    WITH_EXPECTED_STDOUT("43\n")
    WITH_EXPECTED_STDERR("43\n")
);
// clang-format on

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
