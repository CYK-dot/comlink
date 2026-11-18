/**
 * @file    osal_sched_sq.c
 * @brief   Static single-queue scheduler implementation.
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
#include <string.h>

#include "osal_sched_sq_priv.h"
#include "osal_port.h"

#define OSAL_SCHEDSSQ_TICKS_PER_MS \
    ((uint32_t)OSAL_PORT_TICK_HZ / 1000u)

/**
 * @brief Round an offset up to an alignment boundary.
 *
 * @param value Offset in bytes.
 * @param align Alignment in bytes, power of two.
 * @return Aligned offset.
 */
static uint32_t align_up(uint32_t value, uint32_t align)
{
    return (value + (align - 1u)) & ~(align - 1u);
}

/**
 * @brief Convert a millisecond period to scheduler ticks.
 *
 * @param period_ms Period in milliseconds.
 * @return Period in ticks, at least one tick.
 */
static uint32_t period_to_ticks(uint32_t period_ms)
{
    uint32_t const hz = (uint32_t)OSAL_PORT_TICK_HZ;

    if ((hz % 1000u) == 0u) {
        uint32_t const factor = hz / 1000u;
        return (period_ms == 0u) ? 0u : (period_ms * factor);
    }

    return (uint32_t)(((uint64_t)period_ms * (uint64_t)hz + 999u) / 1000u);
}

/**
 * @brief Check that a handle refers to a live single-queue scheduler.
 *
 * @param sched Scheduler handle.
 * @return true when the handle is a static single-queue scheduler.
 */
static bool sched_is_ssq(osal_sched_t sched);

static osal_err_t sq_post_msg(osal_sched_t sched, uint8_t dest_iid, uintptr_t msg);
static osal_err_t sq_post_rtcmsg(osal_sched_t sched, uint8_t dest_iid, uintptr_t msg);
static osal_err_t sq_publish_msg(osal_sched_t sched, uint8_t fwd_tag, uintptr_t msg);
static osal_err_t sq_publish_rtcmsg(osal_sched_t sched, uint8_t fwd_tag, uintptr_t msg);
static osal_err_t sq_tmr_spawn(osal_sched_t sched, const osal_tmr_param_t *param,
                               uint16_t *tmr_id);
static osal_err_t sq_tmr_kill(osal_sched_t sched, uint16_t tmr_id);
static uint16_t sq_pump(osal_sched_t sched, uint16_t max);
static osal_err_t sq_flush(osal_sched_t sched);

/** @brief Operation table exposed through osal_sched_base_t::vtbl. */
static const osal_sched_vtbl_t s_ssq_vtbl = {
    .post_msg = sq_post_msg,
    .post_rtcmsg = sq_post_rtcmsg,
    .publish_msg = sq_publish_msg,
    .publish_rtcmsg = sq_publish_rtcmsg,
    .tmr_spawn = sq_tmr_spawn,
    .tmr_kill = sq_tmr_kill,
    .pump = sq_pump,
    .flush = sq_flush,
};

static bool sched_is_ssq(osal_sched_t sched)
{
    return (sched != (osal_sched_t)0)
           && (sched->sched_type == (uint8_t)OSAL_SCHED_TYPE_SSQ)
           && (sched->vtbl == &s_ssq_vtbl);
}

/**
 * @brief Append one message to the shared queue.
 *
 * @param pthis    Concrete scheduler.
 * @param dest_iid Destination service instance.
 * @param msg      Four-byte message payload.
 * @return OSAL_OK, OSAL_EPARAM or OSAL_EFULL.
 */
static osal_err_t enqueue_msg(struct osal_schedssq *pthis, uint8_t dest_iid,
                              uint32_t msg)
{
    bool full = false;

    if (dest_iid >= pthis->deptbl->entry_cnt) {
        return OSAL_EPARAM;
    }

    osal_port_enter_critical();
    if (pthis->msgq_used >= pthis->msgq_cap) {
        full = true;
    } else {
        struct osal_schedssq_msg * const slot =
            osal_schedssq_msg_at(pthis, pthis->msgq_tail);
        slot->msg = msg;
        slot->dest_iid = dest_iid;
        pthis->msgq_tail = (uint16_t)((pthis->msgq_tail + 1u) % pthis->msgq_cap);
        ++pthis->msgq_used;
    }
    osal_port_quit_critical();

    return full ? OSAL_EFULL : OSAL_OK;
}

