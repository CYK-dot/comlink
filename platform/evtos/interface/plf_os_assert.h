/**
 * @file    plf_os_assert.h
 * @brief   断言与致命错误处理
 */
#ifndef PLF_OS_ASSERT_H
#define PLF_OS_ASSERT_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  致命错误处理，弱符号，platform 层可覆盖
 * @note   默认实现关中断后停机。
 */
void plfos_panic(const char *reason, const char *file, uint16_t line);

#if PLFOS_ENABLE_ASSERT
#define PLFOS_ASSERT(cond)                                                   \
    do {                                                                     \
        if (!(cond)) {                                                       \
            plfos_panic("assert: " #cond, __FILE__, (uint16_t)__LINE__);     \
        }                                                                    \
    } while (0)
#else
#define PLFOS_ASSERT(cond) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_ASSERT_H */
