/**
 * @file    app_meta.h
 * @brief   Image header layout shared by the bootloader and the application
 *
 * Layout of an application image:
 *
 *   base + 0x00   .init            4 B   j handle_reset, the entry point
 *   base + 0x04   .imginfo.magic   4 B   written by the signer
 *   base + 0x08   .imginfo.length  4 B   written by the signer
 *   base + 0x0C   .imginfo.crc32   4 B   written by the signer
 *   base + 0x10   .metainfo        4 B   version, written at build time
 *   base + 0x14   payload               CRC covers [0x14, 0x14 + length)
 *
 * The base address and slot size are not compiled in: they are linker symbols
 * (_app_image_base / _app_image_limit) provided by the bootloader's own linker
 * script, so the script stays their only definition.
 */
#ifndef APP_META_H
#define APP_META_H

#include <stdint.h>
#include "bsp_layout.h"

/** Magic word ('C' 'H' '5' '7' little-endian) */
#define APP_IMG_MAGIC BSP_IMG_MAGIC

/** Field offsets relative to the image base */
#define APP_IMG_INFO_OFFSET   4u   /* .imginfo.magic */
#define APP_IMG_LENGTH_OFFSET 8u   /* .imginfo.length */
#define APP_IMG_CRC_OFFSET    12u  /* .imginfo.crc32 */
#define APP_IMG_META_OFFSET   16u  /* .metainfo: version */
#define APP_IMG_DATA_OFFSET   20u  /* start of the CRC-covered payload */

/** Image information (read-only view) */
typedef struct {
    uint32_t magic;   /**< APP_IMG_MAGIC */
    uint32_t length;  /**< number of covered bytes */
    uint32_t crc32;   /**< IEEE 802.3 CRC32 */
} app_img_info_t;

/** Build-time metadata */
typedef struct {
    uint32_t version; /**< APP_VERSION */
} app_meta_t;

#endif /* APP_META_H */
