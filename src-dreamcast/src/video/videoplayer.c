#include <arch/timer.h>
#include <dc/pvr.h>
#include <dc/sq.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "videoplayer.h"

#include "externals/ffgraph.h"
#include "externals/luascript.h"
#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/sourcehandle.h"
#include "fs.h"
#include "io.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "soundplayer.h"
#include "sprite.h"
#include "vertexprops.h"

#define MACROBLOCK_COUNT(dimmen) ((dimmen / 16) - 1)
#define MACROBLOCK_BYTES ((4 + 4 + (4 * 4)) * 16)

#define VIDEOPLAYER_MAX_BUFFERED_SIZE (3 * 1024 * 1024)     // 3MiB
#define VIDEOPLAYER_MAX_BUFFERED_SIZE_DBL (8 * 1024 * 1024) // 8MiB


struct VideoPlayer_s {
    bool is_muted;
    Stream* sndbridge_stream;
    SourceHandle* media_sourcehandle;
    FFGraph ffgraph;
    DecoderHandle* ffgraph_sndbridge;
    FFGraphInfo info;
    kthread_t* decoder;
    volatile bool decoder_running;
    bool current_buffer_is_front;
    volatile bool video_track_ended;
    volatile bool frame_available;
    bool loop_enabled;
    Sprite sprite;
    Texture texture;
    pvr_ptr_t buffer_front;
    pvr_ptr_t buffer_back;
    float64 last_video_playback_time;
    uint32_t yuv_config;
    uint8_t* frame_plane_y;
    uint64_t* frame_plane_u;
    uint64_t* frame_plane_v;
    size_t yuv_luma_size;
    size_t yuv_chroma_size;
    mutex_t mutex;
};




static void videoplayer_internal_run_decoder_async(VideoPlayer videoplayer);
static void convert_YUV420p_to_YUV422(uint32_t txr, uint16_t tex_width, uint32_t yuv_cfg, uint16_t fra_width, uint16_t fra_height, uint8_t* y_plane, uint64_t* u_plane, uint64_t* v_plane);
static void videoplayer_internal_decode_single_frame(VideoPlayer videoplayer);
static void videoplayer_internal_stop_decoder(VideoPlayer videoplayer);
static inline float64 videoplayer_internal_get_time();
static inline void videoplayer_internal_suspend(VideoPlayer videoplayer);
static void videoplayer_internal_vram_manage(VideoPlayer videoplayer, bool alloc_or_dealloc);


