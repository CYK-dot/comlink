/**
 * @file    bsp_sys.h
 * @brief   System bring-up shared by the bootloader and the application
 */
#ifndef BSP_SYS_H
#define BSP_SYS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise the system clock and the resources the BSP relies on
 *
 * Called once by the bootloader. An application entered from it inherits the
 * configuration and must not reconfigure the clock.
 */
void bsp_sys_init(void);

/**
 * @brief  Start the periodic tick interrupt
 *
 * @param  tick_hz  interrupt rate in Hz (the application's time base)
 * @param  cb       called from the tick interrupt, may be NULL
 *
 * The application calls this once, after its runtime has been initialised and
 * before it enters the scheduler loop. The tick source is chip-specific; the
 * callback runs in interrupt context.
 */
void bsp_tick_start(uint32_t tick_hz, void (*cb)(void));

#ifdef __cplusplus
}
#endif

#endif /* BSP_SYS_H */
