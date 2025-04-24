#include "tuncfest.hh"

static char const binPath[] = "./myheavycat";

#define Test(n)                                                                \
    constexpr auto Test##n##Builder =                                          \
        testBuilder("Test" #n)                                                 \
            .with_stdinput<sv(                                                 \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm")>()                                             \
            .with_expected_stdout<sv(                                          \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm")>()                                             \
            .with_expected_stderr<sv(                                          \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm")>();                                            \
    REGISTER_TEST(Test##n, Test##n##Builder)

Test(1);
Test(2);
Test(3);
Test(4);
Test(5);
Test(6);
Test(7);
Test(8);
Test(9);
Test(11);
Test(12);
Test(13);
Test(14);
Test(15);
Test(16);
Test(17);
Test(18);
Test(19);
Test(20);
Test(21);
Test(22);
Test(23);
Test(24);
Test(25);
Test(26);
Test(28);
Test(29);
Test(30);
Test(31);
Test(32);
Test(33);
Test(34);
Test(35);

int main(void)
{
    // clang-format off
    FunctionalTestRunner<binPath,
        Test1,
        Test2,
        Test3,
        Test4,
        Test5,
        Test6,
        Test7,
        Test8,
        Test9,
        Test11,
        Test12,
        Test13,
        Test14,
        Test15,
        Test16,
        Test17,
        Test18,
        Test19,
        Test20,
        Test21,
        Test22,
        Test23,
        Test24,
        Test25,
        Test26,
        Test28,
        Test29,
        Test30,
        Test31,
        Test32,
        Test33,
        Test34,
        Test35
    >::run_all_tests();
    // clang-format on
}
