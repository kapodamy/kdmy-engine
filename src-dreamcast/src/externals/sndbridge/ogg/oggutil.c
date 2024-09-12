#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef SNDBRINDE_TREMOR
#include <vorbis/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif

#include "logger.h"
#include "externals/sndbridge/oggutil.h"
#include "math2d.h"

#define STRING_START_WITH(str, substr) strncmp(substr, str, strlen(substr)) == 0

#define LOOPSTART "LOOPSTART="
#define LOOPLENGTH "LOOPLENGTH="
#define CODEC_ERROR 0
#define CODEC_VORBIS 1
#define CODEC_OPUS 2
#define CODEC_UNKNOWN 3

#define OggS 0x5367674F
#define VERSION 0x00
#define FLAG_FIRST 0x02



static const uint8_t CODEC_OPUS_SIG[] = {0x4f, 0x70, 0x75, 0x73, 0x48, 0x65, 0x61, 0x64}; // OpusHead
static const uint8_t CODEC_VORBIS_SIG[] = {0x01, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73};     //\x01vorbis


struct __attribute__((__packed__)) OggPacket {
    int32_t magic;      // "OggS" binary string in little-endian
    uint8_t version;    // version
    uint8_t packetFlag; // type

    uint64_t granPos; // granulate position

    int32_t streamId;      // bitstream serial number
    int32_t sequenceCount; // page sequence number

    int32_t checksum; // page checksum

    uint8_t segmentTableSize; // segment size
    //  uint8_t segmentTable[];    // segment table
};


static uint8_t oggutil_get_ogg_codec(SourceHandle* ogg_fd) {
    struct OggPacket oggheader;
    const int32_t oggheader_size = sizeof(oggheader);
    int64_t original_offset = ogg_fd->tell(ogg_fd);
    uint8_t codec = CODEC_UNKNOWN;

    int32_t max_codec_size = math2d_max_int(sizeof(CODEC_VORBIS_SIG), sizeof(CODEC_OPUS_SIG));

    if (ogg_fd->read(ogg_fd, &oggheader, oggheader_size) != oggheader_size) goto L_error;
    if (oggheader.magic != OggS) {
        logger_warn("oggutil_get_ogg_codec() failed, ¿endianess problem?");
        goto L_error;
    }
    if (oggheader.version != VERSION) goto L_error;
    if (oggheader.packetFlag != FLAG_FIRST) goto L_error;
    if (oggheader.segmentTableSize < 1) goto L_error;

    {
        // read segment table
        uint8_t segmentTable[oggheader.segmentTableSize];
        if (ogg_fd->read(ogg_fd, segmentTable, oggheader.segmentTableSize) != oggheader.segmentTableSize) {
            goto L_error;
        }

        // parse segment table
        uint16_t segment_length = 0;
        for (int32_t i = 0; i < oggheader.segmentTableSize; i++) {
            segment_length += segmentTable[i];
        }
        // read codec data
        uint8_t codec_data[segment_length];

        if (segment_length < max_codec_size) goto L_error;

        if (ogg_fd->read(ogg_fd, codec_data, segment_length) != segment_length) {
            goto L_error;
        }

        // guess if the codec is vorbis
        codec = CODEC_VORBIS;
        for (uint8_t i = 0; i < sizeof(CODEC_VORBIS_SIG); i++) {
            if (codec_data[i] != CODEC_VORBIS_SIG[i]) {
                codec = CODEC_UNKNOWN;
                break;
            }
        }
        if (codec != CODEC_UNKNOWN) goto L_return;

        // guess if the codec is opus
        codec = CODEC_OPUS;
        for (uint8_t i = 0; i < sizeof(CODEC_OPUS_SIG); i++) {
            if (codec_data[i] != CODEC_OPUS_SIG[i]) {
                codec = CODEC_UNKNOWN;
                break;
            }
        }
        if (codec != CODEC_UNKNOWN) goto L_return;
    }

L_return:
    ogg_fd->seek(ogg_fd, original_offset, SEEK_SET);
    return codec;
L_error:
    ogg_fd->seek(ogg_fd, original_offset, SEEK_SET);
    return CODEC_ERROR;
}

DecoderHandle* oggutil_init_ogg_decoder(SourceHandle* src_hnd) {
    // assert(src_hnd != NULL);
    // assert(src_hnd->read != NULL);
    // assert(src_hnd->seek != NULL);
    // assert(src_hnd->tell != NULL);

    int32_t result = oggutil_get_ogg_codec(src_hnd);
    DecoderHandle* dec;

    switch (result) {
        case CODEC_VORBIS:
            dec = oggvorbisdecoder_init(src_hnd);
            break;
#if OPUS_SUPPORT
        case CODEC_OPUS:
            dec = oggopusdecoder_init(src_hnd);
#error "backport the oggopusdecoder (from C#)"
            break;
#else
            logger_error("oggutil_init_ogg_decoder() opus audio codec not supported");
            return NULL;
#endif
        case CODEC_ERROR:
            logger_error("oggutil_init_ogg_decoder() can not identify the audio codec");
            return NULL;
        case CODEC_UNKNOWN:
            logger_error("oggutil_init_ogg_decoder() unknown audio codec");
            return NULL;
        default:
            return NULL;
    }

    if (dec == NULL) {
        const char* codec = result == CODEC_OPUS ? "opus" : "vorbis";
        logger_error("oggutil_init_ogg_decoder() can not initialize the decoder for: %s", codec);
        return NULL;
    }

    return dec;
}

void oggutil_find_loop_points(int count, char** comments, int* lengths, int64_t* loop_start, int64_t* loop_length) {
    for (int32_t i = 0; i < count; i++) {
        int32_t length = lengths[i];
        const char* comment = comments[i];

        if (length < 1 || comment == NULL) continue;

        if (STRING_START_WITH(LOOPSTART, comment)) {
            *loop_start = strtol(comment + sizeof(LOOPSTART), NULL, 10);
        }

        if (STRING_START_WITH(LOOPLENGTH, comment)) {
            *loop_length = strtol(comment + sizeof(LOOPLENGTH), NULL, 10);
        }
    }
}
