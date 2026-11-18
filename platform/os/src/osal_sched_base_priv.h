/**
 * @file    osal_sched_base_priv.h
 * @brief   Private control block and operation table of OSAL schedulers.
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
 * This header is private to the OSAL implementation. It defines how a
 * scheduler dispatches its common operations and must never be included by
 * application code.
 */
#ifndef OSAL_SCHED_BASE_PRIV_H
#define OSAL_SCHED_BASE_PRIV_H

#include "osal_sched_base.h"

/**
 * @brief Operations every scheduler must implement.
 *
 * create, destroy and tracing are scheduler-specific and are therefore not part
 * of this table.
 */
typedef struct osal_sched_vtbl {
    /** @brief Post one message to a service instance. */
    osal_err_t (*post_msg)(osal_sched_t sched, uint8_t dest_iid, uintptr_t msg);

    /** @brief Deliver one message without queueing it. */
    osal_err_t (*post_rtcmsg)(osal_sched_t sched, uint8_t dest_iid, uintptr_t msg);

    /** @brief Publish one message to every service selected by a tag. */
    osal_err_t (*publish_msg)(osal_sched_t sched, uint8_t fwd_tag, uintptr_t msg);

    /** @brief Deliver a published message without queueing it. */
    osal_err_t (*publish_rtcmsg)(osal_sched_t sched, uint8_t fwd_tag, uintptr_t msg);

    /** @brief Start a software timer. */
    osal_err_t (*tmr_spawn)(osal_sched_t sched, const osal_tmr_param_t *param,
                            uint16_t *tmr_id);

    /** @brief Stop and release a software timer. */
    osal_err_t (*tmr_kill)(osal_sched_t sched, uint16_t tmr_id);

    /** @brief Dispatch queued work; returns the number of messages dispatched. */
    uint16_t (*pump)(osal_sched_t sched, uint16_t max);

    /** @brief Discard all queued messages. */
    osal_err_t (*flush)(osal_sched_t sched);
} osal_sched_vtbl_t;

/**
 * @brief Common header placed first in every scheduler object.
 *
 * The first-member rule lets a concrete scheduler pointer be converted to
 * osal_sched_t and back without knowing the concrete layout.
 */
typedef struct osal_sched_base osal_sched_base_t;

struct osal_sched_base {
    const osal_sched_vtbl_t *vtbl; /**< Scheduler-specific operations. */
    uint8_t sched_type;            /**< One of osal_sched_type_t. */
    uint8_t sid;                   /**< Caller-assigned scheduler identifier. */
};

#endif /* OSAL_SCHED_BASE_PRIV_H */
