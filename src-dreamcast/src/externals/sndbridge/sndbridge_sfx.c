#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch/timer.h>
#include <dc/g2bus.h>
#include <dc/sound/sfxmgr.h>
#include <dc/sound/sound.h>
#include <dc/spu.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "arm/aica_cmd_iface.h"

#include "externals/sndbridge/sndbridge_sfx.h"
#include "externals/sndbridge/wavutil.h"
#include "logger.h"
#include "malloc_utils.h"


struct _StreamSFX {
    bool looped;
    bool muted;
    int vol;
    int pan;

    float fade_duration_ms;
    int32_t fade_elapsed;
    bool fade_out;

    uint32_t channels[2];
    uint32_t paused_at_pos;
    uint64_t end_timestamp;

    uint32_t loop_start;
    uint32_t loop_end;

    uint32_t buffers[2];
    uint32_t sample_length;
    uint32_t sample_rate;
    uint32_t sample_format;
    bool is_stereo;

    volatile bool queueing_enabled;

    struct _StreamSFX* prev;
    struct _StreamSFX* next;
};


static StreamSFX* sfx_list;
static bool master_mute = false;
static float master_volume = 1.0f;
static kthread_t* fade_thd = NULL;
static bool fade_thd_run = false;
static mutex_t fade_thd_mutex;

/*static inline uint32_t aica_clock() {
    return g2_read_32(SPU_RAM_UNCACHED_BASE + AICA_MEM_CLOCK);
}*/

static uint32_t calc_volume_mono(int vol, float porj, bool muted) {
    if (muted || master_mute || vol < 1 || porj <= 0.0f) return 0;

    vol = (int)(vol * porj * master_volume);

    if (vol < 0)
        return 0;
    else if (vol > 255)
        return 255;
    else
        return (uint32_t)vol;
}

static void calc_volume_stereo(int vol, float porj, int pan, bool muted, uint32_t* left_final, uint32_t* right_final) {
    if (muted || master_mute || vol < 1) goto L_silence;

    vol = (int)(vol * porj * master_volume);

    if (vol < 0) goto L_silence;
    if (vol > 255) vol = 255;

    int vol_left = vol, vol_right = vol;

    // calculate virtual panning (if required)
    if (pan != 128) {
        pan -= 128;
        if (pan < 0) vol_right += (pan + 1) * 2;
        if (pan > 0) vol_left -= (pan * 2);
    }

    *left_final = (uint32_t)vol_left;
    *right_final = (uint32_t)vol_right;
    return;

L_silence:
    *left_final = 0;
    *right_final = 0;
}

static void change_volume(StreamSFX* streamsfx, float percent) {
    AICA_CMDSTR_CHANNEL(tmp, cmd, chan);

    uint32_t vol_left = 255, vol_right = 255;
    uint32_t* channels = streamsfx->channels;

    if (streamsfx->is_stereo)
        calc_volume_stereo(streamsfx->vol, percent, streamsfx->pan, streamsfx->muted, &vol_left, &vol_right);
    else
        vol_left = calc_volume_mono(streamsfx->vol, percent, streamsfx->muted);

    cmd->cmd = AICA_CMD_CHAN;
    cmd->timestamp = 0;
    cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
    cmd->cmd_id = channels[0];
    chan->cmd = AICA_CH_CMD_UPDATE | AICA_CH_UPDATE_SET_VOL;
    chan->vol = vol_left;

    snd_sh4_to_aica(tmp, cmd->size);

    if (streamsfx->is_stereo) {
        cmd->cmd_id = channels[1];
        chan->vol = vol_right;

        snd_sh4_to_aica(tmp, cmd->size);
    }
}


static inline bool parse_data(uint32_t* buffers, WavFormat* wav_fmt, uint32_t data_size, void* data, uint32_t* aica_fmt);

static inline uint32_t read_channel_position(uint32_t channel) {
    return g2_read_32(SPU_RAM_BASE + AICA_CHANNEL((uint32_t)channel) + offsetof(aica_channel_t, pos));
}

