# bsp/CMakeLists.txt - project-level BSP entry
#
# Three adapters, each named by a feature and none by a build option:
#
#   adapt/cpu/<arch>/      bsp.ARCH_*    core primitives (header-only, inline)
#   adapt/chip/<family>/   bsp.FAMILY    chip family: vendor SDK, toolchain,
#                                        image layouts, chip bring-up
#   adapt/board/<board>/   bsp.BOARD     board wiring on top of the chip
#
# The application only talks to the bsp_* facade, never to a vendor SDK
# directly.

cmake_minimum_required(VERSION 3.20)

strata_get_feature("bsp.FAMILY" BSP_FAMILY)
strata_get_feature("bsp.BOARD" BSP_BOARD)
strata_get_feature("bsp.ARCH_QINGKE" BSP_ARCH_QINGKE)

# ---- CPU backend -------------------------------------------------------------
# The core is a BSP feature: the board selects it and this file compiles it. A
# second core is a new bsp.ARCH_* feature plus its adapt/cpu/<arch>/ directory.
if(BSP_ARCH_QINGKE)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/adapt/cpu/qingke"
                     "${CMAKE_CURRENT_BINARY_DIR}/cpu/qingke")
else()
    message(FATAL_ERROR
        "bsp: board '${BSP_BOARD}' selects no CPU backend.\n"
        "  Declare a bsp.ARCH_* feature in bsp/strata_features.toml for it.")
endif()

# ---- Chip adapter ------------------------------------------------------------
set(_bsp_chip "${CMAKE_CURRENT_LIST_DIR}/adapt/chip/${BSP_FAMILY}/bsp.cmake")
if(NOT EXISTS "${_bsp_chip}")
    set(_bsp_choices "")
    file(GLOB _bsp_dirs LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/adapt/chip/*")
    foreach(_c IN LISTS _bsp_dirs)
        if(EXISTS "${_c}/bsp.cmake")
            get_filename_component(_n "${_c}" NAME)
            list(APPEND _bsp_choices "${_n}")
        endif()
    endforeach()
    message(FATAL_ERROR
        "bsp: no chip adapter for family '${BSP_FAMILY}'.\n"
        "  Available: ${_bsp_choices}")
endif()
include("${_bsp_chip}")

# ---- Board adapter -----------------------------------------------------------
set(_bsp_board "${CMAKE_CURRENT_LIST_DIR}/adapt/board/${BSP_BOARD}/bsp.cmake")
if(NOT EXISTS "${_bsp_board}")
    set(_bsp_choices "")
    file(GLOB _bsp_dirs LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/adapt/board/*")
    foreach(_c IN LISTS _bsp_dirs)
        if(EXISTS "${_c}/bsp.cmake")
            get_filename_component(_n "${_c}" NAME)
            list(APPEND _bsp_choices "${_n}")
        endif()
    endforeach()
    message(FATAL_ERROR
        "bsp: no board adapter for '${BSP_BOARD}'.\n"
        "  Available: ${_bsp_choices}")
endif()
include("${_bsp_board}")

# Feature values reach C as STRATA_FEATURE_* macros; bsp_cpu.h picks its inline
# backend from them.
target_include_directories(bsp_common INTERFACE "${STRATA_OUT_DIR}")