VideoPlayer videoplayer_init(const char* src) {
    char* absolute_path = fs_get_full_path_and_override(src);

    if (!io_resource_exists(absolute_path, true, false)) {
        free_chk(absolute_path);
        return NULL;
    }

    int64_t file_size = io_file_size(absolute_path);
    assert(file_size >= 0);

    char* native_path = io_get_native_path(absolute_path, true, false, true);
    SourceHandle* media_sourcehandle = filehandle_init5(native_path);
    free_chk(absolute_path);
    free_chk(native_path);

    // Initialize FFgraph and SoundBridge
    FFGraph ffgraph = NULL;
    DecoderHandle* ffgraph_sndbridge = NULL;
    Stream* sndbridge_stream = NULL;
    Texture texture = NULL;

    // initialize FFgraph
    ffgraph = ffgraph_init(media_sourcehandle);

    if (!ffgraph) {
        logger_error("videoplayer_init() ffgraph_init() failed for: %s", src);
        goto L_failed;
    }

    // adquire information about audio/video streams
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    if (info.audio_has_stream) {
        // adquire ExternalDecoder of the audio track
        ffgraph_sndbridge = ffgraph_sndbridge_create_helper(ffgraph, true, false);

        // initialize SoundBridge stream
        StreamResult ret = sndbridge_enqueue2(ffgraph_sndbridge, &sndbridge_stream);
        if (ret != STREAMRESULT_Success) {
            logger_error("videoplayer_init() SoundBridge::Enqueue() %i for: %s", ret, src);
            goto L_failed;
        }
    }

    pvr_ptr_t buffer_front = NULL;
    pvr_ptr_t buffer_back = NULL;
    bool frame_available = false;
    uint32_t pow2_width = 0;
    uint32_t pow2_height = 0;
    void* frame_plane_y = NULL;
    void* frame_plane_u = NULL;
    void* frame_plane_v = NULL;
    size_t luma_size = 0;
    size_t chroma_size = 0;

    if (info.video_has_stream) {
        pow2_width = (uint32_t)math2d_poweroftwo_calc(info.video_encoded_width);
        pow2_height = (uint32_t)math2d_poweroftwo_calc(info.video_encoded_height);

        size_t texture_data_size = pow2_width * pow2_height * sizeof(uint16_t);

        buffer_front = pvr_mem_malloc(texture_data_size);
        malloc_assert(buffer_front, uint16_t[]);
        // buffer_back = pvr_mem_malloc(texture_data_size);
        // malloc_assert(buffer_back, uint16_t[]);

        texture = texture_init_from_raw(
            buffer_front, texture_data_size, true,
            (int32_t)pow2_width, (int32_t)pow2_height,
            info.video_original_width, info.video_original_height
        );

        texture->scale_factor_width = (float)info.video_original_width / (float)info.video_encoded_width;
        texture->scale_factor_height = (float)info.video_original_height / (float)info.video_encoded_height;
        texture->format = PVR_TXRFMT_YUV422 | PVR_TXRFMT_NONTWIDDLED | PVR_TXRFMT_VQ_DISABLE | PVR_TXRFMT_NOSTRIDE;

        ffgraph_get_yuv_sizes(ffgraph, &luma_size, &chroma_size);
        luma_size += 32 - (luma_size % 32);
        chroma_size += 32 - (chroma_size % 32);

        size_t yuv_data_size = luma_size + (chroma_size * 2);
        assert(yuv_data_size > 0);

        uint8_t* yuv_data = memalign_chk(32, yuv_data_size);
        malloc_assert(yuv_data, uint8_t[]);

        frame_plane_y = (uint64_t*)yuv_data;
        frame_plane_u = (uint64_t*)(yuv_data + luma_size);
        frame_plane_v = (uint64_t*)(yuv_data + luma_size + chroma_size);
    }

    Sprite sprite = sprite_init(texture);
    // sprite_set_draw_size(sprite, info.video_original_width, info.video_original_height);

    VideoPlayer videoplayer = malloc_chk(sizeof(struct VideoPlayer_s));
    malloc_assert(videoplayer, VideoPlayer);

    *videoplayer = (struct VideoPlayer_s){
        .media_sourcehandle = media_sourcehandle,
        .ffgraph_sndbridge = ffgraph_sndbridge,
        .sprite = sprite,
        .is_muted = false,
        .sndbridge_stream = sndbridge_stream,
        .ffgraph = ffgraph,
        .buffer_front = buffer_front,
        .buffer_back = buffer_back,
        .info = info,
        .texture = texture,
        .decoder = NULL,
        .decoder_running = false,
        .current_buffer_is_front = true,
        .video_track_ended = !info.video_has_stream,
        .frame_available = frame_available,
        .loop_enabled = false,
        .last_video_playback_time = 0.0,
        .yuv_config = (0x00 << 24) | (MACROBLOCK_COUNT(pow2_height) << 8) | MACROBLOCK_COUNT(pow2_width),
        .frame_plane_y = frame_plane_y,
        .frame_plane_u = frame_plane_u,
        .frame_plane_v = frame_plane_v,
        .yuv_luma_size = luma_size,
        .yuv_chroma_size = chroma_size
    };

    assert(mutex_init(&videoplayer->mutex, MUTEX_TYPE_NORMAL) == 0);

#ifdef DEBUG
    logger_warn("videoplayer_init() debug build running, decoding will be extremly slow without optimizations");
#endif

    if (videoplayer->ffgraph) {
        videoplayer_internal_decode_single_frame(videoplayer);
    }

    return videoplayer;

L_failed:
    if (ffgraph) ffgraph_destroy(ffgraph);
    if (ffgraph_sndbridge) ffgraph_sndbridge_destroy_helper(ffgraph_sndbridge);
    if (sndbridge_stream) sndbridge_stream_destroy(sndbridge_stream);

    if (media_sourcehandle) media_sourcehandle->destroy(media_sourcehandle);

    return NULL;
}