static inline void play_on_chn(StreamSFX* streamsfx, uint32_t start, uint32_t end) {
    AICA_CMDSTR_CHANNEL(tmp, cmd, chan);

    uint32_t vol_left, vol_right;
    if (streamsfx->is_stereo) {
        calc_volume_stereo(streamsfx->vol, 1.0f, streamsfx->pan, streamsfx->muted, &vol_left, &vol_right);
    } else {
        chan->pan = (uint32_t)streamsfx->pan;
        chan->vol = calc_volume_mono(streamsfx->vol, 1.0f, streamsfx->muted);
    }

    /*bool splitted = streamsfx->looped && start > 0;
    bool second_pass = false;

L_setup_aica_cmdstr:*/
    cmd->cmd = AICA_CMD_CHAN;
    cmd->timestamp = 0;
    cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
    cmd->cmd_id = streamsfx->channels[0];
    chan->cmd = AICA_CH_CMD_START;
    chan->base = streamsfx->buffers[0];
    chan->type = streamsfx->sample_format;
    chan->length = streamsfx->sample_length;
    chan->loop = (uint32_t)streamsfx->looped;
    chan->loopstart = start;
    chan->loopend = end;
    chan->freq = streamsfx->sample_rate;

    /*if (second_pass) {
        uint32_t start_timestamp = (start * 1000) / streamsfx->sample_rate;
        cmd->timestamp = start_timestamp + aica_clock();
    } else if (splitted) {
        chan->loopstart = 0;
        chan->loopend = start;
        chan->loop = (uint32_t) false;
    }*/

    snd_sh4_to_aica_stop();

    if (streamsfx->is_stereo) {
        chan->pan = 0;
        chan->vol = vol_left;
        snd_sh4_to_aica(tmp, cmd->size);

        cmd->cmd_id = streamsfx->channels[1];
        chan->base = streamsfx->buffers[1];
        chan->pan = 255;
        chan->vol = vol_right;
        snd_sh4_to_aica(tmp, cmd->size);

        /*if (splitted) {
            // setup and send looped cmdstr
            second_pass = true;
            splitted = false;
            goto L_setup_aica_cmdstr;
        }*/

    } else {
        snd_sh4_to_aica(tmp, cmd->size);
    }

    if (!streamsfx->queueing_enabled) snd_sh4_to_aica_start();
}

static inline void stop_playback(StreamSFX* streamsfx) {
    snd_sfx_stop((int)streamsfx->channels[0]);
    if (streamsfx->is_stereo) snd_sfx_stop((int)streamsfx->channels[1]);
}


static inline sndbridge_time_t samples_to_time(StreamSFX* streamsfx, uint32_t sample_count) {
#ifdef _arch_dreamcast
    return (int32_t)((sample_count * 1000) / streamsfx->sample_rate);
#else
    return sample_count / (float64)streamsfx->sample_rate;
#endif
}

static inline uint32_t time_to_samples(StreamSFX* streamsfx, sndbridge_time_t time) {
#ifdef _arch_dreamcast
    return ((uint32_t)time * streamsfx->sample_rate) / 1000U;
#else
    return (uint32_t)(time * streamsfx->sample_rate);
#endif
}


static inline void do_fade(StreamSFX* streamsfx, int32_t elapsed) {
    float fade_duration_ms = streamsfx->fade_duration_ms;
    if (fade_duration_ms < 1.0f) return;

    int32_t fade_elapsed = streamsfx->fade_elapsed;
    if (fade_elapsed >= fade_duration_ms) {
        change_volume(streamsfx, streamsfx->fade_out ? 0.0f : 1.0f);
        streamsfx->fade_duration_ms = 0.0f;
        return;
    }

    float percent = fade_elapsed / fade_duration_ms;
    if (streamsfx->fade_out) percent = 1.0f - percent;

    change_volume(streamsfx, percent);

    fade_elapsed += elapsed;
    streamsfx->fade_elapsed = fade_elapsed < fade_duration_ms ? fade_elapsed : (int32_t)fade_duration_ms;
}

