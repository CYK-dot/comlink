cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/bsp/sdk/ch57x/cmake/toolchain-riscv-wch.cmake)
project(daplink_com C ASM)
add_subdirectory(bsp/sdk/ch57x)

ch57x_add_image(bootloader)
target_sources(bootloader PRIVATE main/bootloader.c)
target_link_libraries(bootloader PRIVATE wch_ch57x_stdperi)
ch57x_image_set_layout_default(bootloader)

ch57x_add_image(app)
target_sources(app PRIVATE main/app.c)
target_link_libraries(app PRIVATE wch_ch57x_stdperi)
ch57x_image_set_layout_custom(app
    LINKER  ${CMAKE_SOURCE_DIR}/main/app.ld
    STARTUP ${CMAKE_SOURCE_DIR}/main/startup_app.S)

ch57x_add_manifest(full FROM bootloader app)
