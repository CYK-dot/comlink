/**
 * @file    osal_sched_base.c
 * @brief   Dispatch layer for the common scheduler interface.
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
#include "osal_sched_base_priv.h"
#include "osal_port.h"

/**
 * @brief Check that a scheduler handle can be dispatched.
 *
 * @param sched Scheduler handle.
 * @return true when the handle and its operation table are valid.
 */
static bool sched_valid(osal_sched_t sched)
{
    return (sched != (osal_sched_t)0)
           && (sched->vtbl != (const osal_sched_vtbl_t *)0);
}

osal_sched_type_t osal_sched_type(osal_sched_t sched)
{
    return (osal_sched_type_t)sched->sched_type;
}

uint8_t osal_sched_id(osal_sched_t sched)
{
    return sched->sid;
}

osal_err_t osal_sched_post_msg(osal_sched_t sched, uint8_t dest_iid,
                               uintptr_t msg)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->post_msg(sched, dest_iid, msg);
}

osal_err_t osal_sched_post_rtcmsg(osal_sched_t sched, uint8_t dest_iid,
                                  uintptr_t msg)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->post_rtcmsg(sched, dest_iid, msg);
}

osal_err_t osal_sched_publish_msg(osal_sched_t sched, uint8_t fwd_tag,
                                  uintptr_t msg)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->publish_msg(sched, fwd_tag, msg);
}

osal_err_t osal_sched_publish_rtcmsg(osal_sched_t sched, uint8_t fwd_tag,
                                     uintptr_t msg)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->publish_rtcmsg(sched, fwd_tag, msg);
}

osal_err_t osal_sched_tmr_spawn(osal_sched_t sched,
                                const osal_tmr_param_t *param,
                                uint16_t *tmr_id)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->tmr_spawn(sched, param, tmr_id);
}

osal_err_t osal_sched_tmr_kill(osal_sched_t sched, uint16_t tmr_id)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->tmr_kill(sched, tmr_id);
}

uint16_t osal_sched_pump(osal_sched_t sched, uint16_t max)
{
    if (!sched_valid(sched)) {
        return 0u;
    }
    return sched->vtbl->pump(sched, max);
}

osal_err_t osal_sched_flush(osal_sched_t sched)
{
    if (!sched_valid(sched)) {
        return OSAL_EPARAM;
    }
    return sched->vtbl->flush(sched);
}

void osal_sched_run(osal_sched_t sched)
{
    for (;;) {
        if (osal_sched_pump(sched, UINT16_MAX) == 0u) {
            osal_port_idle();
        }
    }
}
