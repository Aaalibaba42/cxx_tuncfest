#include "tuncfest.hh"

static char const binPath[] = "./myheavycat";

#define Test(n)                                                                \
    constexpr auto Test##n##Builder =                                          \
        TestBuilder<>()                                                        \
            .with_name<"Test" #n>()                                            \
            .with_stdinput<                                                    \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm">()                                              \
            .with_expected_stdout<                                             \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm">()                                              \
            .with_expected_stderr<                                             \
                "'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,"             \
                "cplrghusetjm'.,"                                              \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.,cplrghusetjm'.," \
                "cplrghusetjm">();                                             \
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
Test(36);
Test(37);
Test(38);
Test(39);
Test(40);
Test(41);
Test(42);
Test(43);
Test(44);
Test(45);
Test(46);
Test(47);
Test(48);
Test(49);
Test(50);
Test(51);
Test(52);
Test(53);
Test(54);
Test(55);
Test(56);
Test(57);
Test(58);
Test(59);
Test(60);

int main(void)
{
    // clang-format off
    TestRunner<binPath,
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
        Test35,
        Test36,
        Test37,
        Test38,
        Test39,
        Test40,
        Test41,
        Test42,
        Test43,
        Test44,
        Test45,
        Test46,
        Test47,
        Test48,
        Test49,
        Test50,
        Test51,
        Test52,
        Test53,
        Test54,
        Test55,
        Test56,
        Test57,
        Test58,
        Test59,
        Test60
    >::run_all_tests();
    // clang-format on
}
