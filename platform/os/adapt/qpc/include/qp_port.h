/**
 * @file    qp_port.h
 * @brief   QP/C port for the cooperative QV kernel on this platform
 *
 * Derived from QP/C's ports/risc-v/qv/gnu port. QP/C knows nothing about
 * QingKe: the core-specific parts (interrupt mask, critical-section policy,
 * log2) are BSP CPU primitives and this header only names them.
 */
#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "qp_config.h"
#include "bsp_cpu.h"

#ifdef __cplusplus
#define Q_NORETURN              [[ noreturn ]] void
#define Q_ASSERT_STATIC(expr_)  static_assert((expr_), "QP static assert")
#else
#define Q_NORETURN              _Noreturn void
#define Q_ASSERT_STATIC(expr_)  _Static_assert((expr_), "QP static assert")
#endif

/* QActive event queue type: QV uses the native QP event queue. The QP/C
   risc-v port header omits this, so it is restored here. */
#define QACTIVE_EQUEUE_TYPE QEQueue

/* Interrupt masking and critical sections delegate to the BSP's CPU
   primitives: this port header stays free of core asm. */
#define QF_INT_DISABLE()    bsp_irq_disable()
#define QF_INT_ENABLE()     bsp_irq_enable()

#define QF_CRIT_STAT        bsp_irq_state_t irq_state_;
#define QF_CRIT_ENTRY()     do { irq_state_ = bsp_irq_save(); } while (false)
#define QF_CRIT_EXIT()      bsp_irq_restore(irq_state_)

/* Log2 for the QV ready-set scheduler; RV32 has no cheap instruction. */
#define QF_LOG2(n_)         ((uint_fast8_t)(32U - __builtin_clz((n_))))

/* Keep back-to-back critical sections from being merged into one. */
#define QF_CRIT_EXIT_NOP()  bsp_cpu_nop()

#include "qequeue.h"
#include "qmpool.h"
#include "qp.h"
#include "qv.h"

#endif /* QP_PORT_H_ */
