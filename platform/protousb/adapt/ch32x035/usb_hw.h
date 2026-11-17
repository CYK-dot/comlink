/**
 * @file    usb_hw.h
 * @brief   CH32X035 USB peripheral glue (clock, pins, IRQ vector)
 */
#ifndef USB_HW_H
#define USB_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Enable the USB clock, configure D+/D- and the USB pull-up */
void usb_hw_init(void);

/** @brief Nothing to do: the DCD enables the internal pull-up in dcd_connect() */
void usb_hw_connect(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_HW_H */
