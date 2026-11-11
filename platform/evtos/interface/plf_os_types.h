/**
 * @file    plf_os_types.h
 * @brief   evtOS 公共配置、错误码、实例句柄与前置声明
 */
#ifndef PLF_OS_TYPES_H
#define PLF_OS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLFOS_VERSION_MAJOR 0
#define PLFOS_VERSION_MINOR 1
#define PLFOS_VERSION_PATCH 0

/** @brief 可部署实例总数上限 */
#ifndef PLFOS_MAX_INSTANCES
#define PLFOS_MAX_INSTANCES 8
#endif

/** @brief 服务类型上限，type 取值 1..PLFOS_MAX_TYPES-1 */
#ifndef PLFOS_MAX_TYPES
#define PLFOS_MAX_TYPES 8
#endif

/** @brief 同 type 实例数上限，index 取值 0..PLFOS_MAX_INST_PER_TYPE-1 */
#ifndef PLFOS_MAX_INST_PER_TYPE
#define PLFOS_MAX_INST_PER_TYPE 4
#endif

/** @brief 每实例 mbuf 队列深度 */
#ifndef PLFOS_QUEUE_DEPTH
#define PLFOS_QUEUE_DEPTH 8
#endif

/** @brief 每实例待派发信号 FIFO 深度（中断/定时器发起） */
#ifndef PLFOS_SIG_FIFO_DEPTH
#define PLFOS_SIG_FIFO_DEPTH 4
#endif

/** @brief 定时器槽位上限 */
#ifndef PLFOS_MAX_TIMERS
#define PLFOS_MAX_TIMERS 8
#endif

/** @brief 单个 mbuf 数据区上限（字节） */
#ifndef PLFOS_MAX_MBUF_LEN
#define PLFOS_MAX_MBUF_LEN 248
#endif

/** @brief 信号级联最大嵌套深度，决定最坏栈耗 */
#ifndef PLFOS_MAX_DISPATCH_DEPTH
#define PLFOS_MAX_DISPATCH_DEPTH 4
#endif

/** @brief 调度器对象存储大小，随上述上限变化 */
#ifndef PLFOS_SCHED_SIZE
#define PLFOS_SCHED_SIZE 1024
#endif

/** @brief 1=编译统计计数，0=全部去除 */
#ifndef PLFOS_ENABLE_STATS
#define PLFOS_ENABLE_STATS 1
#endif

/** @brief 1=编译 trace 钩子，0=全部去除 */
#ifndef PLFOS_ENABLE_TRACE
#define PLFOS_ENABLE_TRACE 0
#endif

/** @brief 1=开启 PLFOS_ASSERT */
#ifndef PLFOS_ENABLE_ASSERT
#define PLFOS_ENABLE_ASSERT 1
#endif

/** @brief 错误码 */
typedef int16_t plfos_err_t;

enum {
    PLFOS_OK          =  0,   /**< 成功 */
    PLFOS_E_INVAL     = -1,   /**< 参数非法 */
    PLFOS_E_NOINST    = -2,   /**< 实例不存在或句柄已失效 */
    PLFOS_E_FULL      = -3,   /**< 队列或信号 FIFO 已满 */
    PLFOS_E_NOMEM     = -4,   /**< mbuf 池或定时器槽位耗尽 */
    PLFOS_E_BUSY      = -5,   /**< 目标实例正在执行，信号被重入保护拒绝 */
    PLFOS_E_STATE     = -6,   /**< 当前状态不允许该操作 */
    PLFOS_E_TOOBIG    = -7,   /**< mbuf 长度超限 */
    PLFOS_E_POOL      = -8,   /**< mbuf 不属于该调度器的池 */
    PLFOS_E_EXIST     = -9,   /**< 实例或 type 已注册 */
};

/** @brief 服务实例句柄：generation[23:16] | index[15:8] | type[7:0] */
typedef uint32_t plfos_id_t;

#define PLFOS_ID_SYS        ((plfos_id_t)0x00000000u)  /**< 系统伪发送者：定时器、中断 */
#define PLFOS_ID_INVALID    ((plfos_id_t)0xFFFFFFFFu)  /**< 非法句柄 */

/** @brief 由 type/index 构造句柄，generation=0，可作编译期常量 */
#define PLFOS_ID(t, i) \
    ((plfos_id_t)(((uint32_t)(uint8_t)(t)) | (((uint32_t)(uint8_t)(i)) << 8)))

#define PLFOS_ID_TYPE(h)     ((uint8_t)((uint32_t)(h) & 0xFFu))
#define PLFOS_ID_INDEX(h)    ((uint8_t)(((uint32_t)(h) >> 8) & 0xFFu))
#define PLFOS_ID_GEN(h)      ((uint8_t)(((uint32_t)(h) >> 16) & 0xFFu))
#define PLFOS_ID_IS_VALID(h) ((h) != PLFOS_ID_INVALID)

/** @brief 调度器句柄（不透明，存储在调用者侧） */
typedef struct plfos_sched plfos_t;

/** @brief 消息缓冲区 mbuf（不透明，仅能由 plfos_mbuf_alloc() 创建） */
typedef struct plfos_mbuf plfos_mbuf_t;

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_TYPES_H */
