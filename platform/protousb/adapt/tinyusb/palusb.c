/**
 * @file    palusb.c
 * @brief   Platform Abstraction Layer: USB device (TinyUSB backend)
 *
 * palusb_task() is driven by the application (an OSAL AO), which keeps the USB
 * stack out of interrupt context and out of the QP scheduler internals.
 */
#include "tusb.h"
#include "palusb.h"
#include "usb_hw.h"

void palusb_init(void)
{
    usb_hw_init();
    (void)tusb_rhport_init(0, NULL);
    usb_hw_connect();
}

void palusb_task(void)
{
    tud_task();
}

bool palusb_connected(void)
{
    return tud_cdc_connected();
}

void palusb_send_string(char const *s)
{
    /*
     * Do not gate on tud_cdc_connected(): that reflects DTR, and plenty of
     * serial terminals open the port without raising it. The TX stream only
     * needs the endpoint to be open, which the CDC class does on
     * SET_CONFIGURATION.
     */
    if (tud_cdc_write_str(s) > 0u) {
        (void)tud_cdc_write_flush();
    }
}
