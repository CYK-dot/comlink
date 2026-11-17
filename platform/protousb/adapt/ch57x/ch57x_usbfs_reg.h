/*
 * Register map for the CH57x (CH572/CH570) USB full-speed device controller.
 *
 * The IP is the WCH USBFS family used by CH58x/CH32X035: EP0-3 DMA at 0x10,
 * the EP0-4 control block at 0x20 (TX length + combined control), EP4 shares
 * EP0's DMA buffer. CH57x has no EP5-7, so the corresponding fields exist only
 * to keep the register layout identical and are never accessed (EP_MAX is 4).
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef CH57X_USBFS_REG_H
#define CH57X_USBFS_REG_H

#include <stddef.h>
#include "common/tusb_common.h"
#include "CH57x_common.h"

#define CH57X_USBFS 1

typedef struct {
    __IO uint16_t DMA;
    __IO uint16_t reserved;
} ch57x_ep_dma_t;

typedef struct {
    __IO uint8_t T_LEN;
    __IO uint8_t reserved0;
    __IO uint8_t CTRL;
    __IO uint8_t reserved1;
} ch57x_ep_ctrl_t;

typedef struct {
    __IO uint8_t  BASE_CTRL;        /* 0x00 R8_USB_CTRL */
    __IO uint8_t  UDEV_CTRL;        /* 0x01 R8_UDEV_CTRL */
    __IO uint8_t  INT_EN;           /* 0x02 R8_USB_INT_EN */
    __IO uint8_t  DEV_ADDR;         /* 0x03 R8_USB_DEV_AD */
    __IO uint8_t  Reserve0;         /* 0x04 */
    __IO uint8_t  MIS_ST;           /* 0x05 R8_USB_MIS_ST */
    __IO uint8_t  INT_FG;           /* 0x06 R8_USB_INT_FG */
    __IO uint8_t  INT_ST;           /* 0x07 R8_USB_INT_ST */
    __IO uint8_t  RX_LEN;           /* 0x08 R8_USB_RX_LEN (8-bit) */
    __IO uint8_t  Reserve1[3];      /* 0x09..0x0B */
    __IO uint8_t  UEP4_1_MOD;       /* 0x0C R8_UEP4_1_MOD */
    __IO uint8_t  UEP2_3_MOD;       /* 0x0D R8_UEP2_3_MOD */
    __IO uint8_t  UEP567_MOD;       /* 0x0E (not present on CH57x) */
    __IO uint8_t  Reserve2;         /* 0x0F */
    ch57x_ep_dma_t  EP_DMA_0_3[4];  /* 0x10 EP0-3 DMA (EP4 shares EP0) */
    ch57x_ep_ctrl_t EP_CTRL_0_4[5]; /* 0x20 EP0-4 length/control */
    __IO uint8_t  Reserve3[0x54u - 0x34u];
    ch57x_ep_dma_t  EP_DMA_5_7[3];  /* not present on CH57x */
    __IO uint8_t  Reserve4[0x64u - 0x60u];
    ch57x_ep_ctrl_t EP_CTRL_5_7[3]; /* not present on CH57x */
} USBOTG_FS_TypeDef;

#define USBOTG_FS ((USBOTG_FS_TypeDef *)0x40008000u)

TU_VERIFY_STATIC(sizeof(ch57x_ep_dma_t) == 4, "CH57x EP DMA slot must be 4 bytes");
TU_VERIFY_STATIC(sizeof(ch57x_ep_ctrl_t) == 4, "CH57x EP ctrl slot must be 4 bytes");
TU_VERIFY_STATIC(offsetof(USBOTG_FS_TypeDef, EP_DMA_0_3) == 0x10, "CH57x EP_DMA_0_3 @0x10");
TU_VERIFY_STATIC(offsetof(USBOTG_FS_TypeDef, EP_CTRL_0_4) == 0x20, "CH57x EP_CTRL_0_4 @0x20");

/* Combined per-endpoint control: IN response bits [1:0], OUT response [3:2]. */
#define CH32_USBFS_EP_CTRL_COMBINED 1
/* CH57x keeps the hardware AUTO_TOG: WCH's own CH57x device library enables
 * RB_UEP_AUTO_TOG on EP1-EP3. The CH58x manual-toggle workaround does not apply
 * to this older controller and actively breaks the data toggle. */
/* #define CH32_USBFS_EP4_MANUAL_TOG 1 */
/* EP4 has no DMA register of its own: it overlays EP0's buffer. */
#define CH32_USBFS_EP4_SHARES_EP0   1

