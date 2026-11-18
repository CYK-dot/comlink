/**
 * @file    osal_sched_sq_priv.h
 * @brief   Private layout of the static single-queue scheduler.
 * @author  CYK-Dot <chenyukai@xmurcs.cn>
 * @date    2026-11-18
 *
 * SPDX-FileCopyrightText: 2026 CYK-Dot <chenyukai@xmurcs.cn>
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * This header is private to the OSAL implementation. It defines the workspace
 * layout that osal_sched_sq.c maintains and must never be included by
 * application code.
 */
#ifndef OSAL_SCHED_SQ_PRIV_H
#define OSAL_SCHED_SQ_PRIV_H

#include <stdint.h>

#include "osal_sched_sq.h"
#include "osal_sched_base_priv.h"

/** @brief One queued message: payload plus destination service. */
struct osal_schedssq_msg {
    uint32_t msg;      /**< Four-byte message payload. */
    uint16_t dest_iid; /**< Destination service instance. */
    uint16_t reserved; /**< Keeps the slot size a multiple of four. */
};

OSAL_STATIC_ASSERT(sizeof(struct osal_schedssq_msg) == OSAL_SCHEDSSQ_QUEUE_ELEM_SIZE,
                   "queued message size must match OSAL_SCHEDSSQ_QUEUE_ELEM_SIZE");

/** @brief One software timer record. */
struct osal_schedssq_tmr {
    uint32_t deadline; /**< Absolute tick at which the timer expires. */
    uint32_t period;   /**< Period in ticks; zero for a one-shot timer. */
    uint32_t msg;      /**< Message posted on expiry. */
    uint8_t dest_iid;  /**< Destination service instance. */
    uint8_t used;      /**< Non-zero while the record is allocated. */
};

OSAL_STATIC_ASSERT(sizeof(struct osal_schedssq_tmr) <= OSAL_SCHEDSSQ_TMR_MAX,
                   "timer record exceeds OSAL_SCHEDSSQ_TMR_MAX");

/**
 * @brief Scheduler control block stored at the start of the workspace.
 *
 * Layout inside the workspace:
 * control block, service offset table, service private memory, timer table and
 * message queue. The service offset table starts exactly at
 * sizeof(struct osal_schedssq) and holds one uint16_t workspace offset per
 * service instance; its length is deptbl->entry_cnt.
 *
 * All offsets are byte offsets into the workspace, which limits the workspace
 * to 64 KiB. The queue head and tail are slot indices, not byte offsets.
 */
struct osal_schedssq {
    osal_sched_base_t base;     /**< Must stay the first member. */
    uint16_t asvc_ctx_start;    /**< Offset of the private memory region. */
    uint16_t tmr_cnt;           /**< Number of timer records. */
    uint16_t tmr_tbl;           /**< Offset of the timer table. */
    uint16_t msgq_start;        /**< Offset of the first message slot. */
    uint16_t msgq_head;         /**< Slot index of the next message to dispatch. */
    uint16_t msgq_tail;         /**< Slot index of the next free slot. */
    uint16_t msgq_used;         /**< Number of queued messages. */
    uint16_t msgq_cap;          /**< Queue capacity in messages. */
    const osal_asvc_deploy_tbl_t *deptbl; /**< Read-only deployment table. */
    const osal_tagfwd_tbl_t *fwdtbl;      /**< Read-only forwarding table. */
};

OSAL_STATIC_ASSERT(sizeof(struct osal_schedssq) <= OSAL_SCHEDSSQ_CTRL_MAX,
                   "control block exceeds OSAL_SCHEDSSQ_CTRL_MAX");

/**
 * @brief Convert a public scheduler handle to its concrete control block.
 *
 * @param sched Scheduler handle.
 * @return Pointer to the static single-queue control block.
 */
static inline struct osal_schedssq *osal_schedssq_from_base(osal_sched_t sched)
{
    return (struct osal_schedssq *)sched;
}

/**
 * @brief Locate the service offset table of a scheduler.
 *
 * The table follows the control block and holds deptbl->entry_cnt uint16_t
 * workspace offsets, indexed by service instance identifier.
 *
 * @param pthis Concrete scheduler control block.
 * @return Pointer to the first offset table entry.
 */
static inline const uint16_t *osal_schedssq_asvc_off_tbl(
    const struct osal_schedssq *pthis)
{
    return (const uint16_t *)((const uint8_t *)pthis
                              + sizeof(struct osal_schedssq));
}

/**
 * @brief Locate one timer record.
 *
 * @param pthis Concrete scheduler control block.
 * @param index Record index, below pthis->tmr_cnt.
 * @return Pointer to the timer record.
 */
static inline struct osal_schedssq_tmr *osal_schedssq_tmr_at(
    struct osal_schedssq *pthis, uint16_t index)
{
    return &((struct osal_schedssq_tmr *)((uint8_t *)pthis + pthis->tmr_tbl))[index];
}

/**
 * @brief Locate one message slot.
 *
 * @param pthis Concrete scheduler control block.
 * @param index Slot index, below pthis->msgq_cap.
 * @return Pointer to the message slot.
 */
static inline struct osal_schedssq_msg *osal_schedssq_msg_at(
    struct osal_schedssq *pthis, uint16_t index)
{
    return &((struct osal_schedssq_msg *)((uint8_t *)pthis
                                          + pthis->msgq_start))[index];
}

/**
 * @brief Locate the private memory of one service instance.
 *
 * @param pthis Concrete scheduler control block.
 * @param iid   Service instance identifier.
 * @return Pointer to the private memory.
 */
static inline void *osal_schedssq_pthis(struct osal_schedssq *pthis, uint16_t iid)
{
    uint16_t const off = osal_schedssq_asvc_off_tbl(pthis)[iid];
    return (uint8_t *)pthis + off;
}

#endif /* OSAL_SCHED_SQ_PRIV_H */
