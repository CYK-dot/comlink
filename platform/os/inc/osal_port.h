/**
 * @file    osal_port.h
 * @brief   Porting contract between the OSAL core and the CPU/BSP.
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
#ifndef OSAL_PORT_H
#define OSAL_PORT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Tick rate of the port time base, in Hz.
 *
 * The port must advance the tick at exactly this rate and implement
 * osal_port_get_tick() against it.
 */
#define OSAL_PORT_TICK_HZ 1000u

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enter a critical section.
 *
 * The critical section guards the scheduler queues and tables. Calls nest:
 * the port keeps a nesting counter, and only the outermost exit re-enables
 * interrupts. Calling it from interrupt context is legal and is treated as
 * "interrupts already masked".
 */
void osal_port_enter_critical(void);

/**
 * @brief Leave a critical section entered by osal_port_enter_critical().
 *
 * Must be paired with osal_port_enter_critical() and is safe from interrupt
 * context; it re-enables interrupts only when the nesting counter reaches zero.
 */
void osal_port_quit_critical(void);

/**
 * @brief Report whether the caller runs in interrupt context.
 *
 * @return true when called from an interrupt handler.
 */
bool osal_port_in_isr(void);

/**
 * @brief Advance the OSAL tick.
 *
 * Called by the BSP tick interrupt at OSAL_PORT_TICK_HZ; this is the only
 * function the BSP has to know about.
 */
void osal_port_tick(void);

/**
 * @brief Read the current tick.
 *
 * The read must be atomic with respect to the tick interrupt; a single aligned
 * word read is sufficient on the supported cores.
 *
 * @return Tick count since the port time base started.
 */
uint32_t osal_port_get_tick(void);

/**
 * @brief Enter the low-power idle state.
 *
 * Called only when the scheduler has no ready message and no expired timer;
 * the implementation may execute a wait-for-interrupt instruction.
 */
void osal_port_idle(void);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_PORT_H */
