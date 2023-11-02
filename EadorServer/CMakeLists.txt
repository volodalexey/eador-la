cmake_minimum_required(VERSION 3.26)


set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

add_executable(EadorServer "main.cpp" ${HEADERS} ${SOURCES})
target_include_directories(EadorServer PUBLIC "../include")
