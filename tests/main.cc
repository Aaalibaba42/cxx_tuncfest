#include "../tuncfest/tuncfest.hh"

static char const binPath[] = "/usr/bin/cat";

ADD_TEST(FirstTest, "42\n", "42\n", "", 0);
ADD_TEST(SecondTest, "43", "43", "", 0);

int main(void)
{
    FunctionalTestRunner<binPath, FirstTest, SecondTest>::run_all_tests();
}