void videoplayer_destroy(VideoPlayer* videoplayer_ptr) {
    VideoPlayer videoplayer = *videoplayer_ptr;
    if (!videoplayer) return;

    luascript_drop_shared(videoplayer);

    videoplayer_internal_stop_decoder(videoplayer);

    ffgraph_destroy(videoplayer->ffgraph);
    if (videoplayer->ffgraph_sndbridge) ffgraph_sndbridge_destroy_helper(videoplayer->ffgraph_sndbridge);
    if (videoplayer->sndbridge_stream) sndbridge_stream_destroy(videoplayer->sndbridge_stream);

    videoplayer->media_sourcehandle->destroy(videoplayer->media_sourcehandle);

    sprite_destroy(&videoplayer->sprite);

    if (videoplayer->texture) {
        videoplayer->texture->data_vram = NULL; // obligatory
        texture_destroy(&videoplayer->texture);
    }

    if (videoplayer->buffer_front) pvr_mem_free(videoplayer->buffer_front);
    if (videoplayer->buffer_back) pvr_mem_free(videoplayer->buffer_back);

    mutex_destroy(&videoplayer->mutex);
    free_chk(videoplayer->frame_plane_y);

    free_chk(videoplayer);
    *videoplayer_ptr = NULL;
}

Sprite videoplayer_get_sprite(VideoPlayer videoplayer) {
    return videoplayer->sprite;
}

void videoplayer_replay(VideoPlayer videoplayer) {
    videoplayer_stop(videoplayer);
    videoplayer_play(videoplayer);
}

void videoplayer_play(VideoPlayer videoplayer) {
    videoplayer_internal_run_decoder_async(videoplayer);
}

void videoplayer_pause(VideoPlayer videoplayer) {
    if (videoplayer->sndbridge_stream) {
        sndbridge_stream_pause(videoplayer->sndbridge_stream);
    }
    videoplayer_internal_stop_decoder(videoplayer);
    videoplayer_internal_vram_manage(videoplayer, false);
    videoplayer_internal_suspend(videoplayer);
}

void videoplayer_stop(VideoPlayer videoplayer) {
    if (videoplayer->sndbridge_stream) {
        sndbridge_stream_stop(videoplayer->sndbridge_stream);
    }

    videoplayer_internal_stop_decoder(videoplayer);
    videoplayer_internal_vram_manage(videoplayer, false);

    if (videoplayer->info.video_has_stream) {
        // do not call ffgraph_seek() function, is already called by sndbridge_stream_stop() function
        if (!videoplayer->sndbridge_stream) ffgraph_seek(videoplayer->ffgraph, 0.0);
        videoplayer->last_video_playback_time = 0.0;
        videoplayer->video_track_ended = false;
    }

    videoplayer_internal_suspend(videoplayer);
}

void videoplayer_loop_enable(VideoPlayer videoplayer, bool enable) {
    if (videoplayer->info.video_has_stream)
        videoplayer->loop_enabled = enable;
    else if (videoplayer->sndbridge_stream)
        sndbridge_stream_set_looped(videoplayer->sndbridge_stream, enable);
}

void videoplayer_fade_audio(VideoPlayer videoplayer, bool in_or_out, float duration) {
    if (videoplayer->sndbridge_stream) {
        sndbridge_stream_do_fade(videoplayer->sndbridge_stream, in_or_out, duration / 1000.0f);
    }
}


void videoplayer_set_volume(VideoPlayer videoplayer, float volume) {
    if (videoplayer->sndbridge_stream) {
        sndbridge_stream_set_volume(videoplayer->sndbridge_stream, volume);
    }
}

