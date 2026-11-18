# platform/os/CMakeLists.txt - OSAL implementation
#
# Public headers in inc/, private layout headers and implementation in src/,
# CPU/BSP port in adapt/port/.

add_library(platform_os STATIC
    "${CMAKE_CURRENT_LIST_DIR}/src/osal_sched_base.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/osal_sched_sq.c"
    "${CMAKE_CURRENT_LIST_DIR}/adapt/port/osal_port.c"
)

target_include_directories(platform_os PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/inc"
)

target_include_directories(platform_os PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

target_link_libraries(platform_os PUBLIC bsp_common)
target_link_libraries(platform_os PRIVATE project_warnings)
