#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "externals/sndbridge/wavutil.h"
#include "logger.h"
#include "malloc_utils.h"

#define MAXIMUM_SUPPORTED_CHANNELS 2


typedef struct __attribute__((__packed__)) {
    uint32_t id;
    uint32_t type;
    uint32_t start;
    uint32_t end;
    uint32_t fraction;
    uint32_t play_counts;
} WavSamplePoint;

typedef struct __attribute__((__packed__)) {
    uint32_t manufacturer;
    uint32_t product;
    uint32_t sample_period;
    uint32_t midi_unity_note;
    uint32_t midi_pitch_fraction;
    uint32_t smpte_format;
    uint32_t smpte_offset;
    uint32_t sample_loops_count;
    uint32_t sample_data;
} WavSample;

typedef struct __attribute__((__packed__)) {
    uint32_t cue_point_id;        // 0|1
    uint32_t play_order_position; // 0
    uint32_t data_chunk_id;       // data
    uint32_t chunk_start;         // 0
    uint32_t block_start;         // 0
    uint32_t frame_offset;        // in samples
} WavCuePoint;



typedef struct {
    DecoderHandle handle;

    SourceHandle* src_hnd;
    int32_t data_offset;
    int32_t bytes_position;
    int32_t total_bytes;

    int32_t channels;
    uint32_t rate;
    int64_t loop_start, loop_length;
} ADPCMreader;