void videoplayer_set_mute(VideoPlayer videoplayer, bool muted) {
    if (videoplayer->sndbridge_stream) {
        videoplayer->is_muted = muted;
        sndbridge_stream_mute(videoplayer->sndbridge_stream, muted);
    }
}

void videoplayer_seek(VideoPlayer videoplayer, float64 timestamp) {
    bool was_playing = videoplayer_is_playing(videoplayer);

    float64 duration = videoplayer_get_duration(videoplayer);
    if (timestamp > duration) timestamp = duration;

    videoplayer_internal_stop_decoder(videoplayer);

    if (videoplayer->sndbridge_stream) {
        if (videoplayer->info.video_has_stream) {
            // pause audio (if was playing)
            sndbridge_stream_pause(videoplayer->sndbridge_stream);
        }

        //
        // let soundbridge call to ffgraph_seek() function
        //
#if _arch_dreamcast
        sndbridge_stream_seek(videoplayer->sndbridge_stream, (sndbridge_time_t)timestamp);
#else
        sndbridge_stream_seek(videoplayer->sndbridge_stream, timestamp / 1000.0);
#endif
    } else {
        float64 seek_time = ffgraph_seek(videoplayer->ffgraph, timestamp / 1000.0);
        if (seek_time < 0.0) {
            videoplayer->last_video_playback_time = duration;
            videoplayer->video_track_ended = true;
        } else {
            videoplayer->video_track_ended = false;
        }
    }

    if (videoplayer->info.video_has_stream) {
        if (was_playing) {
            videoplayer_internal_run_decoder_async(videoplayer);
            return;
        }

        // get and upload current frame
        videoplayer_internal_decode_single_frame(videoplayer);
    } else {
        videoplayer->last_video_playback_time = timestamp / 1000.0;
    }

    videoplayer_internal_suspend(videoplayer);
}

void videoplayer_set_property(VideoPlayer videoplayer, int32_t property_id, float value) {
    switch (property_id) {
        case MEDIA_PROP_VOLUME:
            videoplayer_set_volume(videoplayer, value);
            break;
        case MEDIA_PROP_SEEK:
            videoplayer_seek(videoplayer, value);
            break;
        case MEDIA_PROP_PLAYBACK:
            switch ((int)value) {
                case PLAYBACK_PLAY:
                    videoplayer_play(videoplayer);
                    break;
                case PLAYBACK_PAUSE:
                    videoplayer_pause(videoplayer);
                    break;
                case PLAYBACK_STOP:
                    videoplayer_stop(videoplayer);
                    break;
                case PLAYBACK_MUTE:
                    videoplayer_set_mute(videoplayer, true);
                    break;
                case PLAYBACK_UNMUTE:
                    videoplayer_set_mute(videoplayer, false);
                    break;
            }
            break;
        default:
            sprite_set_property(videoplayer->sprite, property_id, value);
            break;
    }
}


bool videoplayer_is_muted(VideoPlayer videoplayer) {
    return videoplayer->is_muted;
}

bool videoplayer_is_playing(VideoPlayer videoplayer) {
    if (videoplayer->sndbridge_stream && sndbridge_stream_is_active(videoplayer->sndbridge_stream))
        return true;
    else
        return videoplayer->decoder_running;
}

Fading videoplayer_has_fading_audio(VideoPlayer videoplayer) {
    if (videoplayer->sndbridge_stream) {
        return (Fading)sndbridge_stream_active_fade(videoplayer->sndbridge_stream);
    }
    return FADING_NONE;
}

float64 videoplayer_get_duration(VideoPlayer videoplayer) {
    return videoplayer->info.estimated_duration_seconds * 1000.0;
}

float64 videoplayer_get_position(VideoPlayer videoplayer) {
    if (videoplayer->sndbridge_stream) {
#ifdef _arch_dreamcast
        return (float64)sndbridge_stream_position(videoplayer->sndbridge_stream);
#else
        return sndbridge_stream_position(videoplayer->sndbridge_stream) * 1000.0;
#endif
    } else {
        return videoplayer->last_video_playback_time;
    }
}

