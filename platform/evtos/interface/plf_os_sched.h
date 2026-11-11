/**
 * @file    plf_os_sched.h
 * @brief   调度器生命周期、主循环与时基
 *
 * @note    调度器单线程、run-to-completion，所有实例共享一条栈；
 *          入口函数内禁止阻塞，延时请用 plfos_timer_start()。
 * @note    实例固定属于一个调度器，mbuf 不可跨调度器投递。
 */
#ifndef PLF_OS_SCHED_H
#define PLF_OS_SCHED_H

#include "plf_os_types.h"
#include "plf_os_inst.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 调度器对象存储，自带 8 字节对齐 */
typedef union {
    uint64_t _align;
    uint8_t  bytes[PLFOS_SCHED_SIZE];
} plfos_sched_storage_t;

/** @brief 调度器初始化参数 */
typedef struct {
    void                   *pool_mem;      /**< mbuf 池，8 字节对齐，由调用者提供 */
    uint32_t                pool_size;     /**< 池字节数 */
    const plfos_inst_cfg_t *defaults;      /**< 可选，init 时自动部署的实例表 */
    uint16_t                default_count; /**< defaults 条目数，0 表示不部署 */
} plfos_cfg_t;

/**
 * @brief  初始化调度器并部署默认实例
 * @param  store 调用者提供的存储，内容会被清零
 * @return 调度器指针；参数非法返回 NULL
 */
plfos_t *plfos_init(plfos_sched_storage_t *store, const plfos_cfg_t *cfg);

/** @brief 反初始化：销毁所有实例与定时器，清空 mbuf 池 */
void plfos_deinit(plfos_t *os);

/**
 * @brief  启动：依次调用各实例的 on_init
 * @return 首个失败实例的错误码，此时已初始化的实例会被 on_stop 回滚
 */
plfos_err_t plfos_start(plfos_t *os);

/** @brief 主循环，不返回：等价于循环调用 plfos_poll()，空闲时调用 idle 钩子 */
void plfos_run(plfos_t *os);

/**
 * @brief  单步调度，用于自管主循环或 PC 端测试
 * @return true=本轮派发过事件，false=无事可做
 */
bool plfos_poll(plfos_t *os);

/** @brief 设置空闲钩子，可用于低功耗；钩子内禁止调用派发类接口 */
void plfos_set_idle_hook(plfos_t *os, void (*fn)(void *user), void *user);

/**
 * @brief  推进 1ms 时基，扫描到期定时器
 * @note   可在 SysTick 中断调用；存在多个调度器时需各自调用。
 */
void plfos_tick(plfos_t *os);

/** @brief 自 plfos_init() 起的毫秒数，32 位回绕 */
uint32_t plfos_uptime_ms(const plfos_t *os);

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_SCHED_H */
