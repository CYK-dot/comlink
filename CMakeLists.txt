cmake_minimum_required(VERSION 3.20)

# ---- 交叉工具链（WCH RISC-V GCC 15.2，需环境变量 WCH_TOOLCHAIN_ROOT_GCC15_2）----
set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/sdk/ch57x/cmake/toolchain-riscv-wch.cmake
    CACHE FILEPATH "WCH RISC-V toolchain file")

project(daplink_com C ASM)

# ---- BSP：极简模式 -----------------------------------------------------------
# 只链接 ch57x 标准库 wch_periph，不引入任何板级封装；
# 启动文件与链接脚本全部使用 BSP 内置版本（startup_CH572.S + 生成的 CH572.ld）。
add_subdirectory(bsp/sdk/ch57x)

add_executable(daplink_com.elf
    main/main.c
)

target_link_libraries(daplink_com.elf PRIVATE wch_periph)
target_link_wch_startup(daplink_com.elf CH572)   # 内置启动文件 + 内置链接脚本
wch_generate_hex(daplink_com.elf CH572)          # 产出 .hex 并打印各段大小
