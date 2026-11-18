/**
 * @file    app.c
 * @brief   Application entry: one USB test service on the static scheduler.
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
#include <stdbool.h>
#include <stdint.h>

#include "bsp_sys.h"
#include "bspdbg.h"
#include "comlink_ids.h"
#include "osal_port.h"
#include "osal_sched_sq.h"
#include "palusb.h"

/** @brief Private memory of the USB test service. */
typedef struct {
    uint32_t ticks; /**< Reserved for future use. */
} usbtest_ctx_t;

/**
 * @brief Construct the USB test service.
 *
 * @param pthis      Private memory.
 * @param param      Construction parameter, unused.
 * @param param_size Construction parameter size, unused.
 * @return Always true.
 */
static bool usbtest_ctor(void *pthis, const void *param, uint16_t param_size)
{
    usbtest_ctx_t * const ctx = (usbtest_ctx_t *)pthis;

    (void)param;
    (void)param_size;

    ctx->ticks = 0u;
    return true;
}

/**
 * @brief Consume one message of the USB test service.
 *
 * @param pthis Private memory, unused.
 * @param sched Scheduler that dispatched the message, unused.
 * @param msg   Message identifier.
 */
static void usbtest_msg_cb(void *pthis, osal_sched_t sched, uintptr_t msg)
{
    (void)pthis;
    (void)sched;

    switch ((uint32_t)msg) {
    case COMLINK_MSG_USB_TASK:
        palusb_task();
        break;
    case COMLINK_MSG_USB_HELLO:
        palusb_send_string("hello world\r\n");
        break;
    case COMLINK_MSG_LED:
        bspdbg_led_toggle();
        break;
    default:
        break;
    }
}

/** @brief Class declaration of the USB test service. */
static const osal_asvc_class_t s_usbtest_class = {
    .ctor = usbtest_ctor,
    .dtor = (void (*)(void *))0,
    .msg_cb = usbtest_msg_cb,
    .rtc_msg_cb = (void (*)(void *, osal_sched_t, uintptr_t))0,
};

/** @brief Deployment table of the application. */
static const osal_asvc_deploy_t s_deploy[] = {
    {
        .base = &s_usbtest_class,
        .ctor_param = (const void *)0,
        .ctor_param_size = 0u,
        .pthis_size = (uint16_t)sizeof(usbtest_ctx_t),
        .pthis_align = 4u,
    },
};

static const osal_asvc_deploy_tbl_t s_deptbl = {
    .entries = s_deploy,
    .entry_cnt = 1u,
};

static const osal_tagfwd_tbl_t s_fwdtbl = {
    .entries = (const uint32_t *)0,
    .entry_cnt = 0u,
};

/** @brief Workspace of the scheduler, sized by the compile-time bound. */
#define APP_WS_SIZE \
    OSAL_SCHEDSSQ_WORKSPACE_SIZE( \
        1u, \
        OSAL_SCHEDSSQ_PTHIS_BYTES(sizeof(usbtest_ctx_t), 4u), \
        3u, \
        8u)

static uint8_t s_workspace[APP_WS_SIZE] __attribute__((aligned(8)));

static const osal_schedssq_ctorparam_t s_sched_param = {
    .deptbl = &s_deptbl,
    .fwdtbl = &s_fwdtbl,
    .workspace = s_workspace,
    .workspace_size = (uint16_t)sizeof(s_workspace),
    .msgq_max_len = 8u,
    .tmr_max_cnt = 3u,
};

/**
 * @brief Application entry point.
 *
 * @return Never returns.
 */
int main(void)
{
    osal_sched_t sched = (osal_sched_t)0;
    osal_tmr_param_t tmr;
    uint16_t tmr_id;

    bspdbg_led_init();
    palusb_init();

    if (osal_schedssq_create(0u, &s_sched_param, &sched) != OSAL_OK) {
        for (;;) {
        }
    }

    tmr = (osal_tmr_param_t){
        .dest_iid = COMLINK_ASVCIID_USBTEST,
        .msg = (uintptr_t)COMLINK_MSG_USB_TASK,
        .period_ms = 1u,
        .repeat = true,
    };
    if (osal_sched_tmr_spawn(sched, &tmr, &tmr_id) != OSAL_OK) {
        for (;;) {
        }
    }

    tmr = (osal_tmr_param_t){
        .dest_iid = COMLINK_ASVCIID_USBTEST,
        .msg = (uintptr_t)COMLINK_MSG_USB_HELLO,
        .period_ms = 1000u,
        .repeat = true,
    };
    if (osal_sched_tmr_spawn(sched, &tmr, &tmr_id) != OSAL_OK) {
        for (;;) {
        }
    }

    tmr = (osal_tmr_param_t){
        .dest_iid = COMLINK_ASVCIID_USBTEST,
        .msg = (uintptr_t)COMLINK_MSG_LED,
        .period_ms = 500u,
        .repeat = true,
    };
    if (osal_sched_tmr_spawn(sched, &tmr, &tmr_id) != OSAL_OK) {
        for (;;) {
        }
    }

    bsp_tick_start(OSAL_PORT_TICK_HZ, osal_port_tick);
    osal_sched_run(sched);
}

