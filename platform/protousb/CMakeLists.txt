# platform/protousb - TinyUSB device stack integration
#
# The controller driver (DCD) is project-owned and lives in adapt/<chip>/; the
# tinyusb submodule stays a clean upstream checkout. See README.md.

set(_PROTOUSB_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(_TINYUSB_DIR  "${_PROTOUSB_DIR}/tinyusb")

if(NOT EXISTS "${_TINYUSB_DIR}/src/tusb.c")
    message(FATAL_ERROR "platform/protousb: tinyusb submodule is missing")
endif()

if(BSP_FAMILY STREQUAL "ch57x")
    set(_USB_CHIP_DIR "${_PROTOUSB_DIR}/adapt/ch57x")
    set(_USB_CHIP_SRC
        "${_USB_CHIP_DIR}/usb_dcd_ch57x.c"
        "${_USB_CHIP_DIR}/usb_ch57x.c")
    # CH57x is the same USBFS IP family as CH58x, minus EP5-7.
    set(_USB_MCU_OPT "OPT_MCU_CH583")
    set(_USB_CHIP_DEFS "CH57X_USBFS=1")
else()
    set(_USB_CHIP_DIR "${_PROTOUSB_DIR}/adapt/ch32x035")
    set(_USB_CHIP_SRC
        "${_USB_CHIP_DIR}/usb_dcd_ch32x035.c"
        "${_USB_CHIP_DIR}/usb_ch32x035.c")
    set(_USB_MCU_OPT "OPT_MCU_CH32X035")
    set(_USB_CHIP_DEFS "")
endif()

add_library(platform_usb STATIC
    # TinyUSB core (DCD/HCD are deliberately not part of tinyusb_sources_get)
    "${_TINYUSB_DIR}/src/tusb.c"
    "${_TINYUSB_DIR}/src/common/tusb_fifo.c"
    "${_TINYUSB_DIR}/src/device/usbd.c"
    "${_TINYUSB_DIR}/src/class/cdc/cdc_device.c"
    # Project glue: descriptors, facade, chip DCD and chip USB glue
    "${_PROTOUSB_DIR}/adapt/tinyusb/usb_descriptors.c"
    "${_PROTOUSB_DIR}/adapt/tinyusb/palusb.c"
    ${_USB_CHIP_SRC}
)

target_include_directories(platform_usb PUBLIC
    "${_PROTOUSB_DIR}/inc"
    "${_PROTOUSB_DIR}/adapt/tinyusb"
)

target_include_directories(platform_usb PRIVATE
    "${_USB_CHIP_DIR}"
)

target_include_directories(platform_usb SYSTEM PUBLIC
    "${_TINYUSB_DIR}/src"
)

target_compile_definitions(platform_usb PUBLIC CFG_TUSB_MCU=${_USB_MCU_OPT})

if(NOT "${_USB_CHIP_DEFS}" STREQUAL "")
    target_compile_definitions(platform_usb PUBLIC ${_USB_CHIP_DEFS})
endif()

target_link_libraries(platform_usb PRIVATE bsp_chip)

