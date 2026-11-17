/**
 * @file    qp_config.h
 * @brief   QP/C configuration for this product
 *
 * Compiled into QP/C and the application alike; qp_cfgchk.h turns a mismatch
 * between the two into a link error.
 */
#ifndef QP_CONFIG_H_
#define QP_CONFIG_H_

/* Active objects. The design caps instances at 32 (bitmap width). */
#define QF_MAX_ACTIVE       32U

/* One tick rate: the platform tick is the only time base. */
#define QF_MAX_TICK_RATE     1U

/* Event pools are available, but allocated by the application (no heap). */
#define QF_MAX_EPOOL         3U

/* 32-bit time-event counters: periods beyond the 65 s of 16 bits. */
#define QF_TIMEEVT_CTR_SIZE  4U

#endif /* QP_CONFIG_H_ */
