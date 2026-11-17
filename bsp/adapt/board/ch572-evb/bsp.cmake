# bsp/adapt/board/ch572-evb/bsp.cmake - CH572 evaluation board
#
# Board-level wiring on top of the CH57x chip adapter: the things that depend
# on how this board is built, not on the silicon.
#
#   bsp_dbg   debug LED and polling delay (src/bspdbg.c)
#
# The chip adapter (selected by bsp.FAMILY) must have run before this one; the
# board only builds on its bsp_chip facade.

get_filename_component(_BSP_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

if(NOT TARGET bsp_chip)
    message(FATAL_ERROR
        "bsp/board/ch572-evb: target 'bsp_chip' is missing.\n"
        "  The chip adapter must be included before the board adapter.")
endif()

if(NOT TARGET bsp_dbg)
    add_library(bsp_dbg STATIC "${_BSP_BOARD_DIR}/src/bspdbg.c")
    target_link_libraries(bsp_dbg PUBLIC bsp_chip)
    target_link_libraries(bsp_dbg PRIVATE project_warnings)
endif()
