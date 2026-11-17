# bsp/adapt/chip/ch57x/bsp.cmake - chip adapter for the CH57x family
#
# This is the only place the application talks to for build plumbing. It wraps
# the ch57x SDK and adds the project's own partitioning:
#
#   bsp_add_image(<target>)                       create an image (ELF)
#   bsp_image_set_layout_boot(<target>)           boot slot (0x0, 240K, layout/boot.ld)
#   bsp_image_set_layout_app(<target>)            app slot  (0x4000, 220K, layout/app.ld)
#   bsp_image_set_layout_default(<target>)        vendor default shape (0x0, 240K)
#   bsp_image_set_layout_custom(<target> LINKER <ld> STARTUP <s>)
#   bsp_add_image_signed(<target> <raw_target>)   sign a raw image (magic/length/crc32)
#   bsp_add_hex(<binary_target> <elf_target>)     flashable Intel HEX
#   bsp_add_manifest(<target> FROM <image>...)    merge images into one full image
#
# Runtime boot helpers live in include/bspboot.h (library target bsp_boot).
#
# Directory layout:
#   ../../../inc  public headers (shared by all adapters)
#   layout/       image shapes: linker script + startup
#   image/        image format: signer and generated layout header
#   src/          implementations
#
# The board-specific parts (debug LED, board wiring) live in bsp/adapt/board/.
# The standard peripheral library is still linked directly by the application
# (wch_ch57x_stdperi); isolating it behind this layer is a later step.

