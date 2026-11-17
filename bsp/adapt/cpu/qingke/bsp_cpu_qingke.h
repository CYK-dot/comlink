/**
 * @file    bsp_cpu_qingke.h
 * @brief   Inline CPU primitives for the WCH QingKe (RV32) core
 *
 * Included by bsp/inc/bsp_cpu.h when bsp.ARCH_QINGKE is set; not meant to be
 * included directly.
 *
 * Interrupts are gated through the architectural mstatus.MIE: the WCH startup
 * enables them exactly there (mstatus = 0x1888). The catch is that a critical
 * section may also run inside an interrupt handler (QP's tick handler does),
 * and some QingKe generations do not tolerate mstatus writes from interrupt
 * context. The saved state therefore never touches a CSR when the handler
 * nesting counter says we are in an ISR: it simply reports "masked", so the
 * matching restore cannot unmask anything.
 *
 * MPIE is never touched, so an ISR still returns correctly through mret.
 */
#ifndef BSP_CPU_QINGKE_H
#define BSP_CPU_QINGKE_H

/**
 * @brief  Mask interrupts and report the previous state
 *
 * The return value is only meaningful to bsp_irq_restore(), and is zero when
 * the caller runs inside an interrupt handler.
 */
static inline bsp_irq_state_t bsp_irq_save(void)
{
    bsp_irq_state_t state = 0u;

    if (bsp_cpu_isr_nesting_ == 0u) {
        __asm__ volatile("csrr %0, mstatus" : "=r"(state));
        __asm__ volatile("csrc mstatus, 8" ::: "memory");
        state &= 8u;
    }
    return state;
}

/** @brief Restore the state returned by bsp_irq_save() */
static inline void bsp_irq_restore(bsp_irq_state_t state)
{
    if (state != 0u) {
        __asm__ volatile("csrs mstatus, 8" ::: "memory");
    }
}

/** @brief Mask interrupts */
static inline void bsp_irq_disable(void)
{
    __asm__ volatile("csrc mstatus, 8" ::: "memory");
}

/** @brief Unmask interrupts */
static inline void bsp_irq_enable(void)
{
    __asm__ volatile("csrs mstatus, 8" ::: "memory");
}

/**
 * @brief  Sleep until the next interrupt
 *
 * Entered with interrupts masked, as the scheduler's idle path requires:
 * unmask, wait, mask again. The caller must have at least one interrupt
 * source enabled, or this sleeps forever.
 */
static inline void bsp_cpu_idle(void)
{
    __asm__ volatile("csrs mstatus, 8\n\twfi\n\tcsrc mstatus, 8" ::: "memory");
}

/**
 * @brief  Mask interrupts and stop: the last act of a fatal error handler
 *
 * Never returns.
 */
static inline void bsp_cpu_halt(void)
{
    bsp_irq_disable();
    for (;;) {
        __asm__ volatile("wfi");
    }
}

/** @brief Keep back-to-back critical sections from being merged */
static inline void bsp_cpu_nop(void)
{
    __asm__ volatile("nop");
}

#endif /* BSP_CPU_QINGKE_H */
