/**
 * @file    plf_os_mbuf.h
 * @brief   mbuf 分配、访问与投递
 *
 * @note    mbuf 为转移语义：plfos_post() 成功后所有权归接收方，由其释放或转发。
 */
#ifndef PLF_OS_MBUF_H
#define PLF_OS_MBUF_H

#include "plf_os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  从调度器的 mbuf 池分配一个 mbuf
 * @param  type 业务类型，原样透传给接收方
 * @param  len  数据区长度，0..PLFOS_MAX_MBUF_LEN
 * @return mbuf 指针；池耗尽或长度超限返回 NULL
 * @note   数据区未初始化；仅可在任务上下文调用。
 */
plfos_mbuf_t *plfos_mbuf_alloc(plfos_t *os, uint8_t type, uint16_t len);

/**
 * @brief  释放 mbuf，归还所属池
 * @param  mbuf 可为 NULL
 */
void plfos_mbuf_free(plfos_t *os, plfos_mbuf_t *mbuf);

/** @brief 取数据区可写指针 */
void *plfos_mbuf_data(plfos_mbuf_t *mbuf);

/** @brief 取数据区长度 */
uint16_t plfos_mbuf_len(const plfos_mbuf_t *mbuf);

/** @brief 取业务类型 */
uint8_t plfos_mbuf_type(const plfos_mbuf_t *mbuf);

/**
 * @brief  异步投递 mbuf，所有权转移给接收方
 * @return PLFOS_OK / PLFOS_E_INVAL / PLFOS_E_NOINST / PLFOS_E_FULL / PLFOS_E_POOL
 * @note   失败时所有权不转移，调用者仍需自行释放。
 *         中断上下文请用 plfos_post_isr()；允许投递给自身。
 */
plfos_err_t plfos_post(plfos_t *os, plfos_id_t to, plfos_mbuf_t *mbuf);

/**
 * @brief  中断上下文的投递：只入队，不派发
 * @note   mbuf 需在任务上下文预分配，中断内不做分配。
 */
plfos_err_t plfos_post_isr(plfos_t *os, plfos_id_t to, plfos_mbuf_t *mbuf);

#ifdef __cplusplus
}
#endif

#endif /* PLF_OS_MBUF_H */
