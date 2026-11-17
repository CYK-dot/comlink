/**
 * @file    bspdbg.c
 * @brief   Board-level debug helpers for the CH572 evaluation board
 */
#include "bspdbg.h"
#include "CH57x_common.h"

/** Debug LED pin on this board */
#define BSPDBG_LED_PIN GPIO_Pin_7

void bspdbg_led_init(void)
{
    GPIOA_ModeCfg(BSPDBG_LED_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(BSPDBG_LED_PIN);
}

void bspdbg_led_toggle(void)
{
    GPIOA_InverseBits(BSPDBG_LED_PIN);
}

void bspdbg_poll_delayms(uint32_t ms)
{
    /* mDelaymS takes uint16_t; split long delays into chunks */
    while(ms > 0u)
    {
        uint16_t chunk = (ms > 60000u) ? 60000u : (uint16_t)ms;
        mDelaymS(chunk);
        ms -= chunk;
    }
}
