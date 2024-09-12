#ifndef _kdt_h
#define _kdt_h

#include <stdint.h>

#define KDT_HEADER_FLAG_TWIDDLE 0x01
#define KDT_HEADER_FLAG_VQ 0x02
#define KDT_HEADER_FLAG_SUBSLICE 0x04
#define KDT_HEADER_FLAG_UPERSLICE 0x08
#define KDT_HEADER_FLAG_LZSS 0x10
#define KDT_HEADER_FLAG_OPACITYSLICE 0x20

#define KDT_MAX_DIMMEN 1024
#define KDT_VQ_CODEBOOK_SIZE (sizeof(uint16_t) * 4 * 256)

#define KDT__NAME_MAKE(s) (uint32_t)(s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24))
#define KDT_SIGNATURE KDT__NAME_MAKE("kdt\x20")

typedef enum {
    KDT_PixelFormat_YUV422 = 0,
    KDT_PixelFormat_RGB565 = 1,
    KDT_PixelFormat_ARGB1555 = 2,
    KDT_PixelFormat_ARGB4444 = 3,
    KDT_PixelFormat_NONE = 15,
} KDT_PixelFormat;

typedef struct __attribute__((__packed__)) {
    uint32_t signature;
    uint8_t flags;
    uint8_t pixel_format;
    uint16_t encoded_width;
    uint16_t encoded_height;
    uint16_t original_width;
    uint16_t original_height;
    uint16_t frame_width;
    uint16_t frame_height;
    uint16_t subslice_cutoff_x;
    uint16_t subslice_cutoff_y;
    uint16_t padding_width;
    uint16_t padding_height;
    uint16_t reserved;
    uint8_t data[];
} KDT;

#endif
