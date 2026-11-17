/**
 * @file    app_metainfo.c
 * @brief   镜像头的占位段：info 三个字段由 bsp_add_image_signed 回填，版本号编译期写入
 */
#include "app_meta.h"

#ifndef APP_VERSION
#define APP_VERSION 0x00010000u
#endif

/* 占位值，签名脚本会写入真正的 magic/length/crc32 */
__attribute__((section(".imginfo.magic"), used))
const uint32_t app_img_magic = 0u;

__attribute__((section(".imginfo.length"), used))
const uint32_t app_img_length = 0u;

__attribute__((section(".imginfo.crc32"), used))
const uint32_t app_img_crc32 = 0u;

__attribute__((section(".metainfo"), used))
const app_meta_t app_meta = {
    .version = APP_VERSION,
};
