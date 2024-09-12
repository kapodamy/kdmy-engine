#ifndef _kdm_h
#define _kdm_h

#include <stdbool.h>
#include <stdint.h>

#define KDM_FLAGS_PACKET_VIDEO_KEYFRAME 0x01

#define KDM_FLAGS_HEADER_HAS_VIDEO 0x01
#define KDM_FLAGS_HEADER_HAS_AUDIO 0x02
#define KDM_FLAGS_HEADER_AUDIO_STEREO 0x20
#define KDM_FLAGS_HEADER_RESERVED1 0x40
#define KDM_FLAGS_HEADER_RESERVED2 0x80

#define KDM_MAX_TEXTURE_DIMMEN 1024

#define KDM__NAME_MAKE(s) (uint32_t)(s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24))
#define KDM_SIGNATURE KDM__NAME_MAKE("KDM\x20")

#define KDM__HAS_FLAG(flags, flag) ((flags & flag) != 0x00)

typedef struct __attribute__((__packed__)) {
    uint32_t timestamp; // expressed in nanoseconds (1000ms)
    uint32_t offset;
} KDMCue;


typedef struct __attribute__((__packed__)) {
    uint32_t signature;
    uint8_t version;
    uint8_t flags;
    uint16_t cue_table_length;
    uint32_t estimated_duration_in_milliseconds;

    uint16_t video_encoded_width;
    uint16_t video_encoded_height;
    uint16_t video_original_width;
    uint16_t video_original_height;
    float video_fps;

    uint16_t audio_frequency;

    // uint32_t cue_table[];
    // KDMPacket packets[];
} KDMFileHeader;

typedef struct __attribute__((__packed__)) {
    uint16_t packet_flags;
    int32_t video_data_size;
    int32_t audio_data_size;
    // uint8_t data_video[];
    // uint8_t data_audio[];
} KDMPacketHeader;

#endif
