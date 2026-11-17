/**
 * @file    bspdbg.h
 * @brief   Board-level debug helpers: a status LED and a polling delay
 *
 * Implemented per board (bsp/adapt/board/<board>/src). They are intentionally
 * simple and blocking: use them in bring-up and bootloader paths, not in places
 * that need an event driven delay.
 */
#ifndef BSPDBG_H
#define BSPDBG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure the debug LED pin as a push-pull output, driven low
 */
void bspdbg_led_init(void);

/**
 * @brief  Toggle the debug LED
 */
void bspdbg_led_toggle(void);

/**
 * @brief  Block for the given number of milliseconds
 *
 * @param  ms  delay in milliseconds
 *
 * @note   Busy wait; accuracy depends on the configured system clock matching
 *         the calibration constant of the delay loop.
 */
void bspdbg_poll_delayms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* BSPDBG_H */
