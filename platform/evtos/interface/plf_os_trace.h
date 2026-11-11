/**
 * @file    plf_os_trace.h
 * @brief   事件 trace 钩子
 */
#ifndef PLF_OS_TRACE_H
#define PLF_OS_TRACE_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PLFOS_ENABLE_TRACE
/** @brief trace 事件类型 */
typedef enum {
    PLFOS_TRACE_POST        = 1, /**< a=目标句柄 b=mbuf 长度 */
    PLFOS_TRACE_POST_FAIL   = 2, /**< a=目标句柄 b=错误码 */
    PLFOS_TRACE_SIGNAL      = 3, /**< a=目标句柄 b=sig */
    PLFOS_TRACE_SIGNAL_FAIL = 4, /**< a=目标句柄 b=错误码 */
    PLFOS_TRACE_DISPATCH    = 5, /**< a=实例句柄 b=type 或 sig */
    PLFOS_TRACE_DROP        = 6, /**< a=实例句柄 b=原因 */
    PLFOS_TRACE_TIMER       = 7, /**< a=实例句柄 b=槽号 */
} plfos_trace_ev_t;

/** @brief trace 钩子，在事件发生点同步回调；仅可记录，禁止阻塞与重入派发 */
typedef void (*plfos_trace_fn)(void *user, uint8_t ev, plfos_id_t id, uint32_t a, uint32_t b);

/** @brief 设置 trace 钩子 */
void plfos_trace_set(plfos_t *os, plfos_trace_fn fn, void *user);
#endif /* PLFOS_ENABLE_TRACE */

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_TRACE_H */
