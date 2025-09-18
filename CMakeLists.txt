cmake_minimum_required(VERSION 3.31)
project(apusc)
# Ari Palanjian's Unix Server Core?
enable_testing()

add_subdirectory(extern/googletest)
add_subdirectory(tests)

set(CMAKE_CXX_STANDARD 20)
