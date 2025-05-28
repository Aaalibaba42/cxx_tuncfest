cmake_minimum_required(VERSION 3.10)
project(simple_example VERSION 0.1.0 LANGUAGES CXX)

# This requires the latest C++ features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Works with every compilation warning I have tested
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -Werror -Wall -Wextra -pedantic -Wfloat-equal -Wundef -Wshadow -Wswitch-default -Wdouble-promotion -Wformat=2 -Wformat-signedness -Wno-format-zero-length -Wmissing-include-dirs -Wshift-negative-value -Wstring-compare -Wmissing-noreturn -Warray-bounds -Wdangling-else -Wenum-conversion"
)

# Getting the tuncfest automagically
include(FetchContent)
FetchContent_Declare(
    tuncfest
    GIT_REPOSITORY https://github.com/Aaalibaba42/cxx_tuncfest.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(tuncfest)

# Very simple to link
add_executable(main main.cc)
target_link_libraries(main PRIVATE tuncfest)

# Automatically launch the testsuite during the build process
add_custom_command(
    TARGET main
    POST_BUILD
    COMMAND $<TARGET_FILE:main>
)