osal_err_t osal_schedssq_create(uint8_t sid,
                                const osal_schedssq_ctorparam_t *param,
                                osal_sched_t *out)
{
    struct osal_schedssq *pthis;
    uint16_t const entry_cnt = (param != (const osal_schedssq_ctorparam_t *)0)
                               && (param->deptbl != (const osal_asvc_deploy_tbl_t *)0)
                               ? param->deptbl->entry_cnt : 0u;
    uint32_t max_align = 4u;
    uint32_t off;
    uint16_t off_tbl;
    uint16_t ctx_start;
    uint16_t *offsets;
    uint16_t i;

    if ((param == (const osal_schedssq_ctorparam_t *)0)
        || (out == (osal_sched_t *)0)
        || (param->workspace == (void *)0)
        || (param->deptbl == (const osal_asvc_deploy_tbl_t *)0)
        || (param->deptbl->entries == (const osal_asvc_deploy_t *)0)
        || (entry_cnt == 0u)
        || (entry_cnt > OSAL_SCHEDSSQ_MAX_ASVC_CNT)
        || (param->msgq_max_len == 0u)) {
        return OSAL_EPARAM;
    }
    if (((uint32_t)(uintptr_t)param->workspace & (OSAL_SCHEDSSQ_ALIGN - 1u)) != 0u) {
        return OSAL_EPARAM;
    }

    for (i = 0u; i < entry_cnt; ++i) {
        const osal_asvc_deploy_t * const entry = &param->deptbl->entries[i];
        uint32_t const align = (entry->pthis_align == 0u)
                               ? 4u : (uint32_t)entry->pthis_align;

        if ((entry->base == (const osal_asvc_class_t *)0)
            || (align > (uint32_t)OSAL_SCHEDSSQ_ALIGN)
            || ((align & (align - 1u)) != 0u)) {
            return OSAL_EPARAM;
        }
        if (align > max_align) {
            max_align = align;
        }
    }

    pthis = (struct osal_schedssq *)param->workspace;

    off = (uint32_t)sizeof(struct osal_schedssq);
    off = align_up(off, 2u);
    off_tbl = (uint16_t)off;
    off += (uint32_t)entry_cnt * 2u;

    off = align_up(off, max_align);
    ctx_start = (uint16_t)off;
    for (i = 0u; i < entry_cnt; ++i) {
        const osal_asvc_deploy_t * const entry = &param->deptbl->entries[i];
        uint32_t const align = (entry->pthis_align == 0u)
                               ? 4u : (uint32_t)entry->pthis_align;
        off = align_up(off, align);
        off += (uint32_t)entry->pthis_size;
    }

    off = align_up(off, 4u);
    if (off > 0xFFFFu) {
        return OSAL_ENOSLOT;
    }

    {
        uint16_t const tmr_tbl = (uint16_t)off;
        off += (uint32_t)param->tmr_max_cnt * (uint32_t)sizeof(struct osal_schedssq_tmr);
        off = align_up(off, 4u);
        if (off > 0xFFFFu) {
            return OSAL_ENOSLOT;
        }

        {
            uint16_t const msgq_start = (uint16_t)off;
            off += (uint32_t)param->msgq_max_len
                   * (uint32_t)sizeof(struct osal_schedssq_msg);
            if ((off > 0xFFFFu) || (off > (uint32_t)param->workspace_size)) {
                return OSAL_ENOSLOT;
            }

            (void)memset(param->workspace, 0, (size_t)off);

            pthis->base.vtbl = &s_ssq_vtbl;
            pthis->base.sched_type = (uint8_t)OSAL_SCHED_TYPE_SSQ;
            pthis->base.sid = sid;
            pthis->asvc_ctx_start = ctx_start;
            pthis->tmr_cnt = param->tmr_max_cnt;
            pthis->tmr_tbl = tmr_tbl;
            pthis->msgq_start = msgq_start;
            pthis->msgq_head = 0u;
            pthis->msgq_tail = 0u;
            pthis->msgq_used = 0u;
            pthis->msgq_cap = param->msgq_max_len;
            pthis->deptbl = param->deptbl;
            pthis->fwdtbl = param->fwdtbl;
        }
    }

    offsets = (uint16_t *)((uint8_t *)pthis + off_tbl);
    off = (uint32_t)ctx_start;
    for (i = 0u; i < entry_cnt; ++i) {
        const osal_asvc_deploy_t * const entry = &param->deptbl->entries[i];
        uint32_t const align = (entry->pthis_align == 0u)
                               ? 4u : (uint32_t)entry->pthis_align;
        off = align_up(off, align);
        offsets[i] = (uint16_t)off;
        off += (uint32_t)entry->pthis_size;
    }

    for (i = 0u; i < entry_cnt; ++i) {
        const osal_asvc_class_t * const cls = param->deptbl->entries[i].base;
        void * const ctx = (uint8_t *)pthis + offsets[i];

        if ((cls->ctor != (bool (*)(void *, const void *, uint16_t))0)
            && !cls->ctor(ctx, param->deptbl->entries[i].ctor_param,
                          param->deptbl->entries[i].ctor_param_size)) {
            while (i > 0u) {
                const osal_asvc_class_t * const prev_cls =
                    param->deptbl->entries[i - 1u].base;
                --i;
                if (prev_cls->dtor != (void (*)(void *))0) {
                    prev_cls->dtor((uint8_t *)pthis + offsets[i]);
                }
            }
            pthis->base.vtbl = (const osal_sched_vtbl_t *)0;
            pthis->base.sched_type = (uint8_t)OSAL_SCHED_TYPE_INVALID;
            osal_schedssq_trace_hook(&pthis->base, (uintptr_t)0,
                                     OSAL_SCHEDSSQ_TRACE_CREATE, OSAL_EPARAM);
            return OSAL_EPARAM;
        }
    }

    *out = &pthis->base;
    osal_schedssq_trace_hook(&pthis->base, (uintptr_t)0,
                             OSAL_SCHEDSSQ_TRACE_CREATE, OSAL_OK);
    return OSAL_OK;
}

