#include <assert.h>
#include <stdio.h>

#include <arch/arch.h>
#include <arch/timer.h>
#include <kos/fs.h>
#include <kos/init.h>
#include <kos/thread.h>

#include "animlist.h"
#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/sndbridge_sfx.h"
#include "float64.h"
#include "fontglyph.h"
#include "fontholder.h"
#include "fs.h"
#include "io.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "number_format_specifiers.h"
#include "preloadcache.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "songplayer.h"
#include "soundplayer.h"
#include "sprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "videoplayer.h"

#include "externals/ffgraph.h"
#include "externals/luascript.h"

#include "game/common/messagebox.h"

#include "externals/xoshiro128.c"


#define MACROBLOCK_COUNT(dimmen) ((dimmen / 16) - 1)
#define MACROBLOCK_BYTES ((4 + 4 + (4 * 4)) * 16)
#define WRITE_INT64(ptr, value) ({    \
    int64_t* __tmp__ = (int64_t*)ptr; \
    *__tmp__ = value;                 \
    __tmp__++;                        \
    ptr = (uint8_t*)__tmp__;          \
    ptr;                              \
})
#define READ_INT64(ptr) ({            \
    int64_t* __tmp__ = (int64_t*)ptr; \
    int64_t __value__ = *__tmp__;     \
    __tmp__++;                        \
    ptr = (uint8_t*)__tmp__;          \
    __value__;                        \
})


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

