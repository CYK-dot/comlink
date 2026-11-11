/**
 * @file    bootloader.c
 * @brief   bootloader demo：PA7 快闪 10 秒后跳转到 app（0x4000）
 */
#include "CH57x_common.h"

#define LED_PIN          GPIO_Pin_7
#define APP_BASE         0x00004000u   /* 与 main/app.ld 中 FLASH ORIGIN 一致 */
#define BOOT_WAIT_MS     10000u
#define BOOT_BLINK_MS    50u
#define JAL_OPCODE       0x6Fu         /* app 首条指令应为 j handle_reset */

/* app 镜像是否已烧录：0x4000 处的首条指令是 JAL，擦除态 0xFFFFFFFF 不是 */
static bool app_present(void)
{
    return ((*(volatile uint32_t *)APP_BASE) & 0x7Fu) == JAL_OPCODE;
}

/* 无有效镜像：3 短闪 + 1 秒停顿，无限循环（后续接 USB MSC / RF 下载） */
static void wait_for_upgrade(void)
{
    for(;;)
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            GPIOA_InverseBits(LED_PIN);
            mDelaymS(80);
            GPIOA_InverseBits(LED_PIN);
            mDelaymS(80);
        }
        mDelaymS(1000);
    }
}

int main(void)
{
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);
    GPIOA_ModeCfg(LED_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(LED_PIN);

    for(uint32_t t = 0; t < BOOT_WAIT_MS; t += 2 * BOOT_BLINK_MS)
    {
        GPIOA_InverseBits(LED_PIN);
        mDelaymS(BOOT_BLINK_MS);
        GPIOA_InverseBits(LED_PIN);
        mDelaymS(BOOT_BLINK_MS);
    }

    if(!app_present())
    {
        wait_for_upgrade();
    }

    PFIC_DisableAllIRQ();              /* app 的 _start 会重装向量表并重新开中断 */
    ((void (*)(void))APP_BASE)();      /* 跳 app 的 _start，不返回 */
    for(;;)
    {
    }
}
