#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "filehandle.h"
#include "oggutil.h"

static const uint32_t OggS = 0x5367674F;
static const uint8_t VERSION = 0x00;
static const uint8_t FLAG_FIRST = 0x02;

#ifdef SNDBRIDGE_OPUS_DECODING
static const uint8_t CODEC_OPUS[] = { 0x4f, 0x70, 0x75, 0x73, 0x48, 0x65, 0x61, 0x64 }; // OpusHead
static const uint8_t CODEC_OPUS_SIZE = sizeof(CODEC_OPUS);
#endif

static const uint8_t CODEC_VORBIS[] = { 0x01, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73 }; //\x01vorbis
static const uint8_t CODEC_VORBIS_SIZE = sizeof(CODEC_VORBIS);


#pragma pack(push, 1)
struct OggPacket_t {
    uint32_t magic;     // "OggS" binary string in little-endian
    uint8_t version;    // version
    uint8_t packetFlag; // type

    uint64_t granPos; // granulate position

    uint32_t streamId;      // bitstream serial number
    uint32_t sequenceCount; // page sequence number

    uint32_t checksum; // page checksum

    uint8_t segmentTableSize; // segment size
    // uint8_t* segmentTable;    // segment table
};
#pragma pack(pop)


uint8_t oggutil_get_ogg_codec(FileHandle_t* ogg_fd) {
    struct OggPacket_t oggheader;
    const ssize_t oggheader_size = sizeof(struct OggPacket_t);
    int64_t original_offset = file_tell(ogg_fd);

#ifdef SNDBRIDGE_OPUS_DECODING
    const uint8_t max_codec_size = CODEC_VORBIS_SIZE > CODEC_OPUS_SIZE ? CODEC_VORBIS_SIZE : CODEC_OPUS_SIZE;
#else
    const uint8_t max_codec_size = CODEC_VORBIS_SIZE;
#endif

    if (file_read(ogg_fd, &oggheader, oggheader_size) != oggheader_size) goto L_error;
    if (oggheader.magic != OggS) {
        fprintf(stderr, "soundplayer_get_ogg_codec() OggS failed, ¿endianess problem?\n");
        goto L_error;
    }
    if (oggheader.version != VERSION) goto L_error;
    if (oggheader.packetFlag != FLAG_FIRST) goto L_error;
    if (oggheader.segmentTableSize < 1) goto L_error;

    // read segment table
    uint8_t* segmentTable = malloc(oggheader.segmentTableSize);
    assert(segmentTable);
    if (file_read(ogg_fd, segmentTable, oggheader.segmentTableSize) != oggheader.segmentTableSize) {
        free(segmentTable);
        goto L_error;
    }

    // parse segment table
    uint16_t segment_length = 0;
    for (size_t i = 0; i < oggheader.segmentTableSize; i++) {
        segment_length += segmentTable[i];
    }
    free(segmentTable);

    if (segment_length < max_codec_size) goto L_error;

    // read codec data
    uint8_t* codec_data = malloc(segment_length);
    assert(codec_data);
    if (file_read(ogg_fd, codec_data, segment_length) != segment_length) {
        free(codec_data);
        goto L_error;
    }

    // guess if the codec is vorbis
    uint8_t codec = OGGUTIL_CODEC_VORBIS;
    for (uint8_t i = 0; i < CODEC_VORBIS_SIZE; i++) {
        if (codec_data[i] != CODEC_VORBIS[i]) {
            codec = OGGUTIL_CODEC_UNKNOWN;
            break;
        }
    }
    if (codec != OGGUTIL_CODEC_UNKNOWN) goto L_return;

#ifdef SNDBRIDGE_OPUS_DECODING
    // guess if the codec is opus
    codec = OGGUTIL_CODEC_OPUS;
    for (uint8_t i = 0; i < CODEC_OPUS_SIZE; i++) {
        if (codec_data[i] != CODEC_OPUS[i]) {
            codec = OGGUTIL_CODEC_UNKNOWN;
            break;
        }
    }
    if (codec != OGGUTIL_CODEC_UNKNOWN) goto L_return;
#endif

L_return:
    free(codec_data);
    file_seek(ogg_fd, original_offset, SEEK_SET);
    return codec;
L_error:
    file_seek(ogg_fd, original_offset, SEEK_SET);
    return OGGUTIL_CODEC_ERROR;
}