bool videoplayer_has_ended(VideoPlayer videoplayer) {
    bool audio_track_ended = true;
    if (videoplayer->sndbridge_stream)
        audio_track_ended = sndbridge_stream_has_ended(videoplayer->sndbridge_stream);

    return videoplayer->video_track_ended && audio_track_ended;
}


bool videoplayer_has_video_track(VideoPlayer videoplayer) {
    return videoplayer->info.video_has_stream;
}

bool videoplayer_has_audio_track(VideoPlayer videoplayer) {
    // WARNING: audio playback can be unavailable
    return videoplayer->info.audio_has_stream;
}

void videoplayer_poll_streams(VideoPlayer videoplayer) {
    // check if time to show the next frame
    if (!videoplayer->frame_available) return;

#ifdef DEBUG
    assert(videoplayer->buffer_back != NULL);
#endif

    // prepare texture swap
    bool current_buffer_is_front = videoplayer->current_buffer_is_front;
    pvr_ptr_t buffer = current_buffer_is_front ? videoplayer->buffer_back : videoplayer->buffer_front;

    mutex_lock(&videoplayer->mutex);
    {
        // do texture update
        convert_YUV420p_to_YUV422(
            (uint32_t)buffer, videoplayer->texture->width, videoplayer->yuv_config,
            videoplayer->info.video_encoded_width, videoplayer->info.video_encoded_height,
            videoplayer->frame_plane_y, videoplayer->frame_plane_u, videoplayer->frame_plane_v
        );
        videoplayer->current_buffer_is_front = !current_buffer_is_front;
        videoplayer->frame_available = false;
        videoplayer->texture->data_vram = buffer;
    }
    mutex_unlock(&videoplayer->mutex);
}


static void* videoplayer_internal_decoder(void* arg) {
    VideoPlayer videoplayer = (VideoPlayer)arg;
    Stream* audio = videoplayer->sndbridge_stream;
    YUVFrame frame;
    bool has_prefeched_frame = false;

L_prepare:
    float64 time_pts = 0.0;
    float64 time_offset = -1.0;

    time_offset += videoplayer->last_video_playback_time;

    while (videoplayer->decoder_running) {

        // if loop is enabled, check if both audio and video ended and play again
        if (videoplayer->video_track_ended) {
            if (!audio || sndbridge_stream_has_ended(audio)) {
                if (!videoplayer->loop_enabled) {
                    videoplayer->decoder_running = false;
                    videoplayer_internal_suspend(videoplayer);
                    break;
                }

                if (audio)
                    sndbridge_stream_stop(audio);
                else
                    ffgraph_seek(videoplayer->ffgraph, 0.0);

                // loop again
                videoplayer->video_track_ended = false;
                goto L_prepare;
            } else {
                thd_pass();
                continue;
            }
        }

        if (!has_prefeched_frame) {
            // prefetch next frame
            time_pts = ffgraph_read_video_frame(videoplayer->ffgraph, &frame);
            has_prefeched_frame = true;

            // do YUV422 conversion and upload to PVR
            if (time_pts >= 0.0) {
                memcpy(videoplayer->frame_plane_y, frame.y, videoplayer->yuv_luma_size);
                memcpy(videoplayer->frame_plane_u, frame.u, videoplayer->yuv_chroma_size);
                memcpy(videoplayer->frame_plane_v, frame.v, videoplayer->yuv_chroma_size);
            }
        }

        if (!videoplayer->decoder_running) {
            break;
        }

        if (time_offset < 0.0) {
            if (audio) sndbridge_stream_play(audio);
            time_offset = videoplayer_internal_get_time();
        }

        float64 current_time = videoplayer_internal_get_time() - time_offset;
        if (current_time < time_pts) {
            thd_pass();
            continue;
        }

        if (time_pts <= -2.0) {
            // video track ended
            videoplayer->video_track_ended = true;
        } else if (time_pts < 0.0) {
            // an error ocurred
            has_prefeched_frame = false;
        } else {
            has_prefeched_frame = false;
            videoplayer->last_video_playback_time = time_pts;
            videoplayer->frame_available = true;
        }
    }

    return NULL;
}

