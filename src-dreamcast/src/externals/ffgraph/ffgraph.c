#include <math.h>
#include <string.h>

#include <kos/mutex.h>

#include "pl_ffmpeg/pl_mpeg.h"
#include "pl_ffmpeg/pl_mpeg_buffer.c"
#include "pl_ffmpeg/pl_mpeg_video.c"

#include "externals/ffgraph.h"
#include "float64.h"
#include "kdm.h"
#include "logger.h"
#include "malloc_utils.h"
#include "number_format_specifiers.h"

#define VIDEO_BUFFER_SIZE (512 * 1024)
#define CUE_BUFFER_ENTRIES 1024

struct FFGraph_s {
    KDMFileHeader header;
    SourceHandle* media_sourcehandle;
    volatile bool eof_reached;
    volatile bool buffer_underrun;
    mutex_t mutex;
    KDMPacketHeader packet;
    plm_frame_t* last_frame;

    plm_buffer_t* video_buffer;
    plm_video_t* video_decoder;

    size_t audio_buffer_used;
    uint8_t audio_buffer[64 * 1024]; // 64KiB
};


static const uint8_t FFGRAPH_EXPECTED_KDM_VERSION = 3;
static char* FFGRAPH_INFO = "KDMv3 decoder";


static bool ffgraph_internal_parse_next_packet(FFGraph ffgraph);
static void ffgraph_internal_refill_video_buffer(plm_buffer_t* self, void* user);
static float64 ffgraph_internal_seek_file(FFGraph ffgraph, float64 time);


FFGraph ffgraph_init(const SourceHandle* media_sourcehandle) {
    assert(media_sourcehandle);

    FFGraph ffgraph = calloc_for_type(struct FFGraph_s);
    malloc_assert(ffgraph, FFGraph);

    ffgraph->media_sourcehandle = (SourceHandle*)media_sourcehandle;

    mutex_init(&ffgraph->mutex, MUTEX_TYPE_NORMAL);

    KDMFileHeader* header = &ffgraph->header;

    if (media_sourcehandle->read((SourceHandle*)media_sourcehandle, header, sizeof(KDMFileHeader)) != sizeof(KDMFileHeader)) {
        goto L_failed;
    }

    if (header->signature != KDM_SIGNATURE) {
        logger_error("ffgraph_init() invalid KDM signature");
        goto L_failed;
    }

    if (header->version != FFGRAPH_EXPECTED_KDM_VERSION) {
        logger_error(
            "ffgraph_init() unsupporteed KDM version. expected=%i found=%i\n",
            (int)FFGRAPH_EXPECTED_KDM_VERSION, (int)header->version
        );
        goto L_failed;
    }

    if (header->cue_table_length > 0) {
        // skip cue table
        int32_t table_size = header->cue_table_length * (int32_t)sizeof(KDMCue);
        if (media_sourcehandle->seek((SourceHandle*)media_sourcehandle, table_size, SEEK_CUR)) {
            goto L_failed;
        }
    }

    if (KDM__HAS_FLAG(header->flags, KDM_FLAGS_HEADER_HAS_VIDEO)) {
        assert(header->video_encoded_width <= 1024);
        assert(header->video_encoded_height <= 1024);
        assert(header->video_original_width >= header->video_encoded_width);
        assert(header->video_original_height >= header->video_encoded_height);
        assert(header->video_fps > 0.0f && header->video_fps <= 60.0f);

        ffgraph->video_buffer = plm_buffer_create_with_capacity(512 * 1024); // 512KiB
        plm_buffer_set_load_callback(ffgraph->video_buffer, ffgraph_internal_refill_video_buffer, ffgraph);

        ffgraph->video_decoder = plm_video_create_with_buffer(ffgraph->video_buffer, FALSE);
    }

    if (KDM__HAS_FLAG(header->flags, KDM_FLAGS_HEADER_HAS_AUDIO)) {
        //
        // nothing to do, the audio samples are decoded in the AICA SPU
        //
    }

#ifdef DEBUG
    logger_info("KDM loaded:");
    printf(
        "  capabilities: %s%s%s%s%s\n",
        (header->flags & KDM_FLAGS_HEADER_HAS_VIDEO) ? "HAS_VIDEO " : "",
        (header->flags & KDM_FLAGS_HEADER_HAS_AUDIO) ? "HAS_AUDIO " : "",
        (header->flags & KDM_FLAGS_HEADER_AUDIO_STEREO) ? "AUDIO_STEREO " : "",
        (header->flags & KDM_FLAGS_HEADER_RESERVED1) ? "RESERVED1 " : "",
        (header->flags & KDM_FLAGS_HEADER_RESERVED2) ? "RESERVED2 " : ""
    );
    printf(
        "  metadata: v%i, " FMT_FLT64 " seconds, %i keyframes in the cue table\n",
        header->version, header->estimated_duration_in_milliseconds / (float64)1000.0,
        header->cue_table_length
    );
    printf(
        "  audio stream: ADPCM %iHz\n",
        header->audio_frequency
    );
    printf(
        "  video stream:  %ix%i (original %ix%i) %ffps\n",
        header->video_encoded_width, header->video_encoded_height,
        header->video_original_width, header->video_original_height,
        header->video_fps
    );
#endif

    return ffgraph;

L_failed:
    ffgraph_destroy(ffgraph);
    return NULL;
}

