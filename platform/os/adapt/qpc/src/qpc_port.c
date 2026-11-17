/**
 * @file    qpc_port.c
 * @brief   Callbacks QP/C expects from the platform (QV kernel, bare metal)
 *
 * No core asm here: interrupt masking, idle and halt come from the BSP's CPU
 * primitives (bsp/inc/bsp_cpu.h), so this file is board glue only.
 */
#include "qpc.h"
#include "bsp_cpu.h"

void QF_onStartup(void)
{
    /* Nothing to do on this port: QF_run() enters its loop with interrupts
       masked and QV enables them around each dispatch, and QV_onIdle() below
       masks them again after sleeping. Interrupt sources are armed by the BSP. */
}

void QF_onCleanup(void)
{
    bsp_irq_disable();
}

void QV_onIdle(void)
{
    bsp_cpu_idle();
}

Q_NORETURN Q_onError(char const * const module, int_t const id)
{
    (void)module;
    (void)id;

    bsp_cpu_halt();
    for (;;) {
    }
}
