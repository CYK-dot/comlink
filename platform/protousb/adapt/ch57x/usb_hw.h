/**
 * @file    usb_hw.h
 * @brief   CH572/CH570 USB peripheral glue (pin mux, pull-up, IRQ vector)
 */
#ifndef USB_HW_H
#define USB_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Enable the USB analog front-end but keep D+ low (detached) */
void usb_hw_init(void);

/** @brief Turn the D+ pull-up on: the host sees a fresh attach */
void usb_hw_connect(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_HW_H */
