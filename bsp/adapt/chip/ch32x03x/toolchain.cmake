# bsp/adapt/chip/ch32x03x/toolchain.cmake - CH32X03x toolchain entry
#
# Lets the application reference bsp/adapt/chip/<family>/toolchain.cmake instead
# of a vendor SDK path. CMake requires CMAKE_TOOLCHAIN_FILE before project(), so
# this file only forwards to the SDK toolchain.

include("${CMAKE_CURRENT_LIST_DIR}/../../../sdk/ch32x03x/cmake/toolchain-riscv-wch.cmake")
