cmake_minimum_required(VERSION 3.10)

project(tuncfest VERSION 1.0.0 LANGUAGES CXX)

# `static operator()`
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "GCC version 13.0 or higher is required for C++23 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
        message(FATAL_ERROR "Clang version 16.0 or higher is required for C++23 support")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler. Please use GCC 13.0+ or Clang 16.0+.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WARN_FLAGS "-fdiagnostics-color=always -Wall -Wextra -Wpedantic -Werror")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(WARN_FLAGS "${WARN_FLAGS} -Wfloat-equal -Wundef -Wshadow -Wswitch-default -Wlogical-op -Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wno-format-zero-length -Wmissing-include-dirs -Wshift-negative-value -Wshift-overflow=2 -Wuseless-cast -Wstring-compare -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=format -Wsuggest-attribute=cold -Warith-conversion -Wduplicated-cond -Warray-bounds -Wunsafe-loop-optimizations -Wdangling-else -Wenum-conversion")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Everything but the compat, and some other clumbersome things
    set(WARN_FLAGS "${WARN_FLAGS} -Weverything -Wno-pre-c++14-compat -Wno-c++20-compat -Wno-c++98-compat -Wno-c++98-compat-extra-semi -Wno-pre-c++20-compat-pedantic -Wno-implicit-int-float-conversion -ferror-limit=1000 -Wno-c++11-narrowing -Wno-padded -Wno-unsafe-buffer-usage")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_FLAGS}")

add_library(tuncfest INTERFACE)
target_include_directories(tuncfest
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tuncfest>
        $<INSTALL_INTERFACE:include>
)