static inline void read_wav_cue(SourceHandle* hnd, int64_t* start, int64_t* length) {
    uint32_t cues_count;
    if (hnd->read(hnd, &cues_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        // truncated file
        return;
    }

    int next_cue_point_id = 0;
    WavCuePoint cue;
    uint32_t cue_start = 0, cue_end = 0;

    for (uint32_t i = 0; i < cues_count; i++) {
        if (hnd->read(hnd, &cue, sizeof(WavCuePoint)) != sizeof(WavCuePoint)) {
            // truncated file
            return;
        }

        if (cue.data_chunk_id != WAV_HDR_DATA) {
            continue;
        }

        if (cue.cue_point_id == next_cue_point_id) {
            if (next_cue_point_id == 0)
                cue_start = cue.frame_offset;
            else
                cue_end = cue.frame_offset;

            next_cue_point_id++;
        }

        if (next_cue_point_id > 1) {
            if (cue_start > cue_end) {
                *start = -2;
                *length = -2;
            } else {
                *start = cue_start;
                *length = cue_end - cue_start;
            }
            return;
        }
    }
}

static inline void read_wav_spml(SourceHandle* hnd, int64_t* start, int64_t* length) {
    WavSample spml;
    if (hnd->read(hnd, &spml, sizeof(WavSample)) != sizeof(WavSample)) {
        // truncated file
        return;
    }

    if (spml.midi_unity_note != 0 || spml.midi_pitch_fraction != 0) {
        goto L_invalid_fields;
    }
    if (spml.smpte_format != 0x00 || spml.smpte_offset != 0) {
        goto L_invalid_fields;
    }

    WavSamplePoint smpl_point;
    for (uint32_t i = 0; i < spml.sample_loops_count; i++) {
        if (hnd->read(hnd, &smpl_point, sizeof(WavSamplePoint)) != sizeof(WavSamplePoint)) {
            // truncated file
            return;
        }

        if (smpl_point.id == 0) {
            if (smpl_point.fraction != 0) goto L_invalid_fields;
            if (smpl_point.end < smpl_point.start) goto L_invalid_fields;

            // note 1: the "smpl_point.play_counts" field is ignored
            // note 2: the "smpl_point.end" does not exclude the last sample
            *start = smpl_point.start;
            *length = smpl_point.end - smpl_point.start;
            return;
        }
    }

    // empty smpl
    return;

L_invalid_fields:
    *start = -2;
    *length = -2;
}


static int32_t adpcmreader_readBytes(DecoderHandle* decoder, void* buffer, uint32_t buffer_size) {
    if (buffer_size < 1) return 0;

    ADPCMreader* adpcmreader = (ADPCMreader*)decoder;

    int32_t bytes_position = adpcmreader->bytes_position + (int32_t)buffer_size;
    if (bytes_position > adpcmreader->total_bytes) {
        // round-down the amount of requested samples
        buffer_size = (uint32_t)(adpcmreader->total_bytes - adpcmreader->bytes_position);
        if (buffer_size < 1) return 0;

        bytes_position = adpcmreader->total_bytes;
    }

    int32_t bytes_readed = adpcmreader->src_hnd->read(adpcmreader->src_hnd, buffer, (int32_t)buffer_size);

    if (bytes_readed < 0) {
        // an error occurred
        return 0;
    }
    if (bytes_readed < buffer_size) {
        // this never should happen
        buffer_size = (uint32_t)bytes_readed;
    }

    adpcmreader->bytes_position = bytes_position;
    return (int32_t)buffer_size;
}
static SampleFormat adpcmreader_getInfo(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, float64* duration) {
    ADPCMreader* adpcmreader = (ADPCMreader*)decoder;

    *rate = adpcmreader->rate;
    *channels = (uint32_t)adpcmreader->channels;
    *duration = ((adpcmreader->total_bytes * 2.0) / (float64)adpcmreader->rate) / adpcmreader->channels;

    return SampleFormat_ADPCM_4_YAMAHA;
}
static bool adpcmreader_seek(DecoderHandle* decoder, float64 seconds) {
    ADPCMreader* adpcmreader = (ADPCMreader*)decoder;

    float64 estimated_sample_position = adpcmreader->rate * seconds;
    int64_t bytes_position = (int64_t)((estimated_sample_position * adpcmreader->channels) / 2.0);
    int32_t data_offset = adpcmreader->data_offset;

    if (estimated_sample_position < 0) {
        adpcmreader->bytes_position = 0;
        adpcmreader->src_hnd->seek(adpcmreader->src_hnd, data_offset, SEEK_SET);
        return false;
    } else if (bytes_position > adpcmreader->total_bytes) {
        bytes_position = adpcmreader->total_bytes;

        adpcmreader->bytes_position = bytes_position;
        adpcmreader->src_hnd->seek(adpcmreader->src_hnd, bytes_position + data_offset, SEEK_SET);
        return false;
    }

    adpcmreader->bytes_position = (int32_t)bytes_position;

    int64_t ret = adpcmreader->src_hnd->seek(adpcmreader->src_hnd, bytes_position + data_offset, SEEK_SET);
    return ret == 0;
}
static void adpcmreader_getLoopPoints(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length) {
    ADPCMreader* adpcmreader = (ADPCMreader*)decoder;

    *loop_start = adpcmreader->loop_start;
    *loop_length = adpcmreader->loop_length;
}
static void adpcmreader_destroy(DecoderHandle* decoder) {
    free_chk(decoder);
}


DecoderHandle* adpcmreader_init(SourceHandle* hnd) {
    int64_t data_offset, data_length, loop_start, loop_length;
    WavFormat fmt_info;
    if (!wav_read_header(hnd, &fmt_info, &data_offset, &data_length, &loop_start, &loop_length)) return NULL;

    if (fmt_info.format != WAV_YAMAHA_AICA_ADPCM) {
        logger_warn("sndbridge: adpcmreader_init() only \"YAMAHA ADPCM\" sample format is supported");
        goto L_error_fields;
    }

    if (fmt_info.channels < 1 || fmt_info.channels > MAXIMUM_SUPPORTED_CHANNELS) {
        goto L_error_fields;
    }

    if (fmt_info.sample_rate < 1 || fmt_info.sample_rate > 48000) {
        goto L_error_fields;
    }

    if (fmt_info.block_align != 1024) {
        logger_error("sndbridge: adpcmreader_init() unknown block align, expected 1024");
        goto L_error_fields;
    }

    if (fmt_info.bits_per_sample != 4) {
        goto L_error_fields;
    }

    ADPCMreader* reader = malloc_chk(sizeof(ADPCMreader));
    if (!reader) {
        logger_error("sndbridge: adpcmreader_init() out-of-memory");
        return NULL;
    }

    *reader = (ADPCMreader){
        .handle = (DecoderHandle){
            .readBytes = adpcmreader_readBytes,
            .getInfo = adpcmreader_getInfo,
            .seek = adpcmreader_seek,
            .getLoopPoints = adpcmreader_getLoopPoints,
            .destroy = adpcmreader_destroy
        },

        .src_hnd = hnd,
        .data_offset = data_offset,
        .bytes_position = 0,
        .total_bytes = data_length,

        .channels = fmt_info.channels,
        .rate = fmt_info.sample_rate,
        .loop_start = loop_start,
        .loop_length = loop_length
    };

    // seek to data samples
    assert(hnd->seek(hnd, data_offset, SEEK_SET) == 0);

    return (DecoderHandle*)reader;

L_error_fields:
    logger_error("sndbridge: adpcmreader_init() invalid/unsupported WAV");
    return NULL;
}

bool wav_is_file_RIFF_WAVE(SourceHandle* hnd) {
    int64_t offset = hnd->tell(hnd);
    WAV wav;
    bool success;

    if (hnd->read(hnd, &wav, sizeof(WAV)) != sizeof(WAV)) {
        success = false;
    } else if (wav.riff.name != WAV_HDR_RIFF) {
        success = false;
    } else if (wav.riff_type != WAV_RIFF_TYPE_WAVE) {
        success = false;
    } else {
        success = true;
    }

    hnd->seek(hnd, offset, SEEK_SET);
    return success;
}

bool wav_read_header(SourceHandle* hnd, WavFormat* fmt, int64_t* df, int64_t* dl, int64_t* lps, int64_t* lpl) {
    int64_t offset_end;
    int64_t wav_end_offset = hnd->tell(hnd);
    WAV wav;

    if (hnd->read(hnd, &wav, sizeof(WAV)) != sizeof(WAV)) {
        goto L_error_truncated;
    }

    if (wav.riff.name != WAV_HDR_RIFF || wav.riff_type != WAV_RIFF_TYPE_WAVE || wav.fmt.name != WAV_HDR_FMT) {
        logger_error("sndbridge: read_wav_header() invalid WAV file");
        return false;
    }

    if (wav.fmt.chunk_size != sizeof(WavFormat)) {
        // seek the extra blob
        offset_end = hnd->tell(hnd) + (int64_t)(wav.fmt.chunk_size - sizeof(WavFormat));

        if (hnd->seek(hnd, offset_end, SEEK_SET)) {
            goto L_error_truncated;
        }
    }

    wav_end_offset += wav.riff.chunk_size;

    int64_t loop_start = -1, loop_length = -1;

    RIFFChunk chunk;
    while (true) {
        if (hnd->read(hnd, &chunk, sizeof(RIFFChunk)) != sizeof(RIFFChunk)) {
            goto L_error_truncated;
        }

        int64_t offset = hnd->tell(hnd);
        offset_end = offset + chunk.chunk_size;

        if (chunk.name == WAV_HDR_DATA) {
            *df = offset;
            *dl = chunk.chunk_size;
        } else if (chunk.name == WAV_HDR_SMPL) {
            read_wav_spml(hnd, &loop_start, &loop_length);
        } else if (chunk.name == WAV_HDR_CUE) {
            read_wav_cue(hnd, &loop_start, &loop_length);
        }

        if (hnd->seek(hnd, offset_end, SEEK_SET)) {
            goto L_error_truncated;
        }

        if (offset_end >= wav_end_offset) {
            // EOF
            break;
        }
    }

    if (loop_start == -2 || loop_length == -2) {
        logger_error("sndbridge: wav_read_header() invalid loop points or cue/smpl found");
        *lps = *lpl = 0;
    } else if (loop_start < 0 || loop_length < 1) {
        *lps = *lpl = 0;
    } else {
        *lps = loop_start;
        *lpl = loop_length;
    }

    *fmt = wav.format;
    return true;

L_error_truncated:
    logger_error("sndbridge: read_wav_header() truncated WAV file");
    return false;
}

int32_t wav_calc_samples(WavFormat* fmt, int64_t data_length) {
    int64_t samples = data_length * 8;
    samples /= fmt->bits_per_sample;
    samples /= fmt->channels;

    return (int32_t)samples;
}
