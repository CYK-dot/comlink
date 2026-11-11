/**
 * @file    plf_os_timer.h
 * @brief   软定时器：到期向 owner 投递信号
 */
#ifndef PLF_OS_TIMER_H
#define PLF_OS_TIMER_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 定时器句柄（槽号，0 为无效） */
typedef uint8_t plfos_timer_t;

/** @brief 定时器启动参数 */
typedef struct {
    plfos_id_t owner;     /**< 到期后接收信号的实例 */
    uint32_t   sig;       /**< 到期投递的信号码 */
    uint32_t   arg;       /**< 到期携带的数据 */
    uint32_t   period_ms; /**< 周期或延时，毫秒，0 非法 */
    bool       periodic;  /**< true=周期触发，false=单次触发 */
} plfos_timer_cfg_t;

/**
 * @brief  启动或重启定时器
 * @param  tm 入/出参：*tm 为 0 时自动分配槽位，非 0 时复用该槽
 * @return PLFOS_OK / PLFOS_E_INVAL / PLFOS_E_NOMEM / PLFOS_E_NOINST
 * @note   到期不产生 mbuf，而是向 owner 投递 cfg 中的信号，from 为 PLFOS_ID_SYS；
 *         owner 销毁时其名下定时器自动停止。
 */
plfos_err_t plfos_timer_start(plfos_t *os, plfos_timer_t *tm, const plfos_timer_cfg_t *cfg);

/** @brief 停止定时器并归还槽位 */
plfos_err_t plfos_timer_stop(plfos_t *os, plfos_timer_t tm);

/** @brief 剩余时间（毫秒），未启动返回 0 */
uint32_t plfos_timer_remaining_ms(const plfos_t *os, plfos_timer_t tm);

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_TIMER_H */