static void* fade_thread(void* param) {
    (void)param;

    uint64_t last = 0;

    while (fade_thd_run) {
        int32_t elapsed = (int32_t)(timer_ms_gettime64() - last);

        mutex_lock(&fade_thd_mutex);
        {
            for (StreamSFX* current = sfx_list; current; current = current->next) {
                do_fade(current, elapsed);
            }
            last = timer_ms_gettime64();
        }
        mutex_unlock(&fade_thd_mutex);

        thd_sleep(50);
    }

    return NULL;
}


sndbridge_time_t sndbridge_sfx_duration(StreamSFX* streamsfx) {
    return samples_to_time(streamsfx, streamsfx->sample_length);
}

sndbridge_time_t sndbridge_sfx_position(StreamSFX* streamsfx) {
    uint32_t* channels = streamsfx->channels;

    /*uint64_t now = timer_ms_gettime64();
    if (now >= streamsfx->end_timestamp) return sndbridge_sfx_duration(streamsfx);*/

    if (streamsfx->end_timestamp == 0) return 0;

    uint32_t pos = read_channel_position(channels[0]);

    if (streamsfx->is_stereo) {
        uint32_t pos2 = read_channel_position(channels[1]);
        pos = (pos + pos2) / 2;
    }

    return samples_to_time(streamsfx, pos);
}

void sndbridge_sfx_seek(StreamSFX* streamsfx, sndbridge_time_t position) {
    uint64_t now = timer_ms_gettime64();
    bool is_playing = now < streamsfx->end_timestamp;

    uint32_t pos = time_to_samples(streamsfx, position);
    streamsfx->paused_at_pos = pos;
    streamsfx->fade_duration_ms = 0.0f;

    uint32_t end = streamsfx->looped ? streamsfx->loop_end : streamsfx->sample_length;
    if (pos >= end) {
        streamsfx->end_timestamp = 0;
        if (is_playing) stop_playback(streamsfx);
        return;
    }

    if (is_playing) {
        sndbridge_sfx_play(streamsfx);
    }
}

void sndbridge_sfx_play(StreamSFX* streamsfx) {
    uint64_t now = timer_ms_gettime64();
    uint32_t pos = streamsfx->paused_at_pos;
    bool loop = streamsfx->looped;
    uint64_t end_timestamp = streamsfx->end_timestamp;

    if (end_timestamp != 0 && end_timestamp < now) {
        // still playing
        return;
    }

    uint32_t start, end;
    if (loop && streamsfx->loop_end > 0) {
        start = streamsfx->loop_start;
        end = streamsfx->loop_end;
    } else {
        start = pos;
        end = streamsfx->sample_length;
    }

    if (pos >= end || end_timestamp >= now || loop) {
        // ignore resume position if ended or is looped (looped sfx can not be resumed)
        pos = 0;
    }

    play_on_chn(streamsfx, start, end);

    now = timer_ms_gettime64();
    if (loop) {
        // undefined end timestamp
        streamsfx->end_timestamp = UINT64_MAX;
    } else {
        uint32_t end_ms = ((end - pos) * 1000) / streamsfx->sample_rate;
        streamsfx->end_timestamp = now + end_ms;
    }
}

void sndbridge_sfx_pause(StreamSFX* streamsfx) {
    uint32_t* channels = streamsfx->channels;
    uint32_t pos = read_channel_position(channels[0]);

    if (streamsfx->is_stereo) {
        uint32_t pos2 = read_channel_position(channels[1]);
        pos = (pos + pos2) / 2;
    }

    uint64_t now = timer_ms_gettime64();
    if (now >= streamsfx->end_timestamp) {
        // nothing to pause
        return;
    }

    streamsfx->paused_at_pos = pos;
    streamsfx->end_timestamp = 0;
    streamsfx->fade_duration_ms = 0.0f;

    stop_playback(streamsfx);
}