static void videoplayer_internal_run_decoder_async(VideoPlayer videoplayer) {
    if (!videoplayer->info.video_has_stream && videoplayer->sndbridge_stream) {
        sndbridge_stream_play(videoplayer->sndbridge_stream);
        return;
    }

    // (just in case) wait until the decoder thread ends
    videoplayer_internal_stop_decoder(videoplayer);

    // allocate back buffer
    videoplayer_internal_vram_manage(videoplayer, true);

    videoplayer->decoder = thd_create(0, videoplayer_internal_decoder, videoplayer);
    thd_set_prio(videoplayer->decoder, PRIO_DEFAULT - 1);

    videoplayer->frame_available = false;
    videoplayer->decoder_running = true;
    videoplayer->video_track_ended = false;
}

static void videoplayer_internal_stop_decoder(VideoPlayer videoplayer) {
    if (videoplayer->decoder) {
        videoplayer->decoder_running = false;

        if (videoplayer->decoder->state != STATE_FINISHED)
            thd_join(videoplayer->decoder, NULL);
        else
            thd_destroy(videoplayer->decoder);

        videoplayer->decoder = NULL;
    }
}


static inline void copy_macroblock_chrm_plane(uint64_t* dst_block, uint64_t* src_plane, size_t stride, size_t x, size_t y) {
    y *= stride;

    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block = src_plane[y + x];
    y += stride;

    sq_flush(dst_block++);

    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block++ = src_plane[y + x];
    y += stride;
    *dst_block = src_plane[y + x];
    y += stride;

    sq_flush(dst_block++);
}

static void convert_YUV420p_to_YUV422(uint32_t txr, uint16_t tex_width, uint32_t yuv_cfg, uint16_t fra_width, uint16_t fra_height, uint8_t* y_plane, uint64_t* u_plane, uint64_t* v_plane) {
    //
    // code taken from KallistiOS (examples/dreamcast/pvr/yuv_converter/YUV420/yuv420.c)
    // Copyright (C) 2023 Andy Barajas
    // Copyright (C) 2023 Ruslan Rostovtsev
    //

    /* lock now to avoid other videoplayer instances override the YUV config */
    sq_lock((void*)PVR_TA_YUV_CONV);

    /* Set SQ to YUV converter. */
    PVR_SET(PVR_YUV_ADDR, (txr & 0xffffffu));
    /* Divide PVR texture width and texture height by 16 and subtract 1. */
    PVR_SET(PVR_YUV_CFG, yuv_cfg);
    /* Need to read once */
    PVR_GET(PVR_YUV_CFG);


    size_t stride_chrm = ((size_t)fra_width + 15) / 16;
    size_t stride_lmnc = stride_chrm * 16;
    size_t dummies = (MACROBLOCK_BYTES * ((tex_width >> 4) - (stride_lmnc >> 4))) >> 5;
    uint32_t* db = (uint32_t*)SQ_MASK_DEST_ADDR(PVR_TA_YUV_CONV);
    uint64_t* u_block = (uint64_t*)SQ_MASK_DEST_ADDR(PVR_TA_YUV_CONV);
    uint64_t* v_block = (uint64_t*)SQ_MASK_DEST_ADDR(PVR_TA_YUV_CONV + 64);
    uint8_t* y_block = (uint8_t*)SQ_MASK_DEST_ADDR(PVR_TA_YUV_CONV + 128);


    for (size_t y_blk = 0; y_blk < fra_height; y_blk += 16) {
        for (size_t x_blk = 0; x_blk < fra_width; x_blk += 16) {
            size_t x_blk_chrm = x_blk / 16;
            size_t y_blk_chrm = y_blk / 2;

            /* U data for 16x16 pixels */
            copy_macroblock_chrm_plane(u_block, u_plane, stride_chrm, x_blk_chrm, y_blk_chrm);

            /* V data for 16x16 pixels */
            copy_macroblock_chrm_plane(v_block, v_plane, stride_chrm, x_blk_chrm, y_blk_chrm);

            /* Y data for 4 (8x8 pixels), the following loops are extremely difficult to unroll */
            size_t blk_idx = 24;
            for (size_t i = 0; i < 4; ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    size_t index = (y_blk + j + (i / 2 * 8)) * stride_lmnc + x_blk + (i % 2 * 8);
                    *((uint64_t*)&y_block[i * 64 + j * 8]) = *((uint64_t*)&y_plane[index]);
                }
                sq_flush(&y_block[blk_idx]);
                blk_idx += 32;

                for (size_t j = 4; j < 8; ++j) {
                    size_t index = (y_blk + j + (i / 2 * 8)) * stride_lmnc + x_blk + (i % 2 * 8);
                    *((uint64_t*)&y_block[i * 64 + j * 8]) = *((uint64_t*)&y_plane[index]);
                }
                sq_flush(&y_block[blk_idx]);
                blk_idx += 32;
            }
        }

        /* Send dummies if frame texture width doesn't match pvr texture width */
        for (size_t i = 0; i < dummies; ++i) {
            db[i] = db[i + 1] = db[i + 2] = db[i + 3] =
                db[i + 4] = db[i + 5] = db[i + 6] = db[i + 7] = 0;
            sq_flush(&db[i]);
        }
    }

    sq_unlock();
}


