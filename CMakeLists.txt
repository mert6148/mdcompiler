cmake_minimum_required(VERSION 3.16)

project(mdcompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mdcompiler
    src/main.cpp
)

target_include_directories(mdcompiler PRIVATE src)

enable_testing()
add_test(
    NAME cli_help
    COMMAND mdcompiler --help
)
