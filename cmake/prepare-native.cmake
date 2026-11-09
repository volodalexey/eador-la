# Populate the native runtime directory without replacing user data on reruns.
# Game assets (dat/, text/, var/) are imported separately via `make import`.
if(NOT DEFINED SOURCE_DIR OR NOT DEFINED BUILD_DIR OR NOT DEFINED RUNTIME_DIR)
  message(FATAL_ERROR "SOURCE_DIR, BUILD_DIR and RUNTIME_DIR are required")
endif()

file(MAKE_DIRECTORY "${RUNTIME_DIR}")
file(COPY_FILE "${BUILD_DIR}/God/Eador" "${RUNTIME_DIR}/Eador" ONLY_IF_DIFFERENT)

file(MAKE_DIRECTORY "${RUNTIME_DIR}/save")
file(MAKE_DIRECTORY "${RUNTIME_DIR}/screens")

# The game opens "eador.cfg" (lowercase); the repo keeps it as Eador.cfg.
if(NOT EXISTS "${RUNTIME_DIR}/eador.cfg")
  file(COPY_FILE "${SOURCE_DIR}/God/Eador.cfg" "${RUNTIME_DIR}/eador.cfg")
endif()

message(STATUS "Native runtime ready: ${RUNTIME_DIR}")
