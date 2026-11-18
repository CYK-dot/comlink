/**
 * @file    osal_sched_base.h
 * @brief   Common scheduler interface shared by all OSAL schedulers.
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
 */
#ifndef OSAL_SCHED_BASE_H
#define OSAL_SCHED_BASE_H

#include <stdbool.h>
#include <stdint.h>

#include "osal_types.h"

/**
 * @brief Scheduler implementation type.
 */
typedef enum {
    OSAL_SCHED_TYPE_INVALID = 0, /**< Scheduler not initialised. */
    OSAL_SCHED_TYPE_SSQ = 1,     /**< Static single-queue scheduler. */
    OSAL_SCHED_TYPE_DSQ = 2,     /**< Dynamic single-queue scheduler (reserved). */
} osal_sched_type_t;

/**
 * @brief Opaque handle to any scheduler.
 *
 * The control block and the operation table are private to the OSAL
 * implementation; business code only uses this handle with the functions
 * below.
 */
typedef struct osal_sched_base *osal_sched_t;

/**
 * @brief Parameters of one software timer.
 */
typedef struct {
    uint8_t dest_iid;   /**< Service instance that receives the message. */
    uintptr_t msg;      /**< Message posted on expiry. */
    uint32_t period_ms; /**< Period in milliseconds, must be non-zero. */
    bool repeat;        /**< true for a periodic timer, false for one-shot. */
} osal_tmr_param_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read the implementation type of a scheduler.
 *
 * @param sched Scheduler handle, must not be NULL.
 * @return Implementation type.
 */
osal_sched_type_t osal_sched_type(osal_sched_t sched);

/**
 * @brief Read the caller-assigned identifier of a scheduler.
 *
 * @param sched Scheduler handle, must not be NULL.
 * @return Scheduler identifier.
 */
uint8_t osal_sched_id(osal_sched_t sched);

/**
 * @brief Post one message to a service instance.
 *
 * Safe to call from task and interrupt context.
 *
 * @param sched    Scheduler handle.
 * @param dest_iid Destination service instance.
 * @param msg      Four-byte message payload.
 * @return OSAL_OK, OSAL_EPARAM, OSAL_EFULL or OSAL_EDEAD.
 */
osal_err_t osal_sched_post_msg(osal_sched_t sched, uint8_t dest_iid,
                               uintptr_t msg);

/**
 * @brief Deliver a message to a service instance without queueing it.
 *
 * Runs the target callback in the caller's context and must not be called from
 * interrupt context.
 *
 * @param sched    Scheduler handle.
 * @param dest_iid Destination service instance.
 * @param msg      Four-byte message payload.
 * @return OSAL_OK, OSAL_EPARAM or OSAL_EISR.
 */
osal_err_t osal_sched_post_rtcmsg(osal_sched_t sched, uint8_t dest_iid,
                                  uintptr_t msg);

/**
 * @brief Publish one message to every service selected by a tag.
 *
 * Delivery is best effort: the call continues after a full destination queue
 * and returns OSAL_EFULL if at least one post failed. Safe to call from task
 * and interrupt context.
 *
 * @param sched   Scheduler handle.
 * @param fwd_tag Forwarding tag.
 * @param msg     Four-byte message payload.
 * @return OSAL_OK, OSAL_EPARAM or OSAL_EFULL.
 */
osal_err_t osal_sched_publish_msg(osal_sched_t sched, uint8_t fwd_tag,
                                  uintptr_t msg);

/**
 * @brief Deliver a published message without queueing it.
 *
 * Runs the target callbacks in the caller's context and must not be called from
 * interrupt context.
 *
 * @param sched   Scheduler handle.
 * @param fwd_tag Forwarding tag.
 * @param msg     Four-byte message payload.
 * @return OSAL_OK, OSAL_EPARAM or OSAL_EISR.
 */
osal_err_t osal_sched_publish_rtcmsg(osal_sched_t sched, uint8_t fwd_tag,
                                     uintptr_t msg);

/**
 * @brief Start a software timer.
 *
 * @param sched  Scheduler handle.
 * @param param  Timer parameters, must not be NULL.
 * @param tmr_id Receives the timer identifier on success.
 * @return OSAL_OK, OSAL_EPARAM, OSAL_ENOSLOT or OSAL_EISR.
 */
osal_err_t osal_sched_tmr_spawn(osal_sched_t sched,
                                const osal_tmr_param_t *param,
                                uint16_t *tmr_id);

/**
 * @brief Stop and release a software timer.
 *
 * @param sched  Scheduler handle.
 * @param tmr_id Timer identifier returned by osal_sched_tmr_spawn().
 * @return OSAL_OK, OSAL_EPARAM or OSAL_EISR.
 */
osal_err_t osal_sched_tmr_kill(osal_sched_t sched, uint16_t tmr_id);

/**
 * @brief Dispatch queued work.
 *
 * @param sched Scheduler handle.
 * @param max   Maximum number of messages to dispatch.
 * @return Number of messages dispatched; zero when nothing was ready.
 */
uint16_t osal_sched_pump(osal_sched_t sched, uint16_t max);

/**
 * @brief Discard all queued messages.
 *
 * @param sched Scheduler handle.
 * @return OSAL_OK or OSAL_EPARAM.
 */
osal_err_t osal_sched_flush(osal_sched_t sched);

/**
 * @brief Run the scheduler forever.
 *
 * Repeatedly pumps the queue and calls osal_port_idle() when no work is ready.
 * Never returns.
 *
 * @param sched Scheduler handle.
 */
void osal_sched_run(osal_sched_t sched);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_SCHED_BASE_H */
