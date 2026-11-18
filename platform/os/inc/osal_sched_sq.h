/**
 * @file    osal_sched_sq.h
 * @brief   Static single-queue scheduler.
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
#ifndef OSAL_SCHED_SQ_H
#define OSAL_SCHED_SQ_H

#include <stdbool.h>
#include <stdint.h>

#include "osal_sched_base.h"
#include "osal_asvc.h"

/** @brief Maximum number of services a single-queue scheduler can host. */
#define OSAL_SCHEDSSQ_MAX_ASVC_CNT 31u

/** @brief Highest forwarding tag value; tag 0 is reserved and invalid. */
#define OSAL_SCHEDSSQ_MAX_FWDTAG 254u

/** @brief Number of forwarding bitmap entries, indexed by tag. */
#define OSAL_SCHEDSSQ_FWDTAG_ENTRY_CNT 255u

/**
 * @brief Implementation-defined upper bound of the scheduler control block.
 *
 * The exact layout is private to the OSAL implementation; the build asserts
 * that the real control block never exceeds this bound. Applications use it
 * only through OSAL_SCHEDSSQ_WORKSPACE_SIZE().
 */
#define OSAL_SCHEDSSQ_CTRL_MAX (48u + 3u * (sizeof(void *) - 4u))

/**
 * @brief Implementation-defined upper bound of one timer record.
 *
 * Applications use it only through OSAL_SCHEDSSQ_WORKSPACE_SIZE().
 */
#define OSAL_SCHEDSSQ_TMR_MAX 16u

/** @brief Size of one queue slot: four-byte payload plus destination. */
#define OSAL_SCHEDSSQ_QUEUE_ELEM_SIZE 8u

/** @brief Alignment guaranteed for each workspace section. */
#define OSAL_SCHEDSSQ_ALIGN 8u

/**
 * @brief Round a size up to an alignment boundary.
 *
 * @param size  Size in bytes.
 * @param align Alignment in bytes, must be a power of two.
 */
#define OSAL_SCHEDSSQ_ALIGN_UP(size, align) \
    (((size) + ((align) - 1u)) & ~((align) - 1u))

/**
 * @brief Aligned private-memory footprint of one service instance.
 *
 * @param size  Value of osal_asvc_deploy_t::pthis_size.
 * @param align Value of osal_asvc_deploy_t::pthis_align; 0 means 4.
 */
#define OSAL_SCHEDSSQ_PTHIS_BYTES(size, align) \
    OSAL_SCHEDSSQ_ALIGN_UP((size), ((align) == 0u ? 4u : (align)))

/**
 * @brief Compile-time upper bound of the workspace needed by a scheduler.
 *
 * The result is a constant expression and can size a static array or a global
 * buffer. It is a guaranteed upper bound, not the exact size: the scheduler
 * computes the exact layout in create and rejects a buffer that is too small
 * with OSAL_ENOSLOT.
 *
 * @param asvc_cnt    Number of deployed services, 1..OSAL_SCHEDSSQ_MAX_ASVC_CNT.
 * @param pthis_bytes Sum of OSAL_SCHEDSSQ_PTHIS_BYTES() over all services.
 * @param tmr_cnt     Timer capacity, osal_schedssq_ctorparam_t::tmr_max_cnt.
 * @param msgq_len    Queue capacity, osal_schedssq_ctorparam_t::msgq_max_len.
 *
 * @note pthis_align values must not exceed OSAL_SCHEDSSQ_ALIGN.
 */
#define OSAL_SCHEDSSQ_WORKSPACE_SIZE(asvc_cnt, pthis_bytes, tmr_cnt, msgq_len) \
    (OSAL_SCHEDSSQ_ALIGN_UP(OSAL_SCHEDSSQ_CTRL_MAX, OSAL_SCHEDSSQ_ALIGN) \
     + OSAL_SCHEDSSQ_ALIGN_UP((asvc_cnt) * 2u, OSAL_SCHEDSSQ_ALIGN) \
     + OSAL_SCHEDSSQ_ALIGN_UP((pthis_bytes), OSAL_SCHEDSSQ_ALIGN) \
     + OSAL_SCHEDSSQ_ALIGN_UP((tmr_cnt) * OSAL_SCHEDSSQ_TMR_MAX, OSAL_SCHEDSSQ_ALIGN) \
     + OSAL_SCHEDSSQ_ALIGN_UP((msgq_len) * OSAL_SCHEDSSQ_QUEUE_ELEM_SIZE, OSAL_SCHEDSSQ_ALIGN))

OSAL_STATIC_ASSERT(OSAL_SCHEDSSQ_MAX_ASVC_CNT <= 32u,
                   "service count must fit the 32-bit forwarding bitmap");
