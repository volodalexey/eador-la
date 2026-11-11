cmake_minimum_required(VERSION 3.22)
project("GoD")

# Run directory: the eador binary goes here, game data is added by `make import`.
set(EADOR_OUT_DIR "${PROJECT_SOURCE_DIR}/out" CACHE PATH "Directory for the eador binary and game data")

enable_testing()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(raylib)

add_subdirectory("EadorServer")
add_subdirectory("God")
add_subdirectory("EadorUnitTests")
