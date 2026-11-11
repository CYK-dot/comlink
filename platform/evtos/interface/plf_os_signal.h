/**
 * @file    plf_os_signal.h
 * @brief   信号投递（同步直投 / 中断延迟投递）
 */
#ifndef PLF_OS_SIGNAL_H
#define PLF_OS_SIGNAL_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  同步投递信号，在调用者上下文立即执行目标的 on_sig
 * @param  sig 信号码，0 保留
 * @param  arg 附带数据
 * @return PLFOS_OK / PLFOS_E_INVAL / PLFOS_E_NOINST / PLFOS_E_BUSY
 * @note   目标正在执行时不重入，返回 PLFOS_E_BUSY 并计数。
 */
plfos_err_t plfos_signal(plfos_t *os, plfos_id_t to, uint32_t sig, uint32_t arg);

/**
 * @brief  中断上下文的信号：压入目标 FIFO，由主循环补派发
 * @note   FIFO 满返回 PLFOS_E_FULL；接收方看到的 from 为 PLFOS_ID_SYS。
 */
plfos_err_t plfos_signal_isr(plfos_t *os, plfos_id_t to, uint32_t sig, uint32_t arg);

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_SIGNAL_H */
