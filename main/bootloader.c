/**
 * @file    bootloader.c
 * @brief   bootloader：LED 快闪 5 秒后校验并跳转到 app（与芯片无关）
 *
 * 镜像被拒绝时用短闪次数报出原因（1 = 魔数缺失，2 = 长度越界，3 = CRC 不符），
 * 随后停顿 1 秒，循环等待升级。
 */
#include "bsp_sys.h"
#include "bspboot.h"
#include "bspdbg.h"

#define BOOT_WAIT_MS     5000u
#define BOOT_BLINK_MS    50u
#define BOOT_CODE_MS     120u
#define BOOT_PAUSE_MS    1000u

static void wait_for_upgrade(bspboot_img_t reason)
{
    for(;;) {
        for(uint8_t i = 0; i < (uint8_t)reason; i++) {
            bspdbg_led_toggle();
            bspdbg_poll_delayms(BOOT_CODE_MS);
            bspdbg_led_toggle();
            bspdbg_poll_delayms(BOOT_CODE_MS);
        }
        bspdbg_poll_delayms(BOOT_PAUSE_MS);
    }
}

int main(void)
{
    bsp_sys_init();                    /* 时钟与系统初始化由 BSP 负责，app 继承 */
    bspdbg_led_init();

    for(uint32_t t = 0; t < BOOT_WAIT_MS; t += 2 * BOOT_BLINK_MS) {
        bspdbg_led_toggle();
        bspdbg_poll_delayms(BOOT_BLINK_MS);
        bspdbg_led_toggle();
        bspdbg_poll_delayms(BOOT_BLINK_MS);
    }

    bspboot_img_t const img = bspboot_check_app();
    if(img != BSPBOOT_IMG_OK) {
        wait_for_upgrade(img);
    }

    bspboot_launch_app();              /* 不返回 */
    for(;;) {
    }
}
