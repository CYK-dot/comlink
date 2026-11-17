# platform/CMakeLists.txt - platform layer entry
#
# The layer between the application and the chip: the runtime that owns
# scheduling (QP/C today), its CPU primitives, and the OSAL the application
# programs to.
#
# Directory layout:
#   os/qpc/         upstream QP/C (git submodule, kept unmodified)
#   os/adapt/qpc/   this project's port: QP/C configuration, port headers,
#                   board callbacks and the build glue
#   os/inc/         public platform headers (OSAL)
#
# The core primitives this port needs (interrupt mask, idle, halt) come from
# the BSP (bsp/inc/bsp_cpu.h): nothing here knows the core.
#
# One runtime is wired today; a second one is a strata selection (like
# bsp.FAMILY), not a build option.

cmake_minimum_required(VERSION 3.20)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/os/adapt/qpc")