void ffgraph_destroy(FFGraph ffgraph) {
    //
    // note: do not dispose the media_sourcehandle
    //

    if (ffgraph->video_decoder) plm_video_destroy(ffgraph->video_decoder);
    if (ffgraph->video_buffer) plm_buffer_destroy(ffgraph->video_buffer);
    mutex_destroy(&ffgraph->mutex);
    free_chk(ffgraph);
}


void ffgraph_get_streams_info(FFGraph ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    memset(output_info, 0x00, sizeof(FFGraphInfo));

    output_info->estimated_duration_seconds = ffgraph->header.estimated_duration_in_milliseconds / 1000.0;

    output_info->audio_has_stream = KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_AUDIO);
    if (output_info->audio_has_stream) {
        output_info->audio_channels = KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_AUDIO_STEREO) ? 2 : 1;
        output_info->audio_sample_rate = (uint16_t)ffgraph->header.audio_frequency;
    }

    output_info->video_has_stream = KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_VIDEO);
    if (output_info->video_has_stream) {
        output_info->video_encoded_width = (uint16_t)ffgraph->header.video_encoded_width;
        output_info->video_encoded_height = (uint16_t)ffgraph->header.video_encoded_height;
        output_info->video_original_width = (uint16_t)ffgraph->header.video_original_width;
        output_info->video_original_height = (uint16_t)ffgraph->header.video_original_height;
    }
}

int32_t ffgraph_read_audio_samples(FFGraph ffgraph, void* out_samples, int32_t buffer_size) {
    if (!KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_AUDIO)) {
        return -1;
    }
    if (buffer_size < 1) {
        return 0;
    }

#ifdef DEBUG
    assert(out_samples);
