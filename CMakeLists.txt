cmake_minimum_required(VERSION 3.10)
project(CompilerLexer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# TODO: Add Flex/Lex configuration
# find_package(FLEX REQUIRED)
# FLEX_TARGET(...)
# add_executable(...)

# Enable testing with CTest
enable_testing()
add_subdirectory(tests)
