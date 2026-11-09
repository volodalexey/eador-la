cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_executable(EadorServer "main.cpp")
target_include_directories(EadorServer PUBLIC "../include")
target_link_libraries(EadorServer PRIVATE Threads::Threads)
