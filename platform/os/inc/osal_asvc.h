/**
 * @file    osal_asvc.h
 * @brief   Class declaration of an asynchronous service.
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
#ifndef OSAL_ASVC_H
#define OSAL_ASVC_H

#include <stdbool.h>
#include <stdint.h>

#include "osal_sched_base.h"

/**
 * @brief Class declaration of an asynchronous service.
 *
 * A class object is immutable and normally placed in flash. It describes how
 * instances are constructed, destroyed and how they consume messages.
 */
typedef struct {
    /**
     * @brief Construct an instance inside its private memory.
     *
     * @param pthis      Start of the private memory reserved by the scheduler.
     * @param param      Construction parameter, may be NULL when size is zero.
     * @param param_size Size of the construction parameter in bytes.
     * @return true on success, false to reject construction.
     */
    bool (*ctor)(void *pthis, const void *param, uint16_t param_size);

    /**
     * @brief Destroy an instance.
     *
     * Called once per successfully constructed instance when the scheduler is
     * destroyed. Must release everything the instance owns.
     *
     * @param pthis Start of the private memory.
     */
    void (*dtor)(void *pthis);

    /**
     * @brief Consume one queued message.
     *
     * Called by the scheduler pump in the service turn. The message is a
     * four-byte value; the instance may not modify anything outside its
     * private memory except through the OSAL API.
     *
     * @param pthis Private memory of the instance.
     * @param sched Scheduler that dispatched the message.
     * @param msg   Message payload.
     */
    void (*msg_cb)(void *pthis, osal_sched_t sched, uintptr_t msg);

    /**
     * @brief Consume one run-to-completion message.
     *
     * Called directly by osal_schedssq_post_rtcmsg() and
     * osal_schedssq_publish_rtcmsg() in the caller's context, not by the
     * scheduler pump. May be NULL when the service does not use the
     * run-to-completion path.
     *
     * @param pthis Private memory of the instance.
     * @param sched Scheduler the message is delivered through.
     * @param msg   Message payload.
     */
    void (*rtc_msg_cb)(void *pthis, osal_sched_t sched, uintptr_t msg);
} osal_asvc_class_t;

#endif /* OSAL_ASVC_H */
