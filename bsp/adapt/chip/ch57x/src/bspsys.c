/**
 * @file    bspsys.c
 * @brief   System bring-up for CH57x
 *
 * The application tick uses the general-purpose TMR, not SysTick: on the
 * BLE-capable CH57x parts SysTick belongs to the Bluetooth stack's time base
 * and must be left alone.
 */
#include "bsp_sys.h"
#include "bsp_cpu.h"
#include "CH57x_common.h"
#include "CH57x_timer.h"

static void (*s_tick_cb)(void);
static uint32_t s_tick_counts;

void bsp_sys_init(void)
{
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);

    /* Keep the USB device detached while the bootloader waits for the host.
     * RB_PIN_DEBUG_EN shares the USB pins and must be cleared for USB to work. */
    R8_USB_CTRL = 0x00;
    R16_PIN_ALTERNATE &= ~(RB_PIN_USB_EN | RB_UDP_PU_EN | RB_PIN_DEBUG_EN);
}

void bsp_tick_start(uint32_t tick_hz, void (*cb)(void))
{
    s_tick_cb = cb;
    s_tick_counts = (uint32_t)FREQ_SYS / tick_hz;

    TMR_TimerInit(s_tick_counts);
    TMR_ITCfg(ENABLE, TMR_IT_CYC_END);
    PFIC_EnableIRQ(TMR_IRQn);
}

__INTERRUPT void TMR_IRQHandler(void)
{
    TMR_ClearITFlag(TMR_IT_CYC_END);
    TMR_TimerInit(s_tick_counts);       /* reload the period */

    bsp_cpu_isr_enter();
    if (s_tick_cb != (void (*)(void))0) {
        s_tick_cb();
    }
    bsp_cpu_isr_exit();
}
