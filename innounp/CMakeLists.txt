# Распаковщик установщиков GOG Eador: Genesis (Inno Setup 5.5.7).
# Зависит только от zlib; LZMA-декодер — LZMA SDK (public domain) в lzma/.
cmake_minimum_required(VERSION 3.18)
project(innounp C CXX)

find_package(ZLIB REQUIRED)

add_library(innounp STATIC
  innounp.cpp
  lzma/LzmaDec.c
  lzma/Lzma2Dec.c
)
target_compile_features(innounp PUBLIC cxx_std_17)
target_include_directories(innounp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(innounp PUBLIC ZLIB::ZLIB)
set_target_properties(innounp PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(INNOUNP_TOOL "Build the innounp command-line tool" ON)
if(INNOUNP_TOOL)
  add_executable(innounp-cli main.cpp)
  set_target_properties(innounp-cli PROPERTIES OUTPUT_NAME innounp)
  target_link_libraries(innounp-cli PRIVATE innounp)
endif()
