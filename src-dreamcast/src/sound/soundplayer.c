#include "soundplayer.h"

#include <arch/arch.h>

#include "externals/luascript.h"
#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/sndbridge_sfx.h"
#include "fs.h"
#include "io.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"
#include "vertexprops.h"


#define MAX_FILESIZE_VIA_PRELOADCACHE (512 * 1024) // 512KiB


typedef sndbridge_time_t (*CallbackDuration)(void* instance);
typedef sndbridge_time_t (*CallbackPosition)(void* instance);
typedef void (*CallbackSeek)(void* instance, sndbridge_time_t position);
typedef void (*CallbackPlay)(void* instance);
typedef void (*CallbackPause)(void* instance);
typedef void (*CallbackStop)(void* instance);
typedef void (*CallbackSetVolume)(void* instance, float volume);
typedef void (*CallbackMute)(void* instance, bool muted);
typedef bool (*CallbackIsActive)(void* instance);
typedef void (*CallbackDoFade)(void* instance, bool fade_in_or_out, float seconds);
typedef StreamFading (*CallbackActiveFade)(void* instance);
typedef bool (*CallbackHasEnded)(void* instance);
typedef void (*CallbackSetLooped)(void* instance, bool enable);
typedef void (*CallbackSetQueueing)(void* instance, bool enable);
typedef void (*CallbackForceResync)(void* instance);
typedef void (*CallbackDestroy)(void* instance);

typedef struct {
    CallbackDuration duration;
    CallbackPosition position;

    CallbackSeek seek;
    CallbackPlay play;
    CallbackPause pause;
    CallbackStop stop;
    CallbackSetVolume set_volume;
    CallbackMute mute;
    CallbackIsActive is_active;
    CallbackDoFade do_fade;
    CallbackActiveFade active_fade;
    CallbackHasEnded has_ended;
    CallbackSetLooped set_looped;
    CallbackSetQueueing set_queueing;
    CallbackForceResync force_resync;

    CallbackDestroy destroy;
} SoundBridgeFunctions;

struct SoundPlayer_s {
    void* instance;
    SourceHandle* sourcehandle;
    bool is_muted;
    SoundBridgeFunctions fns;
};