get_filename_component(_BSP_CH57X_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
get_filename_component(_BSP_SDK_DIR    "${_BSP_CH57X_DIR}/../../../sdk/ch57x" ABSOLUTE)
set(_BSP_INCLUDE_DIR "${_BSP_CH57X_DIR}/../../../inc")
set(_BSP_IMAGE_DIR   "${_BSP_CH57X_DIR}/image")
set(_BSP_GEN_DIR     "${CMAKE_BINARY_DIR}/bsp/gen")

if(NOT EXISTS "${_BSP_SDK_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "bsp/ch57x: vendor SDK not found at ${_BSP_SDK_DIR}")
endif()

if(NOT TARGET wch_ch57x_stdperi)
    add_subdirectory("${_BSP_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/sdk/ch57x")
endif()

# ---- Image layout ------------------------------------------------------------
# Nothing below reads the .ld files. CMake only hands them to the linker and to
# the footprint tool; the application slot bounds reach C as linker symbols
# provided by layout/boot.ld. Renaming a memory region, reformatting a script or
# adding a region therefore cannot break the build.
set(BSP_CH57X_IMG_MAGIC "0x37354843" CACHE INTERNAL "CH57x image magic word")

set(BSP_IMG_MAGIC "${BSP_CH57X_IMG_MAGIC}")
file(MAKE_DIRECTORY "${_BSP_GEN_DIR}")
configure_file("${_BSP_IMAGE_DIR}/bsp_layout.h.in" "${_BSP_GEN_DIR}/bsp_layout.h" @ONLY)

# ---- Targets ---------------------------------------------------------------
if(NOT TARGET bsp_common)
    add_library(bsp_common INTERFACE)
    target_include_directories(bsp_common INTERFACE "${_BSP_INCLUDE_DIR}" "${_BSP_GEN_DIR}")
    # The chip compile contract is part of the board: every component built for
    # this chip (BSP, platform/OS, drivers) inherits the same arch/ABI, section
    # GC and optimisation flags.
    target_compile_options(bsp_common INTERFACE ${_CH57X_COMPILE})
    # The CPU backend (inline primitives) selected by bsp.ARCH_*.
    target_link_libraries(bsp_common INTERFACE bsp_cpu)
endif()

# Chip facade the board adapter builds on: headers, compile contract and the
# vendor SDK.
if(NOT TARGET bsp_chip)
    add_library(bsp_chip INTERFACE)
    target_link_libraries(bsp_chip INTERFACE bsp_common wch_ch57x_stdperi)
endif()

if(NOT TARGET bsp_sys)
    add_library(bsp_sys STATIC
        "${_BSP_CH57X_DIR}/src/bspsys.c"
        "${_BSP_CH57X_DIR}/../../cpu/qingke/bsp_cpu.c")
    target_link_libraries(bsp_sys PUBLIC bsp_chip)
    target_link_libraries(bsp_sys PRIVATE project_warnings)
endif()

if(NOT TARGET bsp_boot)
    add_library(bsp_boot STATIC "${_BSP_CH57X_DIR}/src/bspboot.c")
    target_link_libraries(bsp_boot PUBLIC bsp_chip bsp_sys)
    target_link_libraries(bsp_boot PRIVATE project_warnings)
endif()

# Signing needs a host interpreter; find_program avoids the cross-compiling
# heuristics of FindPython3.
if(NOT BSP_PYTHON_EXECUTABLE)
    find_program(BSP_PYTHON_EXECUTABLE NAMES python3 python
        DOC "Host Python interpreter used by the image signer")
endif()

################################################################################
# @brief Create an image target (ELF only; use bsp_add_hex() for a flash file)
################################################################################
function(bsp_add_image target)
    ch57x_add_image(${target})
endfunction()

################################################################################
# @brief Bind the vendor default layout: single image starting at 0x0
################################################################################
function(bsp_image_set_layout_default target)
    ch57x_image_set_layout_default(${target})
endfunction()

################################################################################
# @brief Bind the boot slot (layout/boot.ld + the SDK startup)
#
# The boot image gets its own shape, so the two images declare distinct memory
# regions: bootflash + sram here, appflash + sram for the application.
################################################################################
function(bsp_image_set_layout_boot target)
    set(_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    ch57x_image_set_layout_custom(${target}
        LINKER  "${_dir}/layout/boot.ld"
        STARTUP "${WCH_CH57X_STARTUP}")
endfunction()

################################################################################
# @brief Bind this project's application slot (layout/app.ld + startup_app.S)
################################################################################
function(bsp_image_set_layout_app target)
    set(_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    ch57x_image_set_layout_custom(${target}
        LINKER  "${_dir}/layout/app.ld"
        STARTUP "${_dir}/layout/startup_app.S")
endfunction()

################################################################################
# @brief Bind a custom linker script and startup
################################################################################
function(bsp_image_set_layout_custom target)
    ch57x_image_set_layout_custom(${target} ${ARGN})
endfunction()

################################################################################
# @brief Register a derived (signed) image from a raw image
#
# The raw image must carry .imginfo.magic / .imginfo.length / .imginfo.crc32
# placeholders and a .metainfo section holding the version. The signer fills the
# three info fields from the bytes that follow .metainfo.
################################################################################
function(bsp_add_image_signed target raw_target)
    if(target STREQUAL raw_target)
        message(FATAL_ERROR "bsp_add_image_signed: target names must differ ('${target}')")
    endif()
    if(TARGET ${target})
        message(FATAL_ERROR "bsp_add_image_signed: target '${target}' already exists")
    endif()
    get_property(_is_image TARGET ${raw_target} PROPERTY CH57X_IMAGE)
    if(NOT _is_image)
        message(FATAL_ERROR "bsp_add_image_signed: '${raw_target}' is not an image")
    endif()
    get_property(_bound TARGET ${raw_target} PROPERTY CH57X_LAYOUT_BOUND)
    if(NOT _bound)
        message(FATAL_ERROR "bsp_add_image_signed: '${raw_target}' has no layout")
    endif()
    if(NOT BSP_PYTHON_EXECUTABLE)
        message(FATAL_ERROR "bsp_add_image_signed: python3 is required for signing (set BSP_PYTHON_EXECUTABLE)")
    endif()
    get_property(_raw_elf TARGET ${raw_target} PROPERTY CH57X_ELF)

    set(_signed_elf "${CMAKE_BINARY_DIR}/${target}.elf")
    add_custom_command(OUTPUT "${_signed_elf}"
        COMMAND "${BSP_PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/image/sign_image.py"
                --input  "${_raw_elf}"
                --output "${_signed_elf}"
                --objcopy "${CMAKE_OBJCOPY}"
                --objdump "${CMAKE_OBJDUMP}"
                --magic "${BSP_CH57X_IMG_MAGIC}"
                --meta-section ".metainfo"
                --info-sections ".imginfo.magic,.imginfo.length,.imginfo.crc32"
        DEPENDS ${raw_target}
        COMMENT "bsp: sign ${raw_target} -> ${target}.elf"
        VERBATIM)

    add_custom_target(${target} ALL DEPENDS "${_signed_elf}")
    ch57x_image_register(${target} ELF "${_signed_elf}")
endfunction()

################################################################################
# @brief Produce a flashable Intel HEX from an image
################################################################################
function(bsp_add_hex binary_target elf_target)
    ch57x_add_hex(${binary_target} ${elf_target})
endfunction()

################################################################################
# @brief Merge images into one full image by address
################################################################################
function(bsp_add_manifest target)
    ch57x_add_manifest(${target} ${ARGN})
endfunction()
