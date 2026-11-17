/**
 * @file    bspboot.h
 * @brief   Bootloader helpers: validate an application image and jump into it
 *
 * Chip independent entry points for a bootloader built on this BSP:
 *
 *   if (bspboot_check_app() != BSPBOOT_IMG_OK) { ... wait for an update ... }
 *   bspboot_launch_app();              // never returns
 *
 * The application image format is defined by the adapter (app_meta.h) and
 * written by bsp_add_image_signed().
 */
#ifndef BSPBOOT_H
#define BSPBOOT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Why an application image was rejected */
typedef enum {
    BSPBOOT_IMG_OK = 0,     /**< valid image, may be launched */
    BSPBOOT_IMG_BAD_MAGIC,  /**< magic word missing: nothing programmed */
    BSPBOOT_IMG_BAD_LENGTH, /**< payload length outside the slot */
    BSPBOOT_IMG_BAD_CRC,    /**< payload does not match its CRC32 */
} bspboot_img_t;

/**
 * @brief  Check the application image
 *
 * Validates the magic word, the payload length (bounded by the slot size) and
 * the CRC32 of the payload.
 *
 * @return BSPBOOT_IMG_OK, or the first check that failed
 */
bspboot_img_t bspboot_check_app(void);

/**
 * @brief  Jump into the application image
 *
 * Disables interrupts and branches to the image entry point (the .init section
 * at the image base). Does not return.
 */
void bspboot_launch_app(void);

#ifdef __cplusplus
}
#endif

#endif /* BSPBOOT_H */