SoundPlayer soundplayer_init(const char* src) {
    if (!fs_file_exists(src)) {
        logger_error("songplayer_init() file not found: %s", src);
        return NULL;
    }

    if (
#ifdef _arch_dreamcast
        !string_lowercase_ends_with(src, ".wav") &&
#endif
        !string_lowercase_ends_with(src, ".ogg") &&
        !string_lowercase_ends_with(src, ".logg")
    ) {
        logger_error("songplayer_init() unknown filename extension: %s", src);
        return NULL;
    }

    char* absolute_path = fs_get_full_path_and_override(src);

    char* native_path = io_get_native_path(absolute_path, true, false, true);
    bool is_wav_file = string_lowercase_ends_with(native_path, ".wav");

    int64_t file_size = io_file_size(absolute_path);

#ifdef DEBUG
    // this never should happen
    assert(file_size > 0);
#endif

    if (DBL_MEM_CHECK != 0x00) file_size /= 4;

    ArrayBuffer arraybuffer = NULL;
    if (is_wav_file && file_size <= MAX_FILESIZE_VIA_PRELOADCACHE) {
        // WAV files are always ~64KiB in size, load via preload cache if possible
        arraybuffer = io_read_arraybuffer(absolute_path);
        if (!arraybuffer) {
            goto L_failed;
        }
    } else {
        // load OGG files via preload cache if they are not larger than 768KiB
        if (file_size <= MAX_FILESIZE_VIA_PRELOADCACHE) {
            arraybuffer = io_read_arraybuffer(absolute_path);
            if (!arraybuffer) {
                goto L_failed;
            }
        }
    }

    free_chk(absolute_path);

    SourceHandle* sourcehandle;
    if (arraybuffer)
        sourcehandle = filehandle_init4(arraybuffer, true);
    else
        sourcehandle = filehandle_init5(native_path);

    free_chk(native_path);

    if (sourcehandle == NULL) {
        logger_error("soundplayer_init() failed for: %s", src);
        return NULL;
    }

    SoundBridgeFunctions fns;
    void* instance;

    if (sndbridge_can_use_sfx_backend(sourcehandle)) {
        StreamSFX* stream = sndbridge_sfx_init(sourcehandle);
        if (!stream) {
            sourcehandle->destroy(sourcehandle);
            logger_error("soundplayer_init() call to sndbridge_sfx_init() failed for: %s", src);
            return NULL;
        }

        fns.active_fade = (CallbackActiveFade)sndbridge_sfx_active_fade;
        fns.destroy = (CallbackDestroy)sndbridge_sfx_destroy;
        fns.do_fade = (CallbackDoFade)sndbridge_sfx_do_fade;
        fns.duration = (CallbackDuration)sndbridge_sfx_duration;
        fns.has_ended = (CallbackHasEnded)sndbridge_sfx_has_ended;
        fns.set_queueing = (CallbackSetQueueing)sndbridge_sfx_set_queueing;
        fns.force_resync = (CallbackForceResync)sndbridge_sfx_force_resync;
        fns.is_active = (CallbackIsActive)sndbridge_sfx_is_active;
        fns.mute = (CallbackMute)sndbridge_sfx_mute;
        fns.pause = (CallbackPause)sndbridge_sfx_pause;
        fns.play = (CallbackPlay)sndbridge_sfx_play;
        fns.position = (CallbackPosition)sndbridge_sfx_position;
        fns.seek = (CallbackSeek)sndbridge_sfx_seek;
        fns.set_looped = (CallbackSetLooped)sndbridge_sfx_set_looped;
        fns.set_volume = (CallbackSetVolume)sndbridge_sfx_set_volume;
        fns.stop = (CallbackStop)sndbridge_sfx_stop;

        instance = stream;
    } else {
        Stream* stream;
        StreamResult res = sndbridge_enqueue1(sourcehandle, &stream);
        if (res != STREAMRESULT_Success) {
            sourcehandle->destroy(sourcehandle);
            logger_error("soundplayer_init() sndbridge_enqueue1() returned %i for: %s", res, src);
            return NULL;
        }

        fns.active_fade = (CallbackActiveFade)sndbridge_stream_active_fade;
        fns.destroy = (CallbackDestroy)sndbridge_stream_destroy;
        fns.do_fade = (CallbackDoFade)sndbridge_stream_do_fade;
        fns.duration = (CallbackDuration)sndbridge_stream_duration;
        fns.has_ended = (CallbackHasEnded)sndbridge_stream_has_ended;
        fns.set_queueing = (CallbackSetQueueing)sndbridge_stream_set_queueing;
        fns.force_resync = (CallbackForceResync)sndbridge_stream_force_resync;
        fns.is_active = (CallbackIsActive)sndbridge_stream_is_active;
        fns.mute = (CallbackMute)sndbridge_stream_mute;
        fns.pause = (CallbackPause)sndbridge_stream_pause;
        fns.play = (CallbackPlay)sndbridge_stream_play;
        fns.position = (CallbackPosition)sndbridge_stream_position;
        fns.seek = (CallbackSeek)sndbridge_stream_seek;
        fns.set_looped = (CallbackSetLooped)sndbridge_stream_set_looped;
        fns.set_volume = (CallbackSetVolume)sndbridge_stream_set_volume;
        fns.stop = (CallbackStop)sndbridge_stream_stop;

        instance = stream;
    }

    SoundPlayer soundplayer = malloc_chk(sizeof(struct SoundPlayer_s));
    malloc_assert(soundplayer, SoundPlayer);

    *soundplayer = (struct SoundPlayer_s){
        .sourcehandle = sourcehandle,
        .fns = fns,
        .instance = instance,
        .is_muted = false
    };

    return soundplayer;

L_failed:
    free_chk(absolute_path);
    free_chk(native_path);
    return NULL;
}

void soundplayer_destroy(SoundPlayer* soundplayer_ptr) {
    SoundPlayer soundplayer = *soundplayer_ptr;
    if (!soundplayer) return;

    luascript_drop_shared(soundplayer);

    soundplayer->fns.destroy(soundplayer->instance);
    soundplayer->sourcehandle->destroy(soundplayer->sourcehandle);

    soundplayer->instance = NULL;
    soundplayer->sourcehandle = NULL;

    free_chk(soundplayer);
    soundplayer_ptr = NULL;
}


