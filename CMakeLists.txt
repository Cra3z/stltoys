cmake_minimum_required(VERSION 3.22)

project(stltoys)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)

enable_testing()

find_package(GTest REQUIRED)

add_subdirectory(test)