osal_err_t osal_schedssq_destroy(osal_sched_t sched)
{
    struct osal_schedssq *pthis;
    uint16_t i;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    if (osal_port_in_isr()) {
        return OSAL_EISR;
    }

    pthis = osal_schedssq_from_base(sched);
    if (pthis->msgq_used != 0u) {
        return OSAL_EBUSY;
    }

    for (i = 0u; i < pthis->tmr_cnt; ++i) {
        osal_schedssq_tmr_at(pthis, i)->used = 0u;
    }

    i = pthis->deptbl->entry_cnt;
    while (i > 0u) {
        const osal_asvc_class_t * const cls = pthis->deptbl->entries[i - 1u].base;

        --i;
        if (cls->dtor != (void (*)(void *))0) {
            cls->dtor(osal_schedssq_pthis(pthis, i));
        }
    }

    osal_schedssq_trace_hook(sched, (uintptr_t)0,
                             OSAL_SCHEDSSQ_TRACE_DESTROY, OSAL_OK);
    pthis->base.vtbl = (const osal_sched_vtbl_t *)0;
    pthis->base.sched_type = (uint8_t)OSAL_SCHED_TYPE_INVALID;
    return OSAL_OK;
}

static osal_err_t sq_post_msg(osal_sched_t sched, uint8_t dest_iid, uintptr_t msg)
{
    osal_err_t ret;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    ret = enqueue_msg(osal_schedssq_from_base(sched), dest_iid, (uint32_t)msg);
    osal_schedssq_trace_hook(sched, msg, OSAL_SCHEDSSQ_TRACE_POST, ret);
    return ret;
}

static osal_err_t sq_post_rtcmsg(osal_sched_t sched, uint8_t dest_iid,
                                 uintptr_t msg)
{
    struct osal_schedssq *pthis;
    const osal_asvc_class_t *cls;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    if (osal_port_in_isr()) {
        return OSAL_EISR;
    }

    pthis = osal_schedssq_from_base(sched);
    if (dest_iid >= pthis->deptbl->entry_cnt) {
        return OSAL_EPARAM;
    }

    cls = pthis->deptbl->entries[dest_iid].base;
    if (cls->rtc_msg_cb == (void (*)(void *, osal_sched_t, uintptr_t))0) {
        return OSAL_EPARAM;
    }

    cls->rtc_msg_cb(osal_schedssq_pthis(pthis, dest_iid), sched, msg);
    osal_schedssq_trace_hook(sched, msg, OSAL_SCHEDSSQ_TRACE_POST_RTC, OSAL_OK);
    return OSAL_OK;
}