void soundplayer_replay(SoundPlayer soundplayer) {
    soundplayer->fns.stop(soundplayer->instance);
    soundplayer->fns.play(soundplayer->instance);
}

void soundplayer_play(SoundPlayer soundplayer) {
    soundplayer->fns.play(soundplayer->instance);
}

void soundplayer_pause(SoundPlayer soundplayer) {
    soundplayer->fns.pause(soundplayer->instance);
}

void soundplayer_stop(SoundPlayer soundplayer) {
    soundplayer->fns.stop(soundplayer->instance);
}

void soundplayer_loop_enable(SoundPlayer soundplayer, bool enable) {
    soundplayer->fns.set_looped(soundplayer->instance, enable);
}

void soundplayer_fade(SoundPlayer soundplayer, bool in_or_out, float duration) {
    soundplayer->fns.do_fade(soundplayer->instance, in_or_out, duration / 1000.0f);
}


void soundplayer_set_volume(SoundPlayer soundplayer, float volume) {
    soundplayer->fns.set_volume(soundplayer->instance, volume);
}

void soundplayer_set_mute(SoundPlayer soundplayer, bool muted) {
    soundplayer->is_muted = muted;
    soundplayer->fns.mute(soundplayer->instance, muted);
}

void soundplayer_seek(SoundPlayer soundplayer, float64 timestamp) {
#ifdef _arch_dreamcast
    soundplayer->fns.seek(soundplayer->instance, (sndbridge_time_t)timestamp);
#else
    soundplayer->fns.seek(soundplayer->instance, timestamp / 1000.0);
#endif
}

void soundplayer_set_property(SoundPlayer soundplayer, int32_t property_id, float value) {
    switch (property_id) {
        case MEDIA_PROP_VOLUME:
            soundplayer_set_volume(soundplayer, value);
            break;
        case MEDIA_PROP_SEEK:
            soundplayer_seek(soundplayer, value);
            break;
        case MEDIA_PROP_PLAYBACK:
            switch ((int)value) {
                case PLAYBACK_PLAY:
                    soundplayer_play(soundplayer);
                    break;
                case PLAYBACK_PAUSE:
                    soundplayer_pause(soundplayer);
                    break;
                case PLAYBACK_STOP:
                    soundplayer_stop(soundplayer);
                    break;
                case PLAYBACK_MUTE:
                    soundplayer_set_mute(soundplayer, true);
                    break;
                case PLAYBACK_UNMUTE:
                    soundplayer_set_mute(soundplayer, false);
                    break;
            }
            break;
    }
}


bool soundplayer_is_muted(SoundPlayer soundplayer) {
    return soundplayer->is_muted;
}

bool soundplayer_is_playing(SoundPlayer soundplayer) {
    return soundplayer->fns.is_active(soundplayer->instance);
}

Fading soundplayer_has_fading(SoundPlayer soundplayer) {
    return (Fading)soundplayer->fns.active_fade(soundplayer->instance);
}

float64 soundplayer_get_duration(SoundPlayer soundplayer) {
#ifdef _arch_dreamcast
    return (float64)soundplayer->fns.duration(soundplayer->instance);
#else
    return soundplayer->fns.duration(soundplayer->instance) * 1000.0;
#endif
}

float64 soundplayer_get_position(SoundPlayer soundplayer) {
#ifdef _arch_dreamcast
    return (float64)soundplayer->fns.position(soundplayer->instance);
#else
    return soundplayer->fns.position(soundplayer->instance) * 1000.0;
#endif
}

bool soundplayer_has_ended(SoundPlayer soundplayer) {
    return soundplayer->fns.has_ended(soundplayer->instance);
}

void soundplayer_set_queueing(SoundPlayer soundplayer, bool enabled) {
    soundplayer->fns.set_queueing(soundplayer->instance, enabled);
}

void soundplayer_force_resync(SoundPlayer soundplayer) {
    soundplayer->fns.force_resync(soundplayer->instance);
}
