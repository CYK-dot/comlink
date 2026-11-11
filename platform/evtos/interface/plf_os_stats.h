/**
 * @file    plf_os_stats.h
 * @brief   统计计数：OS 只采集，不做输出
 */
#ifndef PLF_OS_STATS_H
#define PLF_OS_STATS_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PLFOS_ENABLE_STATS
/** @brief 单实例统计 */
typedef struct {
    uint32_t posted;            /**< 成功入队的 mbuf 数 */
    uint32_t delivered;         /**< 已派发的 mbuf 数 */
    uint32_t dropped_full;      /**< 因队列满被拒的 mbuf 数 */
    uint32_t dropped_noinst;    /**< 因目标不存在被拒的 mbuf 数 */
    uint32_t sig_delivered;     /**< 已派发的信号数 */
    uint32_t sig_dropped;       /**< 因 FIFO 满丢弃的信号数 */
    uint32_t sig_nested_reject; /**< 因重入被拒的信号数 */
    uint16_t queue_high_water;  /**< mbuf 队列历史最大深度 */
    uint16_t sig_high_water;    /**< 信号 FIFO 历史最大深度 */
} plfos_inst_stats_t;

/** @brief mbuf 池统计 */
typedef struct {
    uint32_t blocks_total; /**< 池切分出的块数 */
    uint32_t blocks_used;  /**< 当前已分配块数 */
    uint32_t blocks_peak;  /**< 历史峰值 */
    uint32_t alloc_fail;   /**< 分配失败次数 */
    uint32_t bytes_total;  /**< 池总字节 */
} plfos_pool_stats_t;

/** @brief 调度器级统计 */
typedef struct {
    uint32_t dispatch_count;    /**< 派发总次数 */
    uint32_t dispatch_max_depth;/**< 实际最大嵌套深度 */
    uint32_t idle_count;        /**< 空闲轮次 */
    uint32_t stack_high_water;  /**< 栈历史最大使用字节数 */
    uint32_t uptime_ms;         /**< 运行时长 */
} plfos_os_stats_t;

/** @brief 读取单实例统计，实例不存在返回 false */
bool plfos_stats_inst(const plfos_t *os, plfos_id_t id, plfos_inst_stats_t *out);

/** @brief 读取 mbuf 池统计 */
void plfos_stats_pool(const plfos_t *os, plfos_pool_stats_t *out);

/** @brief 读取调度器统计 */
void plfos_stats_os(const plfos_t *os, plfos_os_stats_t *out);

/** @brief 清零全部统计 */
void plfos_stats_reset(plfos_t *os);
#endif /* PLFOS_ENABLE_STATS */

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_STATS_H */