static inline float64 videoplayer_internal_get_time() {
    uint32_t secs, nsecs;
    timer_ns_gettime(&secs, &nsecs);
    return (float64)secs + ((float64)nsecs / FLOAT64_LN(1000000000.0));
}

static void videoplayer_internal_decode_single_frame(VideoPlayer videoplayer) {
    YUVFrame frame;
    float64 pts = ffgraph_read_video_frame(videoplayer->ffgraph, &frame);

    if (pts < 0.0) {
        return;
    }

#ifdef DEBUG
    if (!videoplayer->current_buffer_is_front) {
        // this should never happen, if asserts, the decoder thread was never stopped
        assert(videoplayer->buffer_back != NULL);
    }
#endif

    // use the current buffer, can cause sprite tearing briefly after seeking
    pvr_ptr_t target_buffer;
    if (videoplayer->current_buffer_is_front)
        target_buffer = videoplayer->buffer_front;
    else
        target_buffer = videoplayer->buffer_back;

    convert_YUV420p_to_YUV422(
        (uint32_t)target_buffer,
        videoplayer->texture->width,
        videoplayer->yuv_config,
        videoplayer->info.video_encoded_width, videoplayer->info.video_encoded_height,
        frame.y, (uint64_t*)frame.u, (uint64_t*)frame.v
    );

    videoplayer->texture->data_vram = target_buffer;
    videoplayer->last_video_playback_time = pts;
    videoplayer->frame_available = false;
}

static inline void videoplayer_internal_suspend(VideoPlayer videoplayer) {
    videoplayer->media_sourcehandle->suspend(videoplayer->media_sourcehandle);
}

static void videoplayer_internal_vram_manage(VideoPlayer videoplayer, bool alloc_or_dealloc) {
    //
    // (de)allocate back buffer to save PVR memory. Normally
    // each buffer takes 512KiB, in total is 1MiB which is a lot
    // for the nearly ~8MiB free space of the VRAM.
    //

    if (alloc_or_dealloc) {
        if (videoplayer->buffer_back == NULL) {
            videoplayer->buffer_back = pvr_mem_malloc(videoplayer->texture->data_size);
            malloc_assert(videoplayer->buffer_back, uint16_t[]);
        }
    } else {
        if (!videoplayer->current_buffer_is_front) {
            // swap buffers before continue
            pvr_ptr_t front = videoplayer->buffer_front;
            videoplayer->buffer_front = videoplayer->buffer_back;
            videoplayer->buffer_back = front;
            videoplayer->current_buffer_is_front = true;
        }
        if (videoplayer->buffer_back != NULL) {
            pvr_mem_free(videoplayer->buffer_back);
            videoplayer->buffer_back = NULL;
        }
    }
}