#endif

    mutex_lock(&ffgraph->mutex);

    if (ffgraph->audio_buffer_used < 1) {
        if (ffgraph->eof_reached) {
            buffer_size = -1;
            goto L_return;
        }

        if (!ffgraph_internal_parse_next_packet(ffgraph)) {
            buffer_size = -1;
            goto L_return;
        }

        // (edge-case) if the packet does not have audio data, try again
        while (ffgraph->audio_buffer_used < 1) {
            if (ffgraph->buffer_underrun) break;
            if (ffgraph->eof_reached) break;

            if (!ffgraph_internal_parse_next_packet(ffgraph)) {
                buffer_size = -1;
                goto L_return;
            }
        }
    }

    size_t bytes = (size_t)buffer_size;

    if (ffgraph->audio_buffer_used < 1 && ffgraph->buffer_underrun) {
        //
        // playback underrun, this probably due:
        //          * no more audio samples.
        //          * the video decoder is taking to long.
        //          * slow I/O reading.
        //          * video specs are to high (for example bitrate).
        //
        memset(out_samples, 0x00, bytes);
        goto L_return;
    }

    if (bytes > ffgraph->audio_buffer_used) bytes = ffgraph->audio_buffer_used;

    memcpy(out_samples, ffgraph->audio_buffer, bytes);
    ffgraph->audio_buffer_used -= bytes;

    if (ffgraph->audio_buffer_used > 0) {
        memmove(ffgraph->audio_buffer, ffgraph->audio_buffer + bytes, ffgraph->audio_buffer_used);
    }

    buffer_size = (int32_t)bytes;

L_return:
    mutex_unlock(&ffgraph->mutex);
    return buffer_size;
}

float64 ffgraph_read_video_frame(FFGraph ffgraph, YUVFrame* out_frame) {
    if (!KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_VIDEO)) {
        return -2.0;
    }

#ifdef DEBUG
    assert(out_frame);
#endif

    mutex_lock(&ffgraph->mutex);
    float64 time;

    if (ffgraph->eof_reached && plm_buffer_get_size(ffgraph->video_buffer) < 1) {
        time = -2.0;
        goto L_return;
    }

    plm_frame_t* frame = plm_video_decode(ffgraph->video_decoder);

    if (frame) {
        ffgraph->last_frame = frame;
        time = frame->time;
    } else if (ffgraph->buffer_underrun && ffgraph->last_frame) {
        frame = ffgraph->last_frame;
        time = -1.0; // this means duplicated frame due buffer underrun
    } else {
        // note: eof_reached can mean an I/O error, also ends audio reading
        if (ffgraph->eof_reached || plm_video_has_ended(ffgraph->video_decoder))
            time = -2.0; // no more frames
        else
            time = -1.0; // this never should happen

        goto L_return;
    }

    *out_frame = (YUVFrame){
        .y = frame->y.data,
        .u = frame->cb.data,
        .v = frame->cr.data
    };

L_return:
    mutex_unlock(&ffgraph->mutex);
    return time;
}

float64 ffgraph_seek(FFGraph ffgraph, float64 time_in_seconds) {
    plm_frame_t* frame = NULL;
    const bool has_video = KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_VIDEO);
    const bool has_audio = KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_HAS_AUDIO);

    ffgraph->buffer_underrun = false;

    // step 1: seek file
    float64 cue_time = ffgraph_internal_seek_file(ffgraph, time_in_seconds);
    if (ffgraph->eof_reached) {
        // I/O error
        return -1.0;
    }

    // step 2: clear buffers
    if (has_audio) {
        ffgraph->audio_buffer_used = 0;
    }
    if (has_video) {
        ffgraph->last_frame = NULL;
        plm_video_rewind(ffgraph->video_decoder);
        plm_video_set_time(ffgraph->video_decoder, cue_time);
    }

    // step 3: read stream data
    size_t samples_bytes = 0;
    int32_t frames_count = 0;

    if (has_video) {
        frame = plm_video_decode(ffgraph->video_decoder);
        if (!frame && ffgraph->eof_reached) {
            // I/O error
            return -1.0;
        }
        float64 frame_time = frame ? frame->time : cue_time;
        float64 remaining = cue_time - frame_time;
        frames_count = (int32_t)(remaining / plm_video_get_framerate(ffgraph->video_decoder));

        // avoid decoding the desired frame
        frames_count--;
    } else if (!ffgraph_internal_parse_next_packet(ffgraph)) {
        // failed read audio data or eof reached
        return -1.0;
    }

    if (has_audio) {
        if (time_in_seconds > cue_time) {
            samples_bytes = (size_t)((time_in_seconds - cue_time) * ffgraph->header.audio_frequency);
        }
        if (!KDM__HAS_FLAG(ffgraph->header.flags, KDM_FLAGS_HEADER_AUDIO_STEREO)) {
            samples_bytes /= 2;
        }
    }

    // keep parsing the media file until the required timestamp is reached
    if (has_video) {
        if (frames_count > 0) {
            while (!ffgraph->eof_reached) {
                frame = plm_video_decode(ffgraph->video_decoder);

                frames_count--;
                if (frames_count < 1) {
                    break;
                }
                if (samples_bytes >= ffgraph->audio_buffer_used) {
                    samples_bytes -= ffgraph->audio_buffer_used;
                    ffgraph->audio_buffer_used = 0;
                }
            }
        }
    } else {
        while (samples_bytes > ffgraph->audio_buffer_used) {
            samples_bytes -= ffgraph->audio_buffer_used;
            ffgraph->audio_buffer_used = 0;

            if (!ffgraph_internal_parse_next_packet(ffgraph)) {
                // failed read audio data or eof reached
                cue_time = -1.0;
                break;
            }
        }
    }

    // seek audio buffer
    if (samples_bytes < ffgraph->audio_buffer_used) {
        uint8_t* audio_buffer_ptr = (uint8_t*)ffgraph->audio_buffer + samples_bytes;
        memmove(ffgraph->audio_buffer, audio_buffer_ptr, samples_bytes);
        ffgraph->audio_buffer_used -= samples_bytes;
    } else {
        ffgraph->audio_buffer_used = 0;
    }

    return frame ? frame->time : cue_time;
}

