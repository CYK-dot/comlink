/**
 * @file    osal_port.c
 * @brief   OSAL port layer built on the BSP CPU primitives.
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
#include "osal_port.h"

#include "bsp_cpu.h"

/** @brief Tick counter advanced by the BSP tick interrupt. */
static volatile uint32_t s_tick;

/** @brief Nesting depth of osal_port_enter_critical(). */
static uint32_t s_crit_nest;

/** @brief Interrupt state saved by the outermost critical section. */
static bsp_irq_state_t s_crit_state;

void osal_port_enter_critical(void)
{
    if (s_crit_nest == 0u) {
        s_crit_state = bsp_irq_save();
    }
    ++s_crit_nest;
}

void osal_port_quit_critical(void)
{
    if (s_crit_nest != 0u) {
        --s_crit_nest;
        if (s_crit_nest == 0u) {
            bsp_irq_restore(s_crit_state);
        }
    }
}

bool osal_port_in_isr(void)
{
    return bsp_cpu_isr_nesting_ != 0u;
}

void osal_port_tick(void)
{
    ++s_tick;
}

uint32_t osal_port_get_tick(void)
{
    return s_tick;
}

void osal_port_idle(void)
{
    bsp_cpu_idle();
}
