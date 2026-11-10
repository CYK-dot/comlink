/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Description        : 极简 demo —— PA7 每 500ms 翻转一次（亮 500ms / 灭 500ms）
 *
 * BSP: bsp/sdk/ch57x（git submodule），极简模式，使用 BSP 内置启动文件与链接脚本
 *******************************************************************************/

#include "CH57x_common.h"

#define LED_PORT_PIN     GPIO_Pin_7      /* PA7 驱动 LED */
#define LED_HALF_MS      500             /* 翻转间隔 500ms */

int main(void)
{
    /* 主频 100MHz，与 CH57x_common.h 中的 FREQ_SYS 一致，保证 mDelaymS 时基准确 */
    SetSysClock(CLK_SOURCE_HSE_PLL_100MHz);

    /* PA7 推挽输出，初始输出低电平（LED 灭） */
    GPIOA_ModeCfg(LED_PORT_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(LED_PORT_PIN);

    while(1)
    {
        GPIOA_InverseBits(LED_PORT_PIN);   /* 翻转 PA7 */
        mDelaymS(LED_HALF_MS);             /* 500ms 半周期 */
    }
}