void sndbridge_sfx_stop(StreamSFX* streamsfx) {
    uint64_t now = timer_ms_gettime64();

    if (now < streamsfx->end_timestamp) {
        stop_playback(streamsfx);
    }

    streamsfx->paused_at_pos = 0;
    streamsfx->end_timestamp = 0;
    streamsfx->fade_duration_ms = 0.0f;
}

void sndbridge_sfx_set_volume(StreamSFX* streamsfx, float volume) {
    int vol = (int)(volume * 255.0f);

    if (vol < 0)
        vol = 0;
    else if (vol > 255)
        vol = 255;

    streamsfx->vol = vol;
    change_volume(streamsfx, 1.0f);
}

void sndbridge_sfx_mute(StreamSFX* streamsfx, bool muted) {
    streamsfx->muted = muted;
    change_volume(streamsfx, 1.0f);
}

bool sndbridge_sfx_is_active(StreamSFX* streamsfx) {
    uint64_t end_timestamp = streamsfx->end_timestamp;

    if (end_timestamp == UINT64_MAX)
        return true;
    else if (end_timestamp == 0)
        return false;
    else
        return timer_ms_gettime64() < end_timestamp;
}

void sndbridge_sfx_do_fade(StreamSFX* streamsfx, bool fade_in_or_out, float seconds) {
    streamsfx->fade_out = !fade_in_or_out;
    streamsfx->fade_elapsed = 0;
    streamsfx->fade_duration_ms = seconds * 1000.0f;
}

StreamFading sndbridge_sfx_active_fade(StreamSFX* streamsfx) {
    if (!sndbridge_sfx_is_active(streamsfx)) return STREAMFADING_NONE;
    if (streamsfx->fade_duration_ms < 1) return STREAMFADING_NONE;

    return streamsfx->fade_out ? STREAMFADING_OUT : STREAMFADING_IN;
}

bool sndbridge_sfx_has_ended(StreamSFX* streamsfx) {
    uint64_t end_timestamp = streamsfx->end_timestamp;

    if (end_timestamp == UINT64_MAX || end_timestamp == 0) {
        return false;
    }

    return timer_ms_gettime64() >= end_timestamp;
}

void sndbridge_sfx_set_looped(StreamSFX* streamsfx, bool enable) {
    if (!streamsfx->looped == !enable) return;
    streamsfx->looped = enable;

    if (!sndbridge_sfx_is_active(streamsfx)) return;

    // the AICA driver can not clear the loop flag because reinitializes the wavetable
    sndbridge_sfx_pause(streamsfx);
    sndbridge_sfx_play(streamsfx);
}

void sndbridge_sfx_set_queueing(StreamSFX* streamsfx, bool enable) {
    streamsfx->queueing_enabled = enable;
}

void sndbridge_sfx_force_resync(StreamSFX* streamsfx) {
    uint64_t now = timer_ms_gettime64();

    if (streamsfx->looped) {
        // nothing to do
        return;
    }

    uint32_t pos = streamsfx->paused_at_pos;
    uint32_t end = streamsfx->sample_length;

    if (pos >= end) {
        // ignore resume position if ended or is looped (looped sfx can not be resumed)
        pos = 0;
    }

    uint32_t end_ms = ((end - pos) * 1000) / streamsfx->sample_rate;
    streamsfx->end_timestamp = now + end_ms;
}

void sndbridge_sfx_destroy(StreamSFX* streamsfx) {
    mutex_lock(&fade_thd_mutex);
    {
        // unlink from the list
        if (sfx_list == streamsfx) sfx_list = streamsfx->next;
        if (streamsfx->prev) streamsfx->prev->next = streamsfx->next;
        if (streamsfx->next) streamsfx->next->prev = streamsfx->prev;

        sndbridge_sfx_stop(streamsfx);

        snd_sfx_chn_free((int)streamsfx->channels[0]);
        snd_mem_free(streamsfx->buffers[0]);

        if (streamsfx->is_stereo) {
            snd_sfx_chn_free((int)streamsfx->channels[1]);
            snd_mem_free(streamsfx->buffers[1]);
        }

        free_chk(streamsfx);
    }
    mutex_unlock(&fade_thd_mutex);
}

