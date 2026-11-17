/**
 * @file    bspboot.c
 * @brief   Bootloader helpers for CH32X03x
 */
#include "bspboot.h"
#include "app_meta.h"
#include "ch32x035.h"

/* IEEE 802.3 CRC32 (reflected, init/xorout 0xFFFFFFFF); must match the signer */
static uint32_t crc32_byte(uint32_t crc, uint8_t byte)
{
    crc ^= byte;
    for(uint8_t bit = 0; bit < 8; bit++)
    {
        crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return crc;
}

static uint32_t crc32_bytes(const void *data, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFu;

    while(len--)
    {
        crc = crc32_byte(crc, *p++);
    }
    return ~crc;
}

/* Word-wise variant: byte loads from CodeFlash are not reliable on every
   QingKe generation, so a match through either path is accepted. */
static uint32_t crc32_words(const void *data, uint32_t len)
{
    const volatile uint32_t *p = (const volatile uint32_t *)data;
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t words = len >> 2;

    while(words--)
    {
        uint32_t w = *p++;
        for(uint8_t i = 0; i < 4; i++)
        {
            crc = crc32_byte(crc, (uint8_t)(w & 0xFFu));
            w >>= 8;
        }
    }

    const uint8_t *tail = (const uint8_t *)p;
    for(uint32_t i = 0; i < (len & 3u); i++)
    {
        crc = crc32_byte(crc, tail[i]);
    }
    return ~crc;
}

/* Provided by layout/boot.ld: the application slot, declared there so no build
 * script has to read the linker script. The symbols are flash addresses, so the
 * alignment is stated here to keep the header reads word-sized. */
extern const uint8_t _app_image_base[] __attribute__((aligned(4)));
extern const uint8_t _app_image_limit[] __attribute__((aligned(4)));

bspboot_img_t bspboot_check_app(void)
{
    const app_img_info_t *info =
        (const app_img_info_t *)(const void *)(_app_image_base + APP_IMG_INFO_OFFSET);
    uint32_t slot = (uint32_t)(_app_image_limit - _app_image_base);
    const void *payload = (const void *)(_app_image_base + APP_IMG_DATA_OFFSET);

    if(info->magic != APP_IMG_MAGIC)
    {
        return BSPBOOT_IMG_BAD_MAGIC;
    }
    if(info->length == 0u || info->length > slot - APP_IMG_DATA_OFFSET)
    {
        return BSPBOOT_IMG_BAD_LENGTH;
    }
    if((((uintptr_t)payload & 3u) == 0u)
       && (crc32_words(payload, info->length) == info->crc32))
    {
        return BSPBOOT_IMG_OK;
    }
    if(crc32_bytes(payload, info->length) == info->crc32)
    {
        return BSPBOOT_IMG_OK;
    }
    return BSPBOOT_IMG_BAD_CRC;
}

void bspboot_launch_app(void)
{
    __disable_irq();
    ((void (*)(void))(uintptr_t)_app_image_base)();
    for(;;)
    {
    }
}
