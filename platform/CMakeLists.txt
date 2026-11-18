# platform/CMakeLists.txt - platform layer entry
#
# The layer between the application and the chip: the scheduler that owns
# event delivery, its port to the CPU primitives, and the public OSAL headers.
#
# Directory layout:
#   os/inc/     public OSAL headers
#   os/src/     scheduler implementation (private headers live here too)
#   os/adapt/   port layer built on bsp/inc/bsp_cpu.h

cmake_minimum_required(VERSION 3.20)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/os")
