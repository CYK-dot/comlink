/**
 * @file    tusb_config.h
 * @brief   TinyUSB configuration for the CH32X035 CDC demo
 *
 * Only the device stack and the CDC class are enabled; the application drives
 * tud_task() from its own AO, so no RTOS/OSAL glue is needed inside TinyUSB.
 */
#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_TUSB_MCU
#error "CFG_TUSB_MCU must be supplied by the platform_usb target"
#endif
#define CFG_TUSB_OS             OPT_OS_NONE
#define CFG_TUSB_DEBUG          0

/* RHPort 0 is a full-speed device port. */
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE

#define CFG_TUD_ENABLED         1
#define CFG_TUD_MAX_SPEED       OPT_MODE_FULL_SPEED

#define CFG_TUD_ENDPOINT0_SIZE  64

/* One CDC-ACM function (notification + data interfaces). */
#define CFG_TUD_CDC             1
#define CFG_TUD_CDC_RX_BUFSIZE  256
#define CFG_TUD_CDC_TX_BUFSIZE  256
#define CFG_TUD_CDC_EP_BUFSIZE  64

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))
#endif

#ifdef __cplusplus
}
#endif

#endif /* TUSB_CONFIG_H_ */