static osal_err_t sq_publish_msg(osal_sched_t sched, uint8_t fwd_tag,
                                 uintptr_t msg)
{
    struct osal_schedssq *pthis;
    uint32_t bitmap;
    osal_err_t ret = OSAL_OK;
    uint16_t iid;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }

    pthis = osal_schedssq_from_base(sched);
    if ((pthis->fwdtbl == (const osal_tagfwd_tbl_t *)0)
        || (pthis->fwdtbl->entries == (const uint32_t *)0)
        || (fwd_tag == 0u)
        || (fwd_tag >= pthis->fwdtbl->entry_cnt)
        || (fwd_tag > OSAL_SCHEDSSQ_MAX_FWDTAG)) {
        return OSAL_EPARAM;
    }

    bitmap = pthis->fwdtbl->entries[fwd_tag];
    for (iid = 0u; (iid < pthis->deptbl->entry_cnt) && (iid < 32u); ++iid) {
        if ((bitmap & (1UL << iid)) != 0u) {
            if (enqueue_msg(pthis, (uint8_t)iid, (uint32_t)msg) != OSAL_OK) {
                ret = OSAL_EFULL;
            }
        }
    }

    osal_schedssq_trace_hook(sched, msg, OSAL_SCHEDSSQ_TRACE_PUBLISH, ret);
    return ret;
}

static osal_err_t sq_publish_rtcmsg(osal_sched_t sched, uint8_t fwd_tag,
                                    uintptr_t msg)
{
    struct osal_schedssq *pthis;
    uint32_t bitmap;
    uint16_t iid;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    if (osal_port_in_isr()) {
        return OSAL_EISR;
    }

    pthis = osal_schedssq_from_base(sched);
    if ((pthis->fwdtbl == (const osal_tagfwd_tbl_t *)0)
        || (pthis->fwdtbl->entries == (const uint32_t *)0)
        || (fwd_tag == 0u)
        || (fwd_tag >= pthis->fwdtbl->entry_cnt)
        || (fwd_tag > OSAL_SCHEDSSQ_MAX_FWDTAG)) {
        return OSAL_EPARAM;
    }

    bitmap = pthis->fwdtbl->entries[fwd_tag];
    for (iid = 0u; (iid < pthis->deptbl->entry_cnt) && (iid < 32u); ++iid) {
        if ((bitmap & (1UL << iid)) != 0u) {
            const osal_asvc_class_t * const cls = pthis->deptbl->entries[iid].base;

            if (cls->rtc_msg_cb == (void (*)(void *, osal_sched_t, uintptr_t))0) {
                osal_schedssq_trace_hook(sched, msg,
                                         OSAL_SCHEDSSQ_TRACE_PUBLISH_RTC,
                                         OSAL_EPARAM);
                return OSAL_EPARAM;
            }
            cls->rtc_msg_cb(osal_schedssq_pthis(pthis, iid), sched, msg);
        }
    }

    osal_schedssq_trace_hook(sched, msg, OSAL_SCHEDSSQ_TRACE_PUBLISH_RTC, OSAL_OK);
    return OSAL_OK;
}

static osal_err_t sq_tmr_spawn(osal_sched_t sched,
                               const osal_tmr_param_t *param,
                               uint16_t *tmr_id)
{
    struct osal_schedssq *pthis;
    uint32_t ticks;
    uint16_t i;
    bool found = false;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    if (osal_port_in_isr()) {
        return OSAL_EISR;
    }
    if ((param == (const osal_tmr_param_t *)0) || (tmr_id == (uint16_t *)0)
        || (param->period_ms == 0u)) {
        return OSAL_EPARAM;
    }

    pthis = osal_schedssq_from_base(sched);
    if (param->dest_iid >= pthis->deptbl->entry_cnt) {
        return OSAL_EPARAM;
    }

    ticks = period_to_ticks(param->period_ms);
    if (ticks == 0u) {
        ticks = 1u;
    }

    osal_port_enter_critical();
    for (i = 0u; i < pthis->tmr_cnt; ++i) {
        struct osal_schedssq_tmr * const tmr = osal_schedssq_tmr_at(pthis, i);

        if (tmr->used == 0u) {
            tmr->deadline = osal_port_get_tick() + ticks;
            tmr->period = param->repeat ? ticks : 0u;
            tmr->msg = (uint32_t)param->msg;
            tmr->dest_iid = param->dest_iid;
            tmr->used = 1u;
            *tmr_id = i;
            found = true;
            break;
        }
    }
    osal_port_quit_critical();

    return found ? OSAL_OK : OSAL_ENOSLOT;
}

