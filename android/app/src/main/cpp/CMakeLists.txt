cmake_minimum_required(VERSION 3.22)
project(eador C CXX)

# cpp -> main -> src -> app -> android -> репозиторий
get_filename_component(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

# raylib для Android
set(PLATFORM "Android" CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${REPO}/raylib" raylib)

# Обёртка fopen raylib перенаправляет относительные пути в internalDataPath.
get_property(raylib_link_opts TARGET raylib PROPERTY INTERFACE_LINK_OPTIONS)
message(STATUS "raylib INTERFACE_LINK_OPTIONS: ${raylib_link_opts}")
list(FILTER raylib_link_opts EXCLUDE REGEX "wrap=fopen")
set_property(TARGET raylib PROPERTY INTERFACE_LINK_OPTIONS "${raylib_link_opts}")

# innounp (без утилиты)
set(INNOUNP_TOOL OFF CACHE BOOL "" FORCE)
add_subdirectory("${REPO}/innounp" innounp)

include("${CMAKE_CURRENT_SOURCE_DIR}/cp1251.cmake")
include("${REPO}/God/sources.cmake")

set(EADOR_UTF8_FILES)
foreach(src ${EADOR_GAME_SOURCES} ${EADOR_PLATFORM_SOURCES} alogg_impl_rl.cpp)
    list(APPEND EADOR_UTF8_FILES "${REPO}/God/${src}")
endforeach()

# Копии .cpp включают соседние заголовки из своего каталога, поэтому
# перекодируются все заголовки God/.
file(GLOB EADOR_HEADERS "${REPO}/God/*.h" "${REPO}/God/*.hpp")
list(APPEND EADOR_UTF8_FILES ${EADOR_HEADERS}
    "${REPO}/include/alogg.h"
    "${REPO}/include/aloggdll.h"
)

convert_to_cp1251(EADOR_CP1251_FILES "${EADOR_UTF8_FILES}")

add_library(eador SHARED
    ${EADOR_CP1251_FILES}
    android_entry.cpp
    android_input.cpp
    jni_import.cpp
)

target_compile_features(eador PRIVATE cxx_std_20)
target_compile_definitions(eador PRIVATE _CRT_SECURE_NO_WARNINGS NO_EADOR_MAIN)
# Тулчейн NDK добавляет -Werror=format-security; код игры передаёт в printf
# нелитеральные строки формата, на Linux это не ошибка.
target_compile_options(eador PRIVATE -Wno-invalid-source-encoding -Wno-format-security)
target_include_directories(eador PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/cp1251/God"
    "${CMAKE_CURRENT_BINARY_DIR}/cp1251/include"
    "${REPO}/include"
    "${ANDROID_NDK}/sources/android/native_app_glue"
)
target_link_libraries(eador PRIVATE raylib innounp android log z)
target_link_options(eador PRIVATE -u ANativeActivity_onCreate)
