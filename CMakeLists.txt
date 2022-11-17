cmake_minimum_required(VERSION 3.13)
project(CppCommandParser)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(
        -Wcast-align
        -Wpedantic
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Wmissing-include-dirs
        -Wdisabled-optimization
        -Winit-self
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wparentheses
        -Wredundant-decls
        -Wshadow
        -Wsign-promo
        -Wstrict-aliasing
        -Wall
        -Wextra
        -Werror
)

add_subdirectory(external)

add_executable(cpp_command_parser_sample example_main.cpp)
target_include_directories(cpp_command_parser_sample PUBLIC include)

enable_testing()
add_subdirectory(test)
