cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER clang-10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_COMPILER clang++-10)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-O2 -Wall -Werror)

project(SysYCompiler)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
