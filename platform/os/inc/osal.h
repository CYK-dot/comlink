/**
 * @file    osal.h
 * @brief   OS abstraction layer: the runtime API the application programs to
 *
 * Active objects, signals, timers and the tick are OSAL concepts. Which
 * runtime implements them (QP/C today) is not visible here: no header in
 * platform/os/inc includes a runtime header, and the application never calls
 * a runtime function directly.
 */
#ifndef OSAL_H
#define OSAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief  Tick rate the platform runs at, in Hz
 *
 * osal_tick() must be called from an interrupt at exactly this rate; timer
 * periods are converted to ticks against it.
 */
#define OSAL_TICK_HZ 1000u

/**
 * @brief  Static capacity of the OSAL pools (no heap anywhere)
 */
#define OSAL_MAX_AO     8u
/** @brief Events a single active object can hold */
#define OSAL_AO_QUEUE   8u
/** @brief Timers that can exist at the same time */
#define OSAL_MAX_TIMER  8u

/** @brief Active-object handle (opaque to the application) */
typedef struct osal_ao osal_ao_t;
/** @brief Timer handle (opaque to the application) */
typedef struct osal_timer osal_timer_t;

/** @brief Event signal, the only field an OSAL event carries for now */
typedef uint16_t osal_sig_t;

/**
 * @brief  Event delivered to an active object
 *
 * Payload transport is the next increment; the structure grows, the handler
 * signature does not change.
 */
typedef struct {
    osal_sig_t sig;
} osal_evt_t;

/** @brief Active-object event handler, runs in the AO's turn */
typedef void (*osal_ao_handler_t)(osal_ao_t *self, osal_evt_t const *evt);

/** @brief Status codes */
typedef enum {
    OSAL_OK = 0,     /**< success */
    OSAL_EPARAM,     /**< invalid handle or argument */
    OSAL_ENOSLOT,    /**< a static pool is exhausted */
    OSAL_EFULL,      /**< the target queue is full */
} osal_err_t;

/**
 * @brief  Bring up the runtime
 *
 * Must be called once, before any other OSAL function.
 */
void osal_init(void);

/**
 * @brief  Run the scheduler
 *
 * Never returns: the caller hands the main loop over to the runtime.
 */
void osal_run(void);

/**
 * @brief  Advance the OSAL time base by one tick
 *
 * Called from the platform tick interrupt at OSAL_TICK_HZ. Only this function
 * runs in interrupt context on the OSAL side.
 */
void osal_tick(void);

/**
 * @brief  Create an active object from the static pool
 *
 * @param  handler  event handler, required
 * @param  prio     priority, 1..OSAL_MAX_AO, unique among live AOs
 *
 * @return handle, or NULL when the pool is exhausted or the arguments are bad
 */
osal_ao_t *osal_ao_create(osal_ao_handler_t handler, uint8_t prio);

/**
 * @brief  Start a created active object
 *
 * The object becomes eligible for events and runs its initial transition
 * synchronously.
 */
osal_err_t osal_ao_start(osal_ao_t *ao);

/**
 * @brief  Post a signal to an active object (task context)
 *
 * The event is copied from the pool, so the caller owns nothing afterwards.
 * A full queue returns OSAL_EFULL and the event is recycled.
 */
osal_err_t osal_post(osal_ao_t *ao, osal_sig_t sig);

/**
 * @brief  Post a signal from an interrupt handler
 *
 * Same semantics as osal_post(), safe to call with interrupts disabled.
 */
osal_err_t osal_post_isr(osal_ao_t *ao, osal_sig_t sig);

/**
 * @brief  Create a timer that posts the given signal to an active object
 *
 * Nothing expires until osal_timer_start().
 */
osal_timer_t *osal_timer_create(osal_ao_t *ao, osal_sig_t sig);

/**
 * @brief  Start (or restart) a timer
 *
 * @param  period_ms  period in milliseconds, at least one tick
 * @param  periodic   true = repeat every period, false = one shot
 *
 * The owning active object must be started first.
 */
osal_err_t osal_timer_start(osal_timer_t *timer, uint32_t period_ms, bool periodic);

/**
 * @brief  Stop a timer that has not expired yet
 *
 * Stopping an already stopped timer is harmless.
 */
osal_err_t osal_timer_stop(osal_timer_t *timer);

#endif /* OSAL_H */
