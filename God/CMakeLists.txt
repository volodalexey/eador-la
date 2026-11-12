# Общие списки исходников для Linux и Android сборок
include("${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake")

# OGG streaming used by Audio.cpp (alogg API on top of raylib Music streams).
add_library(alogg_impl STATIC alogg_impl_rl.cpp)
target_compile_features(alogg_impl PRIVATE cxx_std_20)
target_include_directories(alogg_impl PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(alogg_impl PRIVATE raylib)

# Код игры без main() (NO_EADOR_MAIN, как в старом проекте VS): с ним линкуются
# и игра, и EadorUnitTests, поэтому исходники компилируются один раз.
add_library(eador_core STATIC ${EADOR_GAME_SOURCES} ${EADOR_PLATFORM_SOURCES})
target_compile_features(eador_core PUBLIC cxx_std_20)
target_compile_definitions(eador_core PUBLIC _CRT_SECURE_NO_WARNINGS NO_EADOR_MAIN)
# Russian string literals stay CP1251 at runtime, as the game fonts expect.
target_compile_options(eador_core PUBLIC
  -fexec-charset=CP1251
  -finput-charset=UTF-8
)
target_include_directories(eador_core PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(eador_core PUBLIC alogg_impl raylib)

add_executable(eador EadorMain.cpp)
target_link_libraries(eador PRIVATE eador_core)
set_target_properties(eador PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${EADOR_OUT_DIR}")

# save/, screens/ and the default eador.cfg next to the binary; user data is kept.
add_custom_command(TARGET eador POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    "-DSOURCE_CFG=${CMAKE_CURRENT_SOURCE_DIR}/Eador.cfg"
    "-DRUNTIME_DIR=${EADOR_OUT_DIR}"
    -P "${PROJECT_SOURCE_DIR}/cmake/prepare-native.cmake"
  VERBATIM
)
