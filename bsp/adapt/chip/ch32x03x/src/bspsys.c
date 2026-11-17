/**
 * @file    bspsys.c
 * @brief   System bring-up for CH32X03x
 *
 * SystemInit() already runs from the startup (clock, flash latency); the only
 * thing left here is the application tick, which uses TIM2.
 */
#include "bsp_sys.h"
#include "bsp_cpu.h"
#include "ch32x035.h"

static void (*s_tick_cb)(void);

void bsp_sys_init(void)
{
}

void bsp_tick_start(uint32_t tick_hz, void (*cb)(void))
{
    TIM_TimeBaseInitTypeDef tb;

    s_tick_cb = cb;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* 1 MHz counter clock, then one update per tick period */
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler = (uint16_t)((SystemCoreClock / 1000000u) - 1u);
    tb.TIM_Period = (uint16_t)((1000000u / tick_hz) - 1u);
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM2_UP_IRQn);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void TIM2_UP_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    bsp_cpu_isr_enter();
    if (s_tick_cb != (void (*)(void))0) {
        s_tick_cb();
    }
    bsp_cpu_isr_exit();
}
