/**
 * @file    usb_ch32x035.c
 * @brief   CH32X035 USB peripheral glue (clock, pins, IRQ vector)
 *
 * The WCH USBFS block needs three things the generic DCD does not do:
 * the USBFS clock, PC16/PC17 as the D+/D- pins, and the AFIO USB IO enable
 * with the right pull-up for the detected VDD (5 V vs 3.3 V). The IRQ vector
 * is owned here as well so the DCD stays chip-agnostic.
 */
#include "ch32x035.h"
#include "bsp_cpu.h"
#include "device/dcd.h"
#include "usb_hw.h"

void usb_hw_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBFS, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* PC16 = UDM, PC17 = UDP */
    gpio.GPIO_Pin  = GPIO_Pin_16;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &gpio);

    gpio.GPIO_Pin  = GPIO_Pin_17;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &gpio);

    if (PWR_VDD_SupplyVoltage() == PWR_VDD_5V) {
        AFIO->CTLR = (AFIO->CTLR
                      & ~(AFIO_CTLR_UDP_PUE | AFIO_CTLR_UDM_PUE | AFIO_CTLR_USB_PHY_V33))
                     | 0x08u /* UDP_PUE_10K */
                     | AFIO_CTLR_USB_IOEN;
    } else {
        AFIO->CTLR = (AFIO->CTLR & ~(AFIO_CTLR_UDP_PUE | AFIO_CTLR_UDM_PUE))
                     | AFIO_CTLR_USB_PHY_V33
                     | 0x0Cu /* UDP_PUE_1K5 */
                     | AFIO_CTLR_USB_IOEN;
    }
}

void usb_hw_connect(void)
{
    /* The DCD's dcd_connect() already enabled the internal pull-up. */
}

void USBFS_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USBFS_IRQHandler(void)
{
    bsp_cpu_isr_enter();
    dcd_int_handler(0);
    bsp_cpu_isr_exit();
}
