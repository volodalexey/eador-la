cmake_minimum_required(VERSION 3.8)
project("GoD")

# Allegro 4.4 prefix for native Linux builds (e.g. the allegro4 submodule
# after `cmake --build --install`). Unset means only EadorServer is built.
set(ALLEGRO4_PREFIX "" CACHE PATH "Root of an Allegro 4.4 installation")

if(ALLEGRO4_PREFIX)
  set(ALLEGRO4_INCLUDE "${ALLEGRO4_PREFIX}/include")
  set(ALLEGRO4_LIB_DIR "${ALLEGRO4_PREFIX}/lib")
  set(ALLEGRO4_LIBRARY "${ALLEGRO4_PREFIX}/lib/liballeg.so")
endif()

enable_testing()

add_subdirectory("EadorServer")

if(ALLEGRO4_PREFIX)
  add_subdirectory("God")
  add_subdirectory("EadorUnitTests")
endif()
