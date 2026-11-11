/**
 * @file    plf_os_inst.h
 * @brief   服务实例：入口函数集、部署、查询与销毁
 */
#ifndef PLF_OS_INST_H
#define PLF_OS_INST_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 实例入口函数集 */
typedef struct {
    /** @brief mbuf 入口，所有权已转移，返回前须释放或转发 */
    void (*on_mbuf)(void *ctx, plfos_t *os, plfos_id_t from, plfos_mbuf_t *mbuf);
    /** @brief 信号入口，禁止阻塞 */
    void (*on_sig)(void *ctx, plfos_t *os, plfos_id_t from, uint32_t sig, uint32_t arg);
    /** @brief 可选，plfos_start() 时调用，返回非 0 使启动失败 */
    plfos_err_t (*on_init)(void *ctx, plfos_t *os);
    /** @brief 可选，实例销毁前调用 */
    void (*on_stop)(void *ctx, plfos_t *os);
} plfos_ops_t;

/** @brief 实例部署配置 */
typedef struct {
    uint8_t            type;   /**< 服务类型，1..PLFOS_MAX_TYPES-1 */
    uint8_t            index;  /**< 同类实例编号，0..PLFOS_MAX_INST_PER_TYPE-1 */
    const plfos_ops_t *ops;    /**< 入口集；NULL 时使用 type 登记的默认值 */
    void              *arg;    /**< 启动参数，作为入口函数的 ctx */
} plfos_inst_cfg_t;

/**
 * @brief  登记某 type 的默认入口函数集
 * @return PLFOS_OK / PLFOS_E_INVAL / PLFOS_E_EXIST
 * @note   须在该 type 的实例部署前调用。
 */
plfos_err_t plfos_type_register(plfos_t *os, uint8_t type, const plfos_ops_t *ops);

/**
 * @brief  部署一个服务实例
 * @param  cfg    配置，内容被拷贝，无需保持存活
 * @param  out_id 可选，返回带 generation 的实际句柄
 * @return PLFOS_OK / PLFOS_E_INVAL / PLFOS_E_EXIST / PLFOS_E_STATE / PLFOS_E_NOMEM
 * @note   运行中亦可调用。
 */
plfos_err_t plfos_inst_deploy(plfos_t *os, const plfos_inst_cfg_t *cfg, plfos_id_t *out_id);

/**
 * @brief  销毁实例
 * @note   依次调用 on_stop、释放队列中未处理的 mbuf、停用其名下定时器。
 */
plfos_err_t plfos_inst_remove(plfos_t *os, plfos_id_t id);

/**
 * @brief  按 type/index 查询当前存活实例的完整句柄
 * @note   静态部署可直接用 PLFOS_ID() 常量，无需查询。
 */
plfos_err_t plfos_id_resolve(const plfos_t *os, uint8_t type, uint8_t index, plfos_id_t *out_id);

/**
 * @brief  取当前正在派发的实例句柄
 * @return 不在入口函数内时返回 PLFOS_ID_INVALID
 */
plfos_id_t plfos_current_id(const plfos_t *os);

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_INST_H */
