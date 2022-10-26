#ifndef _oggutil_h
#define _oggutil_h

#define OGGUTIL_CODEC_ERROR 0
#define OGGUTIL_CODEC_VORBIS 1
#define OGGUTIL_CODEC_OPUS 2
#define OGGUTIL_CODEC_UNKNOWN 3

uint8_t oggutil_get_ogg_codec(FileHandle_t* ogg_fd);

#endif