StreamSFX* sndbridge_sfx_init(SourceHandle* hnd) {
    WavFormat fmt;
    void* data = NULL;
    int64_t data_offset = 0;
    int64_t data_size = 0;
    uint32_t sound_buffers[2];
    uint32_t aica_sample_format;
    int64_t loopstart, looplength;
    bool ret;

    ret = wav_read_header(hnd, &fmt, &data_offset, &data_size, &loopstart, &looplength);
    if (fmt.bits_per_sample < 1 || fmt.sample_rate < 1 || fmt.channels < 1 || fmt.channels > 2) {
        return NULL;
    }

    int32_t sample_length = wav_calc_samples(&fmt, data_size);
    bool is_stereo = fmt.channels > 1;
    int ch1 = snd_sfx_chn_alloc();
    int ch2 = is_stereo ? snd_sfx_chn_alloc() : -1;

    if (ch1 < 0 || (is_stereo && ch2 < 0)) {
        logger_error("sndbride_sfx_init() failed, all AICA channels are occupied");
        return NULL;
    }

    if (hnd->seek(hnd, data_offset, SEEK_SET)) {
        return false;
    }

    data = malloc_chk((size_t)data_size);
    if (!data) {
        return NULL;
    }

    if (hnd->read(hnd, data, data_size) != data_size) {
        // truncated data
        return false;
    }

    ret = parse_data(sound_buffers, &fmt, data_size, data, &aica_sample_format);
    free_chk(data);
    if (!ret) {
        logger_error("sndbride_sfx_init() failed, not enough memory or unsupported format");
        return NULL;
    }


    StreamSFX* streamsfx = calloc_for_type(StreamSFX);
    if (!streamsfx) {
        logger_error("sndbridge_sfx_init() out-of-memory");
        return NULL;
    }

    if (loopstart < 0 || looplength < 1) {
        loopstart = looplength = 0;
    } else if (looplength > sample_length) {
        looplength = (int32_t)(sample_length - loopstart);
    }

    *streamsfx = (StreamSFX){
        .looped = looplength > 0,
        .muted = false,
        .vol = 255,
        .pan = 128,

        .fade_duration_ms = 0.0f,
        .fade_elapsed = 0,
        .fade_out = false,

        .channels = {(uint32_t)ch1, (uint32_t)ch2},
        .paused_at_pos = 0,
        .end_timestamp = 0,

        .loop_start = (uint32_t)loopstart,
        .loop_end = (uint32_t)(loopstart + looplength),

        .buffers = {sound_buffers[0], sound_buffers[1]},
        .sample_length = (uint32_t)sample_length,
        .sample_rate = fmt.sample_rate,
        .sample_format = aica_sample_format,
        .is_stereo = is_stereo,

        .queueing_enabled = false,

        .prev = NULL,
        .next = NULL,
    };

    // link to the list
    if (sfx_list) {
        StreamSFX* last_item = NULL;
        for (StreamSFX* current = sfx_list; current; current = current->next) {
            last_item = current;
        }

        streamsfx->prev = last_item;
        last_item->next = streamsfx;
    } else {
        sfx_list = streamsfx;
    }

    return streamsfx;
}


void sndbride_sfx_startup() {
    if (fade_thd) return;

    fade_thd_run = true;

    assert(mutex_init(&fade_thd_mutex, MUTEX_TYPE_DEFAULT) == 0);
    assert(fade_thd = thd_create(0, fade_thread, NULL));
}

void sndbride_sfx_shutdown() {
    if (sfx_list) {
        logger_warn("sndbride_sfx_shutdown() there are one or more sfx in use");
    }

    fade_thd_run = false;
    thd_join(fade_thd, NULL);
    mutex_destroy(&fade_thd_mutex);
}


