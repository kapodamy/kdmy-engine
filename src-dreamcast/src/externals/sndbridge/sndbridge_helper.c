#include <assert.h>
#include <stdio.h>

#include <kos/thread.h>

#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/sndbridge_sfx.h"
#include "externals/sndbridge/wavutil.h"
#include "logger.h"
#include "number_format_specifiers.h"

// maximum amount of samples that can hold the "loopstart" and "loopend" registers
#define AICA_MAX_SAMPLES 0xFFFF

bool sndbridge_can_use_sfx_backend(SourceHandle* hnd) {
#ifndef _arch_dreamcast
    (void)hnd;
    return false;
#endif

    int64_t orig_offset = hnd->tell(hnd);
    bool ret = wav_is_file_RIFF_WAVE(hnd);
    if (!ret) goto L_prepare_return;

    WavFormat wav_fmt;
    int64_t loop_start, loop_length;
    int64_t data_offset, data_length;

    ret = wav_read_header(hnd, &wav_fmt, &data_offset, &data_length, &loop_start, &loop_length);
    if (!ret) goto L_prepare_return;

    int32_t samples = wav_calc_samples(&wav_fmt, data_length);
    ret = samples < AICA_MAX_SAMPLES;
    if (!ret) goto L_prepare_return;

    // check if loop points exists and they cover all samples
    int64_t loop_end = loop_start + loop_length;
    if (loop_start > 0 && loop_length > 0 && (loop_start != 0 || loop_end != samples)) {
        //
        // the AICA lacks of a writable sample "position" register
        // switch back to the standlone sndbridge
        //
        ret = false;
    }

L_prepare_return:
    hnd->seek(hnd, orig_offset, SEEK_SET);
    return ret;
}


#if DEBUG
void sndbridge_main() {
    sndbridge_init();
    // sndbridge_set_master_volume(0.24f);

    const char* info = sndbridge_get_runtime_info();
    logger_info(info);

    Stream* stream;
    StreamResult enqueue_ret;

    SourceHandle* mem = filehandle_init1("/cd/life(cereal)_give_you_lemons.ogg", true);
    // SourceHandle* mem = filehandle_init1("/cd/st_test_adpcm(12@12).wav", false);
    // SourceHandle* mem = filehandle_init1("/cd/st_test_adpcm.wav", false);
    // SourceHandle* mem = filehandle_init1("/cd/the_crystal_method[mono]_adpcm2.wav", true);
    // SourceHandle* mem = filehandle_init1("/cd/the_crystal_method[stereo]_adpcm2.wav", true);

    assert(mem);

    enqueue_ret = sndbridge_enqueue1(mem, &stream);
    // sndbridge_stream_seek(stream, 5000);

    // SourceHandle* mem = filehandle_init1("./life(cereal)_give_you_lemons[vorbis].ogg", true);
    // SourceHandle* mem = filehandle_init1("./life(cereal)_give_you_lemons[opus].ogg", true);
    // enqueue_ret = sndbridge_enqueue1(mem, &stream);

    if (enqueue_ret != STREAMRESULT_Success) return;
    thd_sleep(100);


    /*bool a = true;
    bool b = true;
    bool c = true;
    bool d = true;
    sndbridge_stream_play(stream);
    while (true) {
        float64 duration = sndbridge_stream_duration(stream);
        float64 position = sndbridge_stream_positio(stream);
        logger_log("duration=" FMT_FLT64 "  time=" FMT_FLT64 "", duration, position);
        Pa_Sleep(100);
        // if (i == 6) sndbridge_stream_pause(stream);
        // if (i == 10) sndbridge_stream_play(stream);
        if (position >= (1.000 + duration)) break;
        if (position >= 5.000 && a) {
            a = false;
            sndbridge_stream_seek(stream, 10.000);
        }
        if (position > 3.000 && b) {
            sndbridge_stream_pause(stream);
            Pa_Sleep(1000);

            duration = sndbridge_stream_duration(stream);
            position = sndbridge_stream_positio(stream);
            logger_log("duration=" FMT_FLT64 "  time=" FMT_FLT64 "", duration, position);
            sndbridge_stream_play(stream);
            b = false;
        }
        if (position > 13.000 && c) {
            sndbridge_stream_pause(stream);
            Pa_Sleep(1000);

            duration = sndbridge_stream_duration(stream);
            position = sndbridge_stream_positio(stream);
            logger_log("duration=" FMT_FLT64 "  time=" FMT_FLT64 "", duration, position);
            sndbridge_stream_play(stream);
            c = false;
        }
        if (position > 17.000 && d) {
            sndbridge_stream_stop(stream);
            Pa_Sleep(1000);

            duration = sndbridge_stream_duration(stream);
            position = sndbridge_stream_positio(stream);
            logger_log("duration=" FMT_FLT64 "  time=" FMT_FLT64 "", duration, position);
            sndbridge_stream_play(stream);
            d = false;
        }
    }
    */

    /*while (true) {
       sndbridge_stream_play(stream);
       Pa_Sleep(2000);
       sndbridge_stream_pause(stream);
       Pa_Sleep(2000);
       sndbridge_stream_seek(stream, 0.0);
    }*/

    // sndbridge_stream_set_looped(stream, false);
    // sndbridge_stream_stop(stream);
    sndbridge_stream_play(stream);
    // Pa_Sleep(3000);
    // sndbridge_stream_seek(stream, 90000);

    int32_t total = 0;
    while (1) {
        float64 duration = sndbridge_stream_duration(stream);
        float64 position = sndbridge_stream_position(stream);
        logger_log("duration" FMT_FLT64 "  time" FMT_FLT64 "", duration, position);
        /*
                if (point == 0 && total > 6000) {
                    logger_info("point 0");
                    sndbridge_stream_pause(stream);
                    point++;
                } else if (point == 1 && total > 3000) {
                    logger_info("point 1");
                    sndbridge_stream_seek(stream, 0000);
                    sndbridge_stream_play(stream);
                    point++;
                }
        */
        thd_sleep(100);
        total += 100;

        if (sndbridge_stream_has_ended(stream)) break;
    }

    logger_log("playback done");

    sndbridge_stream_destroy(stream);
}

