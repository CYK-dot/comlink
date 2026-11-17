# bsp/adapt/chip/ch57x/toolchain.cmake - CH57x toolchain entry
#
# Lets the application reference bsp/adapt/chip/<family>/toolchain.cmake instead
# of a vendor SDK path. CMake requires CMAKE_TOOLCHAIN_FILE before project(), so
# this file only forwards to the SDK toolchain.

include("${CMAKE_CURRENT_LIST_DIR}/../../../sdk/ch57x/cmake/toolchain-riscv-wch.cmake")