OSAL_STATIC_ASSERT(OSAL_SCHEDSSQ_MAX_FWDTAG < OSAL_SCHEDSSQ_FWDTAG_ENTRY_CNT,
                   "tag range must fit the forwarding table");

/**
 * @brief Deployment parameters of one service instance.
 */
typedef struct {
    const osal_asvc_class_t *base; /**< Class declaration, must not be NULL. */
    const void *ctor_param;        /**< Construction parameter, may be NULL. */
    uint16_t ctor_param_size;      /**< Construction parameter size in bytes. */
    uint16_t pthis_size;           /**< Private memory size in bytes. */
    uint8_t pthis_align;           /**< Private memory alignment; 0 means 4 bytes. */
} osal_asvc_deploy_t;

/**
 * @brief Deployment table indexed by service instance identifier.
 */
typedef struct {
    const osal_asvc_deploy_t *entries; /**< Entry iid describes service iid. */
    uint16_t entry_cnt;                /**< Number of valid entries. */
} osal_asvc_deploy_tbl_t;

/**
 * @brief Forwarding table indexed by tag.
 *
 * Bit iid of entry tag selects the service instance iid as a multicast
 * recipient. The table is read-only and is normally placed in flash.
 */
typedef struct {
    const uint32_t *entries; /**< Bitmap per tag; index 0 is unused. */
    uint16_t entry_cnt;      /**< Number of valid entries, at most 255. */
} osal_tagfwd_tbl_t;

/**
 * @brief Construction parameters of a static single-queue scheduler.
 */
typedef struct {
    const osal_asvc_deploy_tbl_t *deptbl; /**< Service deployment table. */
    const osal_tagfwd_tbl_t *fwdtbl;      /**< Tag forwarding table. */
    void *workspace;                      /**< Caller-provided RAM region. */
    uint16_t workspace_size;              /**< Size of the workspace in bytes. */
    uint16_t msgq_max_len;                /**< Message queue capacity in messages. */
    uint16_t tmr_max_cnt;                 /**< Timer table capacity in timers. */
} osal_schedssq_ctorparam_t;

/**
 * @brief Trace events reported to osal_schedssq_trace_hook().
 */
typedef enum {
    OSAL_SCHEDSSQ_TRACE_CREATE = 0, /**< Scheduler created. */
    OSAL_SCHEDSSQ_TRACE_DESTROY,    /**< Scheduler destroyed. */
    OSAL_SCHEDSSQ_TRACE_POST,       /**< Queued message posted. */
    OSAL_SCHEDSSQ_TRACE_POST_RTC,   /**< Run-to-completion message posted. */
    OSAL_SCHEDSSQ_TRACE_PUBLISH,    /**< Queued message published. */
    OSAL_SCHEDSSQ_TRACE_PUBLISH_RTC,/**< Run-to-completion message published. */
} osal_schedssq_trace_evt_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a static single-queue scheduler in caller-provided memory.
 *
 * The workspace must be at least four-byte aligned. Its size must be at least
 * the value of OSAL_SCHEDSSQ_WORKSPACE_SIZE() for the same capacities; create
 * computes the exact layout and returns OSAL_ENOSLOT when the buffer is too
 * small. Every service is constructed from its deployment entry; if a
 * constructor rejects, the services constructed so far are destroyed in
 * reverse order and the call fails.
 *
 * @param sid   Caller-assigned scheduler identifier.
 * @param param Construction parameters, must not be NULL.
 * @param out   Receives the scheduler handle on success.
 * @return OSAL_OK, OSAL_EPARAM or OSAL_ENOSLOT.
 */
osal_err_t osal_schedssq_create(uint8_t sid,
                                const osal_schedssq_ctorparam_t *param,
                                osal_sched_t *out);

/**
 * @brief Destroy a scheduler and release its workspace.
 *
 * All timers are killed and all services are destroyed in reverse construction
 * order. The call fails with OSAL_EBUSY while messages are still queued; use
 * osal_sched_flush() first when pending messages may be discarded. After a
 * successful call the workspace can be reused by another scheduler.
 *
 * @param pthis Scheduler handle.
 * @return OSAL_OK, OSAL_EPARAM, OSAL_EBUSY or OSAL_EISR.
 */
osal_err_t osal_schedssq_destroy(osal_sched_t pthis);

/**
 * @brief Trace hook called for scheduler events.
 *
 * The core provides a weak empty definition; an application may override it.
 * For post and publish events the hook runs in the caller's context, including
 * interrupt context, so it must not call back into the OSAL posting API.
 *
 * @param pthis Scheduler handle.
 * @param msg   Message involved in the event, or zero for create/destroy.
 * @param evt   Event kind.
 * @param ret   Result of the operation being traced.
 */
void osal_schedssq_trace_hook(osal_sched_t pthis, uintptr_t msg,
                              osal_schedssq_trace_evt_t evt, osal_err_t ret);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_SCHED_SQ_H */



