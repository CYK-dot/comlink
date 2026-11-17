# platform/os/adapt/qpc/CMakeLists.txt - QP/C adapter for the platform layer
#
# Target:
#   platform_os         QP/C (QEP + QF + QV kernel) with this project's port
#
# QP/C is compiled in-tree into a static library and linked from the images, so
# it is always built with the application's own qp_config.h (include/) and the
# board's compile contract, which bsp_common carries.

get_filename_component(_PLATFORM_QPC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../qpc" ABSOLUTE)

if(NOT EXISTS "${_PLATFORM_QPC_DIR}/include/qpc.h")
    message(FATAL_ERROR
        "platform/os/adapt/qpc: QP/C submodule is missing at ${_PLATFORM_QPC_DIR}\n"
        "  Run: git submodule update --init platform/os/qpc")
endif()

if(NOT TARGET bsp_common)
    message(FATAL_ERROR
        "platform/os/adapt/qpc: target 'bsp_common' is missing.\n"
        "  add_subdirectory(bsp) must run before add_subdirectory(platform).")
endif()

if(NOT TARGET project_warnings)
    message(FATAL_ERROR
        "platform/os/adapt/qpc: target 'project_warnings' is missing.\n"
        "  The top-level CMakeLists.txt defines it before add_subdirectory(platform).")
endif()

# QP/C framework sources (QEP, QF) plus the cooperative QV kernel. QS (spy) is
# not built: Q_SPY is not defined anywhere in this product.
set(_PLATFORM_QPC_SRC
    "${_PLATFORM_QPC_DIR}/src/qf/qep_hsm.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qep_msm.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_act.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_actq.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_defer.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_dyn.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_mem.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_ps.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_qact.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_qeq.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_qmact.c"
    "${_PLATFORM_QPC_DIR}/src/qf/qf_time.c"
    "${_PLATFORM_QPC_DIR}/src/qv/qv.c")

if(NOT TARGET platform_os)
    add_library(platform_os STATIC
        ${_PLATFORM_QPC_SRC}
        "${CMAKE_CURRENT_LIST_DIR}/src/qpc_port.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/osal_qpc.c")
    add_library(platform_os_qpc ALIAS platform_os)

    # ../../inc is the public OSAL API (platform/os/inc); include/ carries
    # qp_config.h and the port header qp_port.h, which must come first because
    # QP/C includes "qp_port.h" by name.
    target_include_directories(platform_os PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/../../inc"
        "${CMAKE_CURRENT_LIST_DIR}/include")
    # QP/C's own headers are third-party: -isystem keeps warnings inside them
    # out of the application's -Werror.
    target_include_directories(platform_os SYSTEM PUBLIC
        "${_PLATFORM_QPC_DIR}/include")

    # The board contract (arch/ABI, section GC, optimisation, C standard), the
    # bsp include dirs and the CPU backend (bsp_cpu) bsp_common carries.
    target_link_libraries(platform_os PUBLIC bsp_common)
    target_link_libraries(platform_os PRIVATE project_warnings)
endif()