static osal_err_t sq_tmr_kill(osal_sched_t sched, uint16_t tmr_id)
{
    struct osal_schedssq *pthis;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }
    if (osal_port_in_isr()) {
        return OSAL_EISR;
    }

    pthis = osal_schedssq_from_base(sched);
    if (tmr_id >= pthis->tmr_cnt) {
        return OSAL_EPARAM;
    }

    osal_port_enter_critical();
    if (osal_schedssq_tmr_at(pthis, tmr_id)->used == 0u) {
        osal_port_quit_critical();
        return OSAL_EPARAM;
    }
    osal_schedssq_tmr_at(pthis, tmr_id)->used = 0u;
    osal_port_quit_critical();

    return OSAL_OK;
}

static uint16_t sq_pump(osal_sched_t sched, uint16_t max)
{
    struct osal_schedssq *pthis;
    uint32_t now;
    uint16_t i;
    uint16_t done = 0u;

    if (!sched_is_ssq(sched)) {
        return 0u;
    }

    pthis = osal_schedssq_from_base(sched);
    now = osal_port_get_tick();

    for (i = 0u; i < pthis->tmr_cnt; ++i) {
        bool due = false;
        uint32_t msg = 0u;
        uint8_t dest_iid = 0u;

        osal_port_enter_critical();
        {
            struct osal_schedssq_tmr * const tmr = osal_schedssq_tmr_at(pthis, i);

            if ((tmr->used != 0u)
                && ((int32_t)(now - tmr->deadline) >= 0)) {
                due = true;
                msg = tmr->msg;
                dest_iid = tmr->dest_iid;
                if (tmr->period != 0u) {
                    tmr->deadline += tmr->period;
                    if ((int32_t)(now - tmr->deadline) >= 0) {
                        tmr->deadline = now + tmr->period;
                    }
                } else {
                    tmr->used = 0u;
                }
            }
        }
        osal_port_quit_critical();

        if (due) {
            (void)enqueue_msg(pthis, dest_iid, msg);
        }
    }

    while (done < max) {
        struct osal_schedssq_msg msg;
        const osal_asvc_class_t *cls;

        osal_port_enter_critical();
        if (pthis->msgq_used == 0u) {
            osal_port_quit_critical();
            break;
        }
        msg = *osal_schedssq_msg_at(pthis, pthis->msgq_head);
        pthis->msgq_head = (uint16_t)((pthis->msgq_head + 1u) % pthis->msgq_cap);
        --pthis->msgq_used;
        osal_port_quit_critical();

        if (msg.dest_iid >= pthis->deptbl->entry_cnt) {
            continue;
        }
        cls = pthis->deptbl->entries[msg.dest_iid].base;
        if (cls->msg_cb != (void (*)(void *, osal_sched_t, uintptr_t))0) {
            cls->msg_cb(osal_schedssq_pthis(pthis, msg.dest_iid), sched,
                        (uintptr_t)msg.msg);
        }
        ++done;
    }

    return done;
}

static osal_err_t sq_flush(osal_sched_t sched)
{
    struct osal_schedssq *pthis;

    if (!sched_is_ssq(sched)) {
        return OSAL_EPARAM;
    }

    pthis = osal_schedssq_from_base(sched);
    osal_port_enter_critical();
    pthis->msgq_head = 0u;
    pthis->msgq_tail = 0u;
    pthis->msgq_used = 0u;
    osal_port_quit_critical();

    return OSAL_OK;
}

__attribute__((weak)) void osal_schedssq_trace_hook(
    osal_sched_t pthis, uintptr_t msg, osal_schedssq_trace_evt_t evt,
    osal_err_t ret)
{
    (void)pthis;
    (void)msg;
    (void)evt;
    (void)ret;
}

