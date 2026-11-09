set(EADOR_SOURCES
  Audio.cpp BattleAI.cpp Battlefield.cpp BF_Unit.cpp Campaign.cpp Castle.cpp
  CurrentGameState.cpp Eador.cpp Event.cpp GlobalCrutches.cpp G_Object.cpp
  GameMenu.cpp Globals.cpp Hero.cpp Interface.cpp Keymaster.cpp Logging.cpp
  MainAI.cpp MainProcess.cpp Mapper.cpp Player.cpp Prototype.cpp Province.cpp
  ProvList.cpp Randomizer.cpp Screens.cpp Simulation.cpp Unit.cpp Windower.cpp
  WorldRules.cpp Yuplay.cpp NativeDisplay.cpp
)

if(NOT ALLEGRO4_INCLUDE OR NOT ALLEGRO4_LIBRARY)
  message(FATAL_ERROR "Native Eador build requires ALLEGRO4_PREFIX")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(VORBIS REQUIRED vorbisfile)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XINERAMA REQUIRED xinerama)

# OGG streaming used by Audio.cpp (alogg-compatible).
add_library(alogg_impl STATIC alogg_impl.cpp)
target_compile_features(alogg_impl PRIVATE cxx_std_20)
target_include_directories(alogg_impl PRIVATE
  "${ALLEGRO4_INCLUDE}"
  "${PROJECT_SOURCE_DIR}/include"
  ${VORBIS_INCLUDE_DIRS}
)
target_link_libraries(alogg_impl PRIVATE ${VORBIS_LIBRARIES})

# Общие настройки для игры и для библиотеки, с которой линкуются тесты.
function(eador_configure target)
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_compile_definitions(${target} PUBLIC _CRT_SECURE_NO_WARNINGS)
  # Russian string literals stay CP1251 at runtime, as the game fonts expect.
  target_compile_options(${target} PUBLIC
    -fexec-charset=CP1251
    -finput-charset=UTF-8
  )
  # Allegro 4.4 headers first, then the bundled headers (alogg.h etc.).
  target_include_directories(${target} PUBLIC
    "${ALLEGRO4_INCLUDE}"
    "${PROJECT_SOURCE_DIR}/include"
  )
  target_link_directories(${target} PUBLIC "${ALLEGRO4_LIB_DIR}")
  target_link_libraries(${target} PUBLIC
    alogg_impl
    "${ALLEGRO4_LIBRARY}"
    ${VORBIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${XINERAMA_LIBRARIES}
  )
endfunction()

# Код игры без main() (NO_EADOR_MAIN, как в старом проекте VS): с ним линкуются
# и игра, и EadorUnitTests, поэтому исходники компилируются один раз.
add_library(eador_core STATIC ${EADOR_SOURCES})
eador_configure(eador_core)
target_compile_definitions(eador_core PUBLIC NO_EADOR_MAIN)

add_executable(Eador EadorMain.cpp)
target_link_libraries(Eador PRIVATE eador_core)

add_custom_target(prepare-native
  COMMAND "${CMAKE_COMMAND}"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
    "-DRUNTIME_DIR=${PROJECT_SOURCE_DIR}/out/eador-linux"
    -P "${PROJECT_SOURCE_DIR}/cmake/prepare-native.cmake"
  DEPENDS Eador
  VERBATIM
)
