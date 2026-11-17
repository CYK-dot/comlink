cmake_minimum_required(VERSION 3.20)

# ---- strata configurations -----------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bsp/sdk/strata/cmake")
include(strata)

strata_project(
    SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/configs/schema"
    PRESETS "${CMAKE_CURRENT_SOURCE_DIR}/configs/presets"
)
strata_add_component_path(
    "${CMAKE_CURRENT_SOURCE_DIR}/bsp"
)

# ---- cmake project configurations ----------------------------------------------------------------

strata_get_feature("bsp.FAMILY" BSP_FAMILY)
strata_get_feature("bsp.USB" BSP_USB)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/bsp/adapt/chip/${BSP_FAMILY}/toolchain.cmake")

project(elink C ASM)

# ---- compile policy ------------------------------------------------------------------------------
# First-party code (main, bsp/adapt, platform/adapt) and QP/C compile
# warning-free, so warnings are errors there. The vendor SDK libraries are
# pinned third-party code and stay outside the net: only the targets this
# project owns link project_warnings.

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-Werror>)

add_subdirectory(bsp)
add_subdirectory(platform)

# ---- USB device stack ----------------------------------------------------------------------------
# Driven by the bsp.USB feature (chip.usb in the preset): the board carries a
# USB device controller, so the app links platform/protousb.
if(BSP_USB)
    add_subdirectory(platform/protousb)
endif()

# ---- bootloader images ---------------------------------------------------------------------------

bsp_add_image(bootloader)
target_sources(bootloader PRIVATE main/bootloader.c)
target_link_libraries(bootloader PRIVATE bsp_boot bsp_dbg project_warnings)
bsp_image_set_layout_boot(bootloader)
bsp_add_hex(bootloader_hex bootloader)

# ---- application images --------------------------------------------------------------------------

bsp_add_image(app_raw)
target_sources(app_raw PRIVATE main/app.c main/app_metainfo.c)
target_compile_definitions(app_raw PRIVATE APP_VERSION=0x00010000u)
target_link_libraries(app_raw PRIVATE bsp_dbg bsp_sys platform_os project_warnings)
if(BSP_USB)
    target_link_libraries(app_raw PRIVATE platform_usb)
endif()
bsp_image_set_layout_app(app_raw)
bsp_add_image_signed(app app_raw)
bsp_add_hex(app_hex app)

# ---- full test images ----------------------------------------------------------------------------

bsp_add_manifest(full FROM bootloader app)
