/**
 * @file    bsp_cpu.h
 * @brief   CPU primitives: interrupt masking, idle and halt
 *
 * The only interface through which the OS layer touches the core. The
 * implementation is header-only and selected by the BSP's bsp.ARCH_* feature:
 * a critical section runs on every event post and every queue operation, so
 * the primitives must inline, and they live in the backend header
 * bsp/adapt/cpu/<arch>/.
 */
#ifndef BSP_CPU_H
#define BSP_CPU_H

#include <stdint.h>

/** @brief Saved interrupt state: non-zero when interrupts were enabled */
typedef uint32_t bsp_irq_state_t;

/** @brief Interrupt-handler nesting depth (0 = thread context) */
extern volatile uint32_t bsp_cpu_isr_nesting_;

/**
 * @brief  Announce entry into an interrupt handler
 *
 * Every ISR that may call into the OS layer (or bsp_irq_save()) must bracket
 * its callback with bsp_cpu_isr_enter()/bsp_cpu_isr_exit(): the CPU layer then
 * keeps CRITICAL sections inside interrupt context away from mstatus, because
 * some QingKe generations do not tolerate CSR writes there.
 */
void bsp_cpu_isr_enter(void);

/** @brief Announce exit from an interrupt handler */
void bsp_cpu_isr_exit(void);

#include "strata_features.h"

#if STRATA_FEATURE_BSP_ARCH_QINGKE
#include "bsp_cpu_qingke.h"
#else
#error "bsp_cpu.h: no CPU backend selected (declare a bsp.ARCH_* feature)"
#endif

#endif /* BSP_CPU_H */
