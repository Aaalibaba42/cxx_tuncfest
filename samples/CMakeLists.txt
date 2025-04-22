cmake_minimum_required(VERSION 3.10)
project(sample VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    tuncfest
    GIT_REPOSITORY https://github.com/Aaalibaba42/cxx_tuncfest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(tuncfest)

add_executable(main main.cc)
target_link_libraries(main PRIVATE tuncfest)

