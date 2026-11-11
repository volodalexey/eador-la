# Populate the run directory next to the eador binary without replacing user
# data on reruns. Game assets (dat/, text/, var/) are imported via `make import`.
if(NOT DEFINED SOURCE_CFG OR NOT DEFINED RUNTIME_DIR)
  message(FATAL_ERROR "SOURCE_CFG and RUNTIME_DIR are required")
endif()

file(MAKE_DIRECTORY "${RUNTIME_DIR}/save")
file(MAKE_DIRECTORY "${RUNTIME_DIR}/screens")

# The game opens "eador.cfg" (lowercase); the repo keeps it as Eador.cfg.
if(NOT EXISTS "${RUNTIME_DIR}/eador.cfg")
  file(COPY_FILE "${SOURCE_CFG}" "${RUNTIME_DIR}/eador.cfg")
endif()
