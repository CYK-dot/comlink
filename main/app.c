/**
 * @file    app.c
 * @brief   app demo：PA7 每 500ms 翻转一次（慢闪）
 *
 * @note    时钟由 bootloader 配置，app 继承，不再重复 SetSysClock。
 */
#include "CH57x_common.h"

#define LED_PORT_PIN     GPIO_Pin_7
#define LED_HALF_MS      500

/**
 * @brief 启动阶梯诊断：按 LED 图案定位卡死在哪一步，定位完置 0
 *        [0] 翻转 PA7 并保持数秒 —— 说明已进入 main
 *        [1] 连闪 6 次           —— 说明 main 能正常跑完循环
 *        [2] 慢闪                —— 说明完整 BSP 路径可用
 */
#ifndef APP_BRINGUP_LADDER
#define APP_BRINGUP_LADDER 1
#endif

#define BUSY(n)  do { for(volatile uint32_t _b = 0; _b < (uint32_t)(n); _b++) { } } while(0)

int main(void)
{
    __asm volatile("fence.i");   /* 作废 boot 阶段可能残留的指令缓存 */

    GPIOA_ModeCfg(LED_PORT_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(LED_PORT_PIN);

    for(;;)
    {
        GPIOA_InverseBits(LED_PORT_PIN);
        mDelaymS(LED_HALF_MS);
    }
}