static inline void convert_YUV420p_to_YUV422(uint32_t txr, uint16_t tex_width, uint32_t yuv_cfg, uint16_t fra_width, uint16_t fra_height, uint8_t* y_plane, uint64_t* u_plane, uint64_t* v_plane) {
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


uint32_t xoshiro128_random_uint(uint32_t min, uint32_t max) {
    uint32_t value = xoshiro128_next_starstar();

#ifdef DEBUG
    assert(min < max);
#endif

    uint32_t diff = max - min;

    return diff ? min + (value % diff) : min;
}

void test_bufferedfilehandles() {
    const char* filename1 = "/cd/assets/weeks/week1/songs/fresh-Inst.ogg";
    // const char* filename2 = "/cd/assets/weeks/week7/funkin/songs/guns/Inst.ogg";

    SourceHandle* hnd1 = filehandle_init5(filename1);
    file_t file1 = fs_open(filename1, O_RDONLY);

    assert(hnd1 != NULL && file1 != FILEHND_INVALID);

    int64_t hnd1_length = hnd1->length(hnd1);
    size_t file1_length = fs_total(file1);
    assert(hnd1_length == file1_length);

    size_t buffer_length = (DBL_MEM != 0) ? (20 * 1024 * 1024) : (8 * 1024 * 1024);
    uint8_t* buffer = malloc(buffer_length); // do not use malloc_chk()
    uint8_t* ptr = buffer;
    ssize_t total_readed = 0;
    size_t buffer_written = 0;
    size_t end_attempts = 25;
    size_t end_attempts_progress = 0;
    size_t id = 0;

    assert(buffer);

    uint32_t rng_state[4];
    memcpy(rng_state, xoshiro128_state, sizeof(rng_state));

    // thd_pass();

    logger_info("running test...");
    for (int i = 0; i < 5000; /*i++*/) {
        int32_t to_read_orig = (int32_t)xoshiro128_random_uint(0, 8192 + 1024);
        unsigned int wait_time = xoshiro128_random_uint(0, 2400);
        unsigned int cmd = xoshiro128_random_uint(0, 50);

        if (cmd >= 10 && cmd <= 15) {
            id++;
            *((int64_t*)ptr) = hnd1->tell(hnd1);
            buffer_written += sizeof(int64_t);
            ptr += sizeof(int64_t);
        } else if (cmd >= 25 && cmd <= 30) {
            id++;
            ssize_t offset = (ssize_t)xoshiro128_random_uint(0, 512 * 1024);
            int whence = (int)xoshiro128_random_uint(SEEK_SET, SEEK_END + 1);
            if (whence >= SEEK_END) {
                whence = SEEK_END;
                offset *= -1;
            }

            hnd1->seek(hnd1, offset, whence);
            *((int64_t*)ptr) = hnd1->tell(hnd1);
            buffer_written += sizeof(int64_t);
            ptr += sizeof(int64_t);
        }

        int32_t to_read = to_read_orig;
        int64_t available = buffer_length - buffer_written;
        if (available < to_read) to_read = available;

        id++;
        int32_t readed = hnd1->read(hnd1, ptr, to_read);
        if (readed == 0) {
            if (to_read_orig == 0) {
                continue;
            }
            if (end_attempts_progress++ < end_attempts) {
                continue;
            }
            logger_info("EOF");
            break;
        } else if (readed < 0) {
            logger_error("ERROR");
            break;
        }

        if (wait_time == 0)
            thd_pass();
        else if (wait_time > 1)
            thd_sleep(wait_time);

        ptr += readed;
        total_readed += readed;
        buffer_written += (size_t)readed;

        if (total_readed >= buffer_length) {
            logger_info("BUFFER IS FULL BEFORE EOF");
            break;
        }
    }
    // assert(total_readed == hnd1_length);

    memcpy(xoshiro128_state, rng_state, sizeof(rng_state));

    logger_info("buffer filled, checking contents");
    uint8_t tmp_buffer[8192 + 2048];
    ptr = buffer;
    total_readed = 0;
    buffer_written = 0;
    end_attempts_progress = 0;
    id = 0;

    for (int i = 0; i < 5000; /*i++*/) {
        int32_t to_read_orig = (int32_t)xoshiro128_random_uint(0, 8192 + 1024);
        /*unsigned int wait_time = */ xoshiro128_random_uint(0, 2400);
        unsigned int cmd = xoshiro128_random_uint(0, 50);

        if (cmd >= 10 && cmd <= 15) {
            id++;
            assert(*((int64_t*)ptr) == fs_tell(file1));
            buffer_written += sizeof(int64_t);
            ptr += sizeof(int64_t);
        } else if (cmd >= 25 && cmd <= 30) {
            id++;
            ssize_t offset = (ssize_t)xoshiro128_random_uint(0, 512 * 1024);
            int whence = (int)xoshiro128_random_uint(SEEK_SET, SEEK_END + 1);
            if (whence >= SEEK_END) {
                whence = SEEK_END;
                offset *= -1;
            }

            fs_seek(file1, offset, whence);
            assert(*((int64_t*)ptr) == fs_tell(file1));
            buffer_written += sizeof(int64_t);
            ptr += sizeof(int64_t);
        }

        int32_t to_read = to_read_orig;
        int64_t available = buffer_length - buffer_written;
        if (available < to_read) to_read = available;

        id++;
        ssize_t readed = fs_read(file1, tmp_buffer, (size_t)to_read);
        if (readed == 0) {
            if (to_read_orig == 0) {
                continue;
            }
            if (end_attempts_progress++ < end_attempts) {
                continue;
            }
            logger_info("EOF");
            break;
        } else if (readed < 0) {
            logger_error("ERROR");
            break;
        }

        assert(memcmp(ptr, tmp_buffer, (size_t)readed) == 0);

        ptr += readed;
        total_readed += readed;
        buffer_written += (size_t)readed;

        if (total_readed >= buffer_length) {
            logger_info("BUFFER IS FULL BEFORE EOF");
            break;
        }
    }
    // assert(total_readed == file1_length);

    logger_info("closing handle...");
    free(buffer);
    hnd1->destroy(hnd1);
    fs_close(file1);

    logger_info("¡success!");
}

void test_hugetexture() {

    logger_info("---------------------------------");
    uint32_t vram = pvr_mem_available() / 1024;
    logger_info("pvr memory stats: " FMT_U4 "/8192 KiB used (" FMT_FLT_DCMLS(.2) "%% available)", 8192 - vram, vram / 81.92f);
    pvr_mem_stats();

    Atlas atl = atlas_init("/assets/weeks/week2/halloween/halloween_bg.xml");
    Texture tex = texture_init("/assets/weeks/week2/halloween/halloween_bg.png");

    assert(atl);
    assert(tex);

    texture_upload_to_pvr(tex);

    Sprite spr = sprite_init(tex);
    sprite_set_draw_location(spr, 150, 150);
    atlas_apply(atl, spr, "halloweem bg lightning strike0020", true);
    sprite_resize_draw_size(spr, 350, -1, NULL, NULL);

    float w, h;
    sprite_get_draw_size(spr, &w, &h);

    logger_info("---------------------------------");
    vram = pvr_mem_available() / 1024;
    logger_info("pvr memory stats: " FMT_U4 "/8192 KiB used (" FMT_FLT_DCMLS(.2) "%% available)", 8192 - vram, vram / 81.92f);
    pvr_mem_stats();

    logger_info("Rendering sprite... %gx%g", w, h);

    uint64_t ts = timer_ms_gettime64() + 10000;
    while (timer_ms_gettime64() < ts) {
        float elapsed = pvrctx_wait_ready();

        pvr_context_draw_solid_color(&pvr_context, (RGBA){0.3f, 0.3f, 0.3f, 1.0f}, 150, 150, w, h);
        sprite_animate(spr, elapsed);
        sprite_draw(spr, &pvr_context);
    }

    atlas_destroy(&atl);
    sprite_destroy_full(&spr);

    logger_info("---------------------------------");
    vram = pvr_mem_available() / 1024;
    logger_info("pvr memory stats: " FMT_U4 "/8192 KiB used (" FMT_FLT_DCMLS(.2) "%% available)", 8192 - vram, vram / 81.92f);
    pvr_mem_stats();

    // mallocstats_chk();
    logger_info("All tests are completed");

    arch_menu();
}

void test_atlas_from_txt() {

    Atlas atlas = atlas_init("/assets/weeks/week6/weeb/weebTrees.txt");
    for (size_t i = 0; i < atlas->size; i++) {
        AtlasEntry entry = atlas->entries[i];

        logger_info(
            "%s " FMT_FLT " " FMT_FLT " " FMT_FLT " " FMT_FLT,
            entry.name, entry.x, entry.y, entry.width, entry.height
        );
    }

    atlas_destroy(&atlas);

    arch_menu();
}

void test_videoplayer() {
    VideoPlayer videoplayer = videoplayer_init("/assets/ughCutscene.kdm");
    assert(videoplayer);

    Sprite sprite = videoplayer_get_sprite(videoplayer);
    sprite_set_draw_location(sprite, 100.0f, 100.0f);
    sprite_set_draw_size(sprite, 480.0f, 270.0f);

    bool play = true;
    while (true) {
        if (play) {
            play = false;
            videoplayer_play(videoplayer);
        } else if (videoplayer_has_ended(videoplayer)) {
            break;
        }

        float elapsed = pvrctx_wait_ready();

        videoplayer_poll_streams(videoplayer);
        sprite_animate(sprite, elapsed);

        pvr_context_draw_solid_color(&pvr_context, (RGBA){1, 1, 1, 1}, 100.0f, 100.0f, 480.0f, 270.0f);
        sprite_draw(sprite, &pvr_context);
    }

    fflush(stdout);

    arch_menu();
}

void test_ffgraph() {
    SourceHandle* hnd = filehandle_init1("/cd/assets/cuis_adventure.kdm", true);
    assert(hnd);

    FFGraph ffgraph = ffgraph_init(hnd);
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    uint32_t pow2_width = (uint32_t)math2d_poweroftwo_calc(info.video_encoded_width);
    uint32_t pow2_height = (uint32_t)math2d_poweroftwo_calc(info.video_encoded_height);
    uint32_t yuv_config = (0x00 << 24) | (MACROBLOCK_COUNT(pow2_height) << 8) | MACROBLOCK_COUNT(pow2_width);

    size_t texture_data_size = pow2_width * pow2_height * sizeof(uint16_t);
    pvr_ptr_t tex_front = pvr_mem_malloc(texture_data_size);
    pvr_ptr_t tex_back = pvr_mem_malloc(texture_data_size);
    Texture texture = texture_init_from_raw(
        tex_front, texture_data_size, true,
        (int32_t)pow2_width, (int32_t)pow2_height,
        info.video_encoded_width, info.video_encoded_height
    );
    texture->format = PVR_TXRFMT_YUV422 | PVR_TXRFMT_NONTWIDDLED | PVR_TXRFMT_VQ_DISABLE | PVR_TXRFMT_NOSTRIDE;


    static uint8_t audio_buffer[64 * 1024];

    float64 pts;
    YUVFrame frame;
    bool is_front = true;
    while ((pts = ffgraph_read_video_frame(ffgraph, &frame)) >= 0.0) {

        pvr_ptr_t tex = is_front ? tex_front : tex_back;
        texture->data_vram = tex;
        is_front = !is_front;

        convert_YUV420p_to_YUV422(
            (uint32_t)tex, pow2_width, yuv_config,
            info.video_encoded_width, info.video_encoded_height,
            frame.y, (uint64_t*)frame.u, (uint64_t*)frame.v
        );

        // STUB
        ffgraph_read_audio_samples(ffgraph, audio_buffer, sizeof(audio_buffer));

        if (pvr_check_ready() != 0) {
            thd_pass();
        }

        pvr_context_draw_texture(
            &pvr_context, texture,
            0, 0, info.video_encoded_width, info.video_encoded_height,
            100, 100, info.video_encoded_width, info.video_encoded_height
        );
        pvrctx_wait_ready();
    }

    ffgraph_destroy(ffgraph);
    hnd->destroy(hnd);

    arch_menu();
}

void test_flt64_time() {
    thd_sleep(400);

    float64 start = videoplayer_internal_get_time();

    for (int i = 0; i < 10; i++) {
        float64 now = videoplayer_internal_get_time();
        logger_info("time is " FMT_FLT64 " seconds", now - start);
        thd_sleep(math2d_random_uint(100, 1000));
    }

    arch_menu();
}

void test_obj_alloc_by_lua() {
    uint8_t fake_context[1024];

    char* lua = fs_readtext("/assets/test.lua");
    Luascript luascript = luascript_init(
        lua, "lua_test.lua", string_duplicate(FS_ASSETS_FOLDER), fake_context, false
    );

    luascript_call_function(luascript, "main");
    luascript_call_function(luascript, "test_destroy");

    free_chk(lua);
    luascript_destroy(&luascript);

    arch_main();
}

void test_fontglyph_by_add() {
    FontGlyph fontglyph = fontglyph_init("/assets/common/font/alphabet.xml", NULL, false);
    assert(fontglyph);

    FontParams params = {
        .border_color = {0.0f, 0.0f, 0.0f, 0.0f},
        .border_enable = false,
        .border_size = 0.0f,
        .color_by_addition = true,
        .height = 64.0f,
        .paragraph_space = 0.0f,
        .tint_color = {1.0f, 1.0f, 1.0f, 1.0f},
        .border_offset_x = 0.0f,
        .border_offset_y = 0.0f,
    };

    const char* text = "abc123";

    while (true) {
        float elapsed = pvrctx_wait_ready();

        pvr_context_draw_solid_color(&pvr_context, (RGBA){0.5f, 0.5f, 0.5f, 1.0f}, 50.0f, 50.0f, 400.0f, 100.0f);

        fontglyph_animate(fontglyph, elapsed);

        fontglyph_draw_text(
            fontglyph, &pvr_context, &params,
            70.0f, 100.0f,
            0, strlen(text), text
        );
    }

    arch_menu();
}

void test_preload_data_integrity_check() {
    const char* absolute_path = "/assets/weeks/week7/funkin/week7/images/tankmanKilled1.png";
    char* native_path = io_get_native_path(absolute_path, true, false, false);

    ArrayBuffer groundtruth_arraybuffer = io_read_arraybuffer(absolute_path);
    preloadcache_clear();

    ArrayBuffer arraybuffer = arraybuffer_init2(native_path, 1 * 1024 * 1024);
    preloadcache_append(native_path, false, false, arraybuffer);
    arraybuffer_destroy((ArrayBuffer[]){arraybuffer});

    assert(groundtruth_arraybuffer->references == 1);
    assert(arraybuffer->references == 1);
    assert(groundtruth_arraybuffer->length == arraybuffer->length);

    for (size_t i = 0; i < groundtruth_arraybuffer->length; i++) {
        if (groundtruth_arraybuffer->data[i] != arraybuffer->data[i]) {
            logger_error("byte missmatch at " FMT_ZU, i);
            assert(false);
        }
    }

    Texture tex = texture_init(absolute_path);
    assert(tex != NULL);

    texture_destroy(&tex);
    arraybuffer_destroy(&groundtruth_arraybuffer);
    arraybuffer_destroy(&arraybuffer);
    preloadcache_clear();

    logger_info("SUCCESS");
    arch_menu();
}


int main_debug(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    test_preload_data_integrity_check();
    test_fontglyph_by_add();
    test_obj_alloc_by_lua();
    test_flt64_time();
    test_ffgraph();
    test_videoplayer();
    test_atlas_from_txt();
    test_hugetexture();

    SoundPlayer soundplayer = soundplayer_init("/assets/weeks/tutorial/tutorial.ogg");
    assert(soundplayer);
    // soundplayer_seek(soundplayer, 9049);
    soundplayer_play(soundplayer);

    while (soundplayer_is_playing(soundplayer)) {
        thd_sleep(1000);
    }
    soundplayer_destroy(&soundplayer);
    arch_menu();

    SongPlayer songplayer = songplayer_init("/assets/weeks/week4/songs/satin-panties.ogg", false);
    // SongPlayer songplayer = songplayer_init("/assets/weeks/week1/songs/fresh.ogg", false);
    // SongPlayer songplayer = songplayer_init("/assets/dadbattle.ogg", false);
    assert(songplayer);

    logger_info("Running SongPlayer test...");
    float64 duration = songplayer_get_duration(songplayer);
    logger_info("Duration: %f milliseconds", (float)duration);

    SongPlayerInfo songplayer_info;
    songplayer_play(songplayer, &songplayer_info);
    logger_info("Playing...");

    uint64_t ts = timer_ms_gettime64() + 1000;
    while (!songplayer_info.completed) {
        songplayer_poll(songplayer, &songplayer_info);

        uint64_t now = timer_ms_gettime64();
        if (now >= ts) {
            logger_info(
                "Playing elapsed %f milliseconds  %f%% completed         ",
                (float)songplayer_info.timestamp,
                100.0f * (float)(songplayer_info.timestamp / duration)
            );
            ts = now + 1000;
        }
        pvr_context_draw_solid_color(&pvr_context, (RGBA){1, 1, 1, 1}, 0, 0, 100, 20);
        pvrctx_wait_ready();
    }
    logger_info("¡Playback completed!");
    arch_menu();
    return 0;

    test_bufferedfilehandles();
    arch_menu();
    return 0;

    const char* test_str = "abc123XYZ456&@\t";
    char* str_lw = string_to_lowercase(test_str);
    char* str_up = string_to_uppercase(test_str);
    bool str_eq = string_equals_ignore_case(str_lw, str_up);
    logger_info("string:   str_lw=%s str_up=%s  str_eq=%s", str_lw, str_up, str_eq ? "true" : "false");
    return 0;

    Atlas atl = atlas_init("/assets/common/image/start-screen/titleEnter.xml");
    Texture tex = texture_init("/assets/common/image/start-screen/titleEnter.png");
    texture_upload_to_pvr(tex);

    Sprite spr = sprite_init(tex);
    sprite_set_draw_location(spr, 150, 150);
    atlas_apply_from_entry(spr, &atl->entries[49], true);
    sprite_resize_draw_size(spr, 350, -1, NULL, NULL);

    float w, h;
    sprite_get_draw_size(spr, &w, &h);

    logger_info("Rendering sprite... %gx%g", w, h);

    /*int idx = 0;
    float t = 0;*/
    while (true) {
        float elapsed = pvrctx_wait_ready();

        /*if (t > 700) {
            if (idx >= atl->size) idx = 0;
            logger_info("atlas_apply_from_entry idx=%i name=\"%s\"", idx, atl->entries[idx].name);
            atlas_apply_from_entry(spr, &atl->entries[idx], false);
            idx++;
            t = 0;
        } else {
            t += elapsed;
        }*/

        pvr_context_draw_solid_color(&pvr_context, (RGBA){0.3f, 0.3f, 0.3f, 1.0f}, 150, 150, w, h);
        sprite_animate(spr, elapsed);
        sprite_draw(spr, &pvr_context);
    }

    logger_info("All tests are completed");

    arch_menu();
    return 0;
}