#define USBHD_IRQn USB_IRQn

#ifndef NVIC_EnableIRQ
#define NVIC_EnableIRQ(n)  PFIC_EnableIRQ(n)
#define NVIC_DisableIRQ(n) PFIC_DisableIRQ(n)
#endif

/* CTRL */
#define USBFS_CTRL_DMA_EN    (1 << 0)
#define USBFS_CTRL_CLR_ALL   (1 << 1)
#define USBFS_CTRL_RESET_SIE (1 << 2)
#define USBFS_CTRL_INT_BUSY  (1 << 3)
#define USBFS_CTRL_SYS_CTRL  (1 << 4)
#define USBFS_CTRL_DEV_PUEN  (1 << 5)
#define USBFS_CTRL_LOW_SPEED (1 << 6)
#define USBFS_CTRL_HOST_MODE (1 << 7)

/* INT_EN */
#define USBFS_INT_EN_BUS_RST  (1 << 0)
#define USBFS_INT_EN_DETECT   (1 << 0)
#define USBFS_INT_EN_TRANSFER (1 << 1)
#define USBFS_INT_EN_SUSPEND  (1 << 2)
#define USBFS_INT_EN_HST_SOF  (1 << 3)
#define USBFS_INT_EN_FIFO_OV  (1 << 4)
#define USBFS_INT_EN_DEV_NAK  (1 << 6)
#define USBFS_INT_EN_DEV_SOF  (1 << 7)

/* INT_FG */
#define USBFS_INT_FG_BUS_RST  (1 << 0)
#define USBFS_INT_FG_DETECT   (1 << 0)
#define USBFS_INT_FG_TRANSFER (1 << 1)
#define USBFS_INT_FG_SUSPEND  (1 << 2)
#define USBFS_INT_FG_HST_SOF  (1 << 3)
#define USBFS_INT_FG_FIFO_OV  (1 << 4)
#define USBFS_INT_FG_SIE_FREE (1 << 5)
#define USBFS_INT_FG_TOG_OK   (1 << 6)
#define USBFS_INT_FG_IS_NAK   (1 << 7)

/* MIS_ST */
#define USBFS_MIS_ST_SUSPEND  (1 << 2)

/* INT_ST */
#define USBFS_INT_ST_MASK_UIS_ENDP(x)  (((x) >> 0) & 0x0F)
#define USBFS_INT_ST_MASK_UIS_TOKEN(x) (((x) >> 4) & 0x03)
#define USBFS_INT_ST_TOG_OK            (1 << 6)

/* UDEV_CTRL */
#define USBFS_UDEV_CTRL_PORT_EN   (1 << 0)
#define USBFS_UDEV_CTRL_GP_BIT    (1 << 1)
#define USBFS_UDEV_CTRL_LOW_SPEED (1 << 2)
#define USBFS_UDEV_CTRL_DM_PIN    (1 << 4)
#define USBFS_UDEV_CTRL_DP_PIN    (1 << 5)
#define USBFS_UDEV_CTRL_PD_DIS    (1 << 7)

/* TX_CTRL */
#define USBFS_EP_T_RES_MASK (3 << 0)
#define USBFS_EP_T_TOG      (1 << 2)
#define USBFS_EP_T_AUTO_TOG (1 << 3)

#define USBFS_EP_T_RES_ACK   (0 << 0)
#define USBFS_EP_T_RES_NYET  (1 << 0)
#define USBFS_EP_T_RES_NAK   (2 << 0)
#define USBFS_EP_T_RES_STALL (3 << 0)

/* RX_CTRL */
#define USBFS_EP_R_RES_MASK (3 << 0)
#define USBFS_EP_R_TOG      (1 << 2)
#define USBFS_EP_R_AUTO_TOG (1 << 3)

#define USBFS_EP_R_RES_ACK   (0 << 0)
#define USBFS_EP_R_RES_NYET  (1 << 0)
#define USBFS_EP_R_RES_NAK   (2 << 0)
#define USBFS_EP_R_RES_STALL (3 << 0)

/* Combined control register encoding */
#define USBFS_EPC_T_RES_MASK  0x03
#define USBFS_EPC_R_RES_MASK  0x0C
#define USBFS_EPC_R_RES_SHIFT 2
#define USBFS_EPC_AUTO_TOG    0x10
#define USBFS_EPC_T_TOG       0x40
#define USBFS_EPC_R_TOG       0x80

/* token PID */
#define PID_OUT   0
#define PID_SOF   1
#define PID_IN    2
#define PID_SETUP 3

#endif /* CH57X_USBFS_REG_H */
