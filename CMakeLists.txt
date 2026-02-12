cmake_minimum_required(VERSION 3.16)
project(gut VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gut is now header-only (single-header)
add_library(gut INTERFACE)
target_include_directories(gut INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Examples
add_subdirectory(examples)

# Tests
enable_testing()
add_subdirectory(tests)
