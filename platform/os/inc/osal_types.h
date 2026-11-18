/**
 * @file    osal_types.h
 * @brief   Common types and result codes of the OSAL.
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
#ifndef OSAL_TYPES_H
#define OSAL_TYPES_H

#include <stdint.h>

/**
 * @brief Result code returned by OSAL functions.
 */
typedef enum {
    OSAL_OK = 0,   /**< Operation completed successfully. */
    OSAL_EPARAM,   /**< Invalid handle, identifier, tag or argument. */
    OSAL_EFULL,    /**< Target queue or table is full. */
    OSAL_ENOSLOT,  /**< Static pool or workspace has no free slot. */
    OSAL_EDEAD,    /**< Target service is no longer alive. */
    OSAL_EISR,     /**< Operation is not allowed in interrupt context. */
    OSAL_EBUSY,    /**< Object still has pending work. */
} osal_err_t;

/**
 * @brief Compile-time assertion.
 *
 * @param cond Expression that must be true at compile time.
 * @param msg  Diagnostic message for C11 compilers.
 */
#if defined(__cplusplus)
#define OSAL_STATIC_ASSERT(cond, msg) static_assert((cond), msg)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define OSAL_STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)
#else
#define OSAL_CAT_(a, b) a##b
#define OSAL_CAT(a, b) OSAL_CAT_(a, b)
#define OSAL_STATIC_ASSERT(cond, msg) \
    typedef char OSAL_CAT(osal_static_assert_, __LINE__)[(cond) ? 1 : -1]
#endif

#endif /* OSAL_TYPES_H */