const char* ffgraph_get_runtime_info() {
    return FFGRAPH_INFO;
}

void ffgraph_get_yuv_sizes(FFGraph ffgraph, size_t* luma_size, size_t* chroma_size) {
    if (ffgraph->video_decoder) {
        plm_video_get_planes_byte_size(ffgraph->video_decoder, luma_size, chroma_size);
    } else {
        *luma_size = *chroma_size = 0;
    }
}


static bool ffgraph_internal_parse_next_packet(FFGraph ffgraph) {
    KDMPacketHeader* packet = &ffgraph->packet;
    SourceHandle* media = ffgraph->media_sourcehandle;

    if (!ffgraph->buffer_underrun) {
        if (ffgraph->eof_reached || media->read(media, packet, sizeof(KDMPacketHeader)) != sizeof(KDMPacketHeader)) {
            if (ffgraph->video_buffer) plm_buffer_signal_end(ffgraph->video_buffer);
            goto L_return_false;
        }
    }

    //
    // before start reading the packet contents check if there enough space
    // in the audio and video buffers. Otherwise return "true" and wait
    // a call to one or both ffgraph_read_audio_samples() or ffgraph_read_video_frame()
    // functions to deplete buffers.
    //
    size_t audio_available = sizeof(ffgraph->audio_buffer) - ffgraph->audio_buffer_used;
    size_t video_available = plm_buffer_get_available_space(ffgraph->video_buffer);

    if (audio_available < packet->audio_data_size && audio_available < sizeof(ffgraph->audio_buffer)) {
        // play "silence" until there are available space in the audio buffer
        goto L_underrun;
    }
    if (video_available < packet->video_data_size) {
        // note: the video buffer capacity can grow in size but is useless in the current platform
        goto L_underrun;
    }


    if (packet->audio_data_size > 0) {
        if (packet->audio_data_size > sizeof(ffgraph->audio_buffer)) {
            logger_error(
                "ffgraph_internal_parse_next_packet() invalid 'packet.audio_data_size'. maximum=" FMT_ZU " found=" FMT_I4,
                sizeof(ffgraph->audio_buffer), packet->audio_data_size
            );
            goto L_return_false;
        }

        uint8_t* audio_buffer_ptr = &ffgraph->audio_buffer[ffgraph->audio_buffer_used];
        if (media->read(media, audio_buffer_ptr, packet->audio_data_size) != packet->audio_data_size) {
            goto L_failed_truncated;
        }

        ffgraph->audio_buffer_used += (size_t)packet->audio_data_size;
    }

    if (packet->video_data_size > 0) {
        uint8_t buffer[16384]; // 16KiB
        const int32_t buffer_size = (int32_t)sizeof(buffer);

        int32_t data_size = (int32_t)packet->video_data_size;

        while (data_size > 0) {
            int32_t to_read = data_size < buffer_size ? data_size : buffer_size;
            int32_t readed = media->read(media, buffer, to_read);

            if (readed < 1) {
                // stop parsing the file in case of I/O error or EOF
                ffgraph->eof_reached = to_read > 0;
                break;
            }

            plm_buffer_write(ffgraph->video_buffer, buffer, (size_t)readed);
            data_size -= readed;
        }
    }

    ffgraph->buffer_underrun = false;
    return true;

L_failed_truncated:
    logger_error("ffgraph_internal_parse_next_packet() failed, file truncated at " FMT_I8, media->length(media));
    goto L_return_false;

L_underrun:
    ffgraph->buffer_underrun = true;
    return true;

L_return_false:
    ffgraph->eof_reached = true;
    return false;
}