static inline void pcm_u8_to_s8(uint8_t* src, uint32_t data_size) {
    while (data_size > 0) {
        int16_t sample = *src - 128;
        if (sample < INT8_MIN) sample = INT8_MIN;

        int8_t* dst = (int8_t*)src;
        *dst = (int8_t)sample;

        src++;
        data_size--;
    }
}

static inline bool parse_data(uint32_t* buffers, WavFormat* wav_fmt, uint32_t data_size, void* data, uint32_t* aica_fmt) {
    uint32_t fmt = wav_fmt->format;
    uint32_t bits_per_sample = wav_fmt->bits_per_sample;
    uint32_t channels = wav_fmt->channels;
    uint32_t half_data_size = data_size / 2;

    buffers[0] = buffers[1] = 0;

    if (fmt == WAV_PCM && bits_per_sample == 8) {
        //
        // On standard WAV files the PCM 8bit sample format is unsigned, the WAVE_FORMAT_EXTENSIBLE chunk
        // allows other formats like floating-point samples but parsing is not supported
        // another option is allow load "Sun Au" files which allows signed 8bit PCM.
        //
        pcm_u8_to_s8(data, data_size);
    }

    if (channels == 1) {
        if (fmt == WAV_ITU_G723_ADPCM_ANTEX || fmt == WAV_YAMAHA_AICA_ADPCM) {
            *aica_fmt = AICA_SM_ADPCM;
        } else if (fmt == WAV_PCM) {
            if (bits_per_sample == 8) {
                *aica_fmt = AICA_SM_8BIT;
            } else if (bits_per_sample == 16) {
                *aica_fmt = AICA_SM_16BIT;
            } else {
                return false;
            }
        } else {
            return false;
        }

        buffers[0] = snd_mem_malloc(data_size);
        if (!buffers[0]) return false;

        spu_memload_sq(buffers[0], data, (size_t)data_size);
        return true;
    }

    if (fmt == WAV_PCM && bits_per_sample == 16) {
        *aica_fmt = AICA_SM_16BIT;
        buffers[0] = snd_mem_malloc(half_data_size);
        buffers[1] = snd_mem_malloc(half_data_size);

        if (!buffers[0] || !buffers[1]) {
            goto L_stereo_error;
        }

        snd_pcm16_split_sq(data, buffers[0], buffers[1], data_size);
        return true;
    }

    // allocate temporal 32bits aligned buffers
    uint32_t* left_buf = memalign_chk(32, half_data_size);
    uint32_t* right_buf = memalign_chk(32, half_data_size);

    if (!left_buf || !right_buf) {
        free_chk(left_buf);
        free_chk(right_buf);
        return false;
    }

    if (fmt == WAV_PCM && bits_per_sample == 8) {
        snd_pcm8_split(data, left_buf, right_buf, data_size);
        *aica_fmt = AICA_SM_8BIT;
    } else if (fmt == WAV_ITU_G723_ADPCM_ANTEX) {
        memcpy(left_buf, data, half_data_size);
        memcpy(right_buf, data + half_data_size, half_data_size);
        *aica_fmt = AICA_SM_ADPCM;
    } else if (fmt == WAV_YAMAHA_AICA_ADPCM) {
        snd_adpcm_split(data, left_buf, right_buf, data_size);
        *aica_fmt = AICA_SM_ADPCM;
    } else {
        // unknown format and bits per sample
        goto L_stereo_error;
    }

    buffers[0] = snd_mem_malloc(half_data_size);
    buffers[1] = snd_mem_malloc(half_data_size);

    if (!buffers[0] || !buffers[1]) {
        goto L_stereo_error;
    }

    spu_memload_sq(buffers[0], left_buf, (size_t)half_data_size);
    spu_memload_sq(buffers[1], right_buf, (size_t)half_data_size);

    free_chk(left_buf);
    free_chk(right_buf);
    return true;

L_stereo_error:
    if (buffers[0]) snd_mem_free(buffers[0]);
    if (buffers[1]) snd_mem_free(buffers[1]);

    return false;
}
