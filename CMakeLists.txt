cmake_minimum_required(VERSION 3.10)

project(tuncfest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
    -fdiagnostics-color=always -Werror -Wall -Wextra -pedantic -Wfloat-equal -Wundef -Wshadow -Wswitch-default -Wlogical-op -Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wno-format-zero-length -Wmissing-include-dirs -Wshift-negative-value -Wshift-overflow=2 -Wuseless-cast -Wstring-compare -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=format -Wsuggest-attribute=cold -Warith-conversion -Wduplicated-cond -Warray-bounds -Wunsafe-loop-optimizations -Wdangling-else -Wenum-conversion"
)

add_library(tuncfest INTERFACE)

target_include_directories(tuncfest
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tuncfest>
        $<INSTALL_INTERFACE:include>
)