void sndbridge_sfx_main() {
    sndbridge_init();
    sndbride_sfx_startup();

    sndbridge_set_master_volume(0.23f);

    // note: pcm8 sounds must be u8pcm (unsigned 8bit)
    const char* filenames[] = {
        "/cd/sfx_adpcm_mono.wav",
        "/cd/sfx_adpcm_stereo.wav",

        "/cd/sfx_pcm8_mono.wav",
        "/cd/sfx_pcm8_stereo.wav",

        "/cd/sfx_adpcm_mono_loop.wav",
        "/cd/sfx_adpcm_stereo_loop.wav",

        "/cd/sfx_pcm8_mono_loop.wav",
        "/cd/sfx_pcm8_stereo_loop.wav"
    };

    const char* filename = filenames[7];
    logger_log("\n");
    logger_log("loading %s", filename);

    SourceHandle* hnd = filehandle_init1(filename, true);

    assert(hnd);
    assert(wav_is_file_RIFF_WAVE(hnd));

    WavFormat fmt;
    int64_t lps, lpl;
    int64_t dtoff, dtlen;

    assert(wav_read_header(hnd, &fmt, &dtoff, &dtlen, &lps, &lpl));

    int32_t samples = wav_calc_samples(&fmt, dtlen);
    if (samples > 0xFFFF) {
        logger_log("to many samples to be a sfx. samples=%li maximum=%u", samples, 0xFFFF);
        return;
    }

    hnd->seek(hnd, 0, SEEK_SET);
    StreamSFX* sfx = sndbridge_sfx_init(hnd);

    int32_t sleep_ms = 100;
    int32_t elapsed = 0;
    while (elapsed < 7000) {
        int32_t dur = sndbridge_sfx_duration(sfx);
        int32_t pos = sndbridge_sfx_position(sfx);
        bool act = sndbridge_sfx_is_active(sfx);

        logger_log("duration=%li position=%li active=%s", dur, pos, act ? "true" : "false");

        if (sndbridge_sfx_has_ended(sfx)) {
            logger_log("ended");
            break;
        }

        thd_sleep((unsigned int)sleep_ms);

        if (elapsed == 0) {
            sndbridge_sfx_play(sfx);
        }

        if (elapsed == 5000) {
            sndbridge_sfx_set_looped(sfx, false);
        }

        elapsed += sleep_ms;
    }

    logger_log("playback completed for %s", filename);

    sndbridge_sfx_destroy(sfx);
    sndbride_sfx_shutdown();
    hnd->destroy(hnd);
}

#endif
