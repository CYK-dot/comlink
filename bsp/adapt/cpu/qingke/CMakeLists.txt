# bsp/adapt/cpu/qingke/CMakeLists.txt - WCH QingKe (RV32) core backend
#
# Header-only: a critical section runs on every event post and every queue
# operation, so the primitives are static inline in bsp_cpu_qingke.h and there
# is nothing to compile. The bsp.ARCH_QINGKE feature decides whether the BSP
# puts this directory on the include path.

add_library(bsp_cpu INTERFACE)
target_include_directories(bsp_cpu INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
