/**
 * @file    bsp_cpu.c
 * @brief   Interrupt-nesting state for the CPU primitives
 *
 * Kept out of the header because the flag must be one object shared by every
 * translation unit. It is incremented by each interrupt handler that calls
 * into the OS layer; bsp_irq_save() then knows it must not touch mstatus from
 * interrupt context.
 */
#include "bsp_cpu.h"

volatile uint32_t bsp_cpu_isr_nesting_;

void bsp_cpu_isr_enter(void)
{
    ++bsp_cpu_isr_nesting_;
}

void bsp_cpu_isr_exit(void)
{
    --bsp_cpu_isr_nesting_;
}
