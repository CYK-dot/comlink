/**
 * @file    bspdbg.c
 * @brief   Board-level debug helpers for the CH32X035 evaluation board
 */
#include "bspdbg.h"
#include "ch32x035.h"
#include "debug.h"

/** Debug LED on this board: PC15 */
#define BSPDBG_LED_PORT GPIOC
#define BSPDBG_LED_PIN  GPIO_Pin_15

static uint8_t s_led_level;
static uint8_t s_delay_ready;

void bspdbg_led_init(void)
{
    GPIO_InitTypeDef cfg;

    GPIO_StructInit(&cfg);
    cfg.GPIO_Pin   = BSPDBG_LED_PIN;
    cfg.GPIO_Speed = GPIO_Speed_50MHz;
    cfg.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(BSPDBG_LED_PORT, &cfg);
    GPIO_ResetBits(BSPDBG_LED_PORT, BSPDBG_LED_PIN);
    s_led_level = 0;
}

void bspdbg_led_toggle(void)
{
    s_led_level = (uint8_t)(s_led_level == 0u);
    if(s_led_level)
    {
        GPIO_SetBits(BSPDBG_LED_PORT, BSPDBG_LED_PIN);
    }
    else
    {
        GPIO_ResetBits(BSPDBG_LED_PORT, BSPDBG_LED_PIN);
    }
}

void bspdbg_poll_delayms(uint32_t ms)
{
    /* Delay_Ms relies on module state set by Delay_Init(); a caller that only
       entered through the bootloader never initialised it. Delay_Init() is
       idempotent, so doing it lazily keeps this helper self-contained. */
    if(!s_delay_ready)
    {
        Delay_Init();
        s_delay_ready = 1;
    }
    Delay_Ms(ms);
}
