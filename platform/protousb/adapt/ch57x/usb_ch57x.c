/**
 * @file    usb_ch57x.c
 * @brief   CH572/CH570 USB peripheral glue
 *
 * CH57x controls the USB analog front-end and the D+ pull-up through
 * R16_PIN_ALTERNATE. The pull-up is kept off while the bootloader runs and
 * until TinyUSB has initialised the controller, so the host sees exactly one
 * clean attach instead of a device that cannot answer.
 */
#include "CH57x_common.h"
#include "bsp_cpu.h"
#include "device/dcd.h"
#include "usb_hw.h"

void usb_hw_init(void)
{
    /*
     * Force a real detach first: the CH572 ROM presents a USB ISP device on
     * every reset, and if D+ never goes low again the host keeps the failed
     * device and never re-enumerates. Holding D+ low for 300 ms gives the host
     * an unambiguous disconnect; the attach happens later in usb_hw_connect().
     *
     * RB_PIN_DEBUG_EN must be cleared: the debug interface shares the USB
     * pins, and WCH's own CH572 USB example does exactly this before touching
     * the USB controller.
     */
    R8_USB_CTRL = 0x00;
    R16_PIN_ALTERNATE &= ~(RB_PIN_USB_EN | RB_UDP_PU_EN | RB_PIN_DEBUG_EN);
    mDelaymS(300);

    R16_PIN_ALTERNATE |= RB_PIN_USB_EN;      /* analog front-end on, D+ still low */
}

void usb_hw_connect(void)
{
    R16_PIN_ALTERNATE |= RB_UDP_PU_EN;
    R8_USB_CTRL |= RB_UC_DEV_PU_EN;
}

void USB_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USB_IRQHandler(void)
{
    bsp_cpu_isr_enter();
    dcd_int_handler(0);
    bsp_cpu_isr_exit();
}
