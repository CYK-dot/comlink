/**
 * @file    palusb.h
 * @brief   Platform Abstraction Layer: USB device
 *
 * The application talks to this interface only; the underlying stack
 * (TinyUSB today) and the chip controller driver live behind it.
 */
#ifndef PALUSB_H
#define PALUSB_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Bring up the USB peripheral, controller driver and device stack */
void palusb_init(void);

/** @brief Run one pass of the USB device task (call from an OSAL AO) */
void palusb_task(void);

/** @brief True once the host has configured the CDC interface */
bool palusb_connected(void);

/** @brief Queue a string on the CDC IN endpoint */
void palusb_send_string(char const *s);

#ifdef __cplusplus
}
#endif

#endif /* PALUSB_H */