static void ffgraph_internal_refill_video_buffer(plm_buffer_t* self, void* user) {
    PLM_UNUSED(self);

    ffgraph_internal_parse_next_packet((FFGraph)user);
}

static float64 ffgraph_internal_seek_file(FFGraph ffgraph, float64 time) {
    KDMCue cue_buffer[CUE_BUFFER_ENTRIES];

    SourceHandle* media = ffgraph->media_sourcehandle;
    uint16_t cue_entries = ffgraph->header.cue_table_length;

    // time zero cue entry (not present in the kdm file)
    KDMCue cue = (KDMCue){
        .offset = sizeof(KDMFileHeader) + (cue_entries * sizeof(KDMCue)),
        .timestamp = 0
    };

    ffgraph->eof_reached = false;
    if (media->seek(media, (int64_t)sizeof(KDMFileHeader), SEEK_SET)) {
        goto L_IO_error;
    }

    // find the nearest down timestamp entry in the cue table
    int64_t time_microseconds = (int64_t)(time * 1000.0 * 1000.0);
    while (cue_entries > 0) {
        uint16_t entries_to_read = cue_entries < CUE_BUFFER_ENTRIES ? cue_entries : CUE_BUFFER_ENTRIES;
        int32_t entries_bytes = (int32_t)(entries_to_read * sizeof(KDMCue));
        if (media->read(media, cue_buffer, entries_bytes) != entries_bytes) {
            goto L_IO_error;
        }

        for (uint16_t i = 0; i < entries_to_read; i++) {
            if (cue_buffer[i].offset < sizeof(KDMFileHeader)) {
                // invalid cue
                continue;
            }
            if (cue_buffer[i].timestamp == time_microseconds) {
                cue = cue_buffer[i];
                goto L_check_cue;
            } else if (cue_buffer[i].timestamp > time_microseconds) {
                goto L_check_cue;
            }
        }

        cue_entries -= entries_to_read;
        cue = cue_buffer[entries_to_read - 1];
    }

L_check_cue:
    if (cue.offset < sizeof(KDMFileHeader) || cue.offset >= media->length(media)) {
        // cue offset is out of bounds
        goto L_IO_error;
    }

    // jump to the nearest cue
    if (media->seek(media, (int64_t)cue.offset, SEEK_SET)) {
        goto L_IO_error;
    }

    ffgraph->eof_reached = false;
    return (float64)cue.timestamp / (1000.0 * 1000.0);

L_IO_error:
    // I/O error
    ffgraph->eof_reached = true;
    return ffgraph->header.estimated_duration_in_milliseconds / 1000.0;
}

