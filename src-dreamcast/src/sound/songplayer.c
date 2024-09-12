#include <stdbool.h>

#include <arch/timer.h>
#include <dc/sound/sound.h>
#include <kos/thread.h>

#include "externals/luascript.h"
#include "fs.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "songplayer.h"
#include "soundplayer.h"
#include "stringutils.h"


struct SongPlayer_s {
    bool paused;
    int32_t playbacks_size;
    SoundPlayer playbacks[2];
    int32_t index_instrumental;
    int32_t index_voices;
};


static const char* SONGPLAYER_SUFFIX_INSTRUMENTAL = "-inst";
static const char* SONGPLAYER_SUFFIX_VOICES = "-voices";
static const char* SONGPLAYER_SUFFIX_ALTERNATIVE = "-alt";
static const char* SONGPLAYER_NAME_INSTRUMENTAL = "Inst";
static const char* SONGPLAYER_NAME_VOICES = "Voices";


static char* songplayer_internal_separe_paths(int32_t start, int32_t end, const char* src, bool check_if_exists);


SongPlayer songplayer_init(const char* src, bool prefer_alternative) {
    char* path_voices;
    char* path_instrumental;
    bool is_not_splitted = songplayer_helper_get_tracks(
        src, prefer_alternative, &path_voices, &path_instrumental
    );

    if (path_voices == NULL && path_instrumental == NULL && !is_not_splitted) {
        logger_error("songplayer_init() fallback failed, missing file: %s", src);
        logger_error("songplayer_init() cannot load any file, there will only be silence.");

        SongPlayer songplayer = malloc_chk(sizeof(struct SongPlayer_s));
        malloc_assert(songplayer, SongPlayer);

        *songplayer = (struct SongPlayer_s){
            .playbacks_size = 0,
            .paused = true,
            .index_instrumental = -1,
            .index_voices = -1
        };
        return songplayer;
    }

    SongPlayer songplayer;

    if (is_not_splitted) {
        songplayer = songplayer_init2(true, src, NULL);
    } else {
        songplayer = songplayer_init2(false, path_voices, path_instrumental);
        free_chk(path_voices);
        free_chk(path_instrumental);
    }

    return songplayer;
}

SongPlayer songplayer_init2(bool is_not_splitted, const char* path_voices, const char* path_instrumental) {
    SongPlayer songplayer = malloc_chk(sizeof(struct SongPlayer_s));
    malloc_assert(songplayer, songplayer);

    *songplayer = (struct SongPlayer_s){
        .playbacks = {0x00, 0x00},
        .playbacks_size = 0,
        .paused = true,
        .index_instrumental = -1,
        .index_voices = -1,
    };

    if (is_not_splitted) {
        SoundPlayer player = soundplayer_init(path_voices ? path_voices : path_instrumental);
        if (player) {
            songplayer->playbacks_size = 1;
            songplayer->playbacks[0] = player;
            songplayer->index_instrumental = 0;
            songplayer->index_voices = 0;
        }
    } else {
        SoundPlayer player_voices = NULL;
        SoundPlayer player_instrumentals = NULL;

        if (path_voices) {
            player_voices = soundplayer_init(path_voices);
        }

        if (path_instrumental) {
            player_instrumentals = soundplayer_init(path_instrumental);
        }

        if (player_voices) {
            songplayer->index_voices = songplayer->playbacks_size;
            songplayer->playbacks_size++;
        }
        if (player_instrumentals) {
            songplayer->index_instrumental = songplayer->playbacks_size;
            songplayer->playbacks_size++;
        }

        int32_t index = 0;
        if (player_voices) songplayer->playbacks[index++] = player_voices;
        if (player_instrumentals) songplayer->playbacks[index++] = player_instrumentals;
    }

    return songplayer;
}

void songplayer_destroy(SongPlayer* songplayer_ptr) {
    SongPlayer songplayer = *songplayer_ptr;
    if (!songplayer) return;

    luascript_drop_shared(songplayer);

    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_destroy(&songplayer->playbacks[i]);
    }

    free_chk(songplayer);
    *songplayer_ptr = NULL;
}

void songplayer_play(SongPlayer songplayer, SongPlayerInfo* songinfo) {
    if (songplayer->playbacks_size < 1 || !songplayer->paused) return;

    float64 lowest_duration = DOUBLE_Inf;
    int32_t reference_index = 0;

    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        float64 duration = soundplayer_get_duration(songplayer->playbacks[i]);
        if (duration < lowest_duration) {
            lowest_duration = duration;
            reference_index = i;
        }
    }

    // just in case, allow any I/O background operation to run/complete
    thd_pass();

    //
    // Before playing enable queuing (from KallistiOS snd_stream api) this
    // halts the AICA commands queue which is necessary to start both
    // sound streams in sync. Later call snd_sh4_to_aica_start() function
    // to resume the queue processing which has the same effect as
    // snd_stream_queue_go() function (at least internally)
    //

    int32_t completed = 0;
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        if (soundplayer_has_ended(songplayer->playbacks[i])) completed++;
        soundplayer_set_queueing(songplayer->playbacks[i], true);
        soundplayer_play(songplayer->playbacks[i]);
        soundplayer_set_queueing(songplayer->playbacks[i], false);
    }

    // resume AICA queue processing
    snd_sh4_to_aica_start();
    timer_spin_sleep(1);

    // disable interrupts to ensure the same start timestamp of each track
    int irq = irq_disable();

    // resync all tracks start timestamps
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_force_resync(songplayer->playbacks[i]);
    }

    songplayer->paused = false;

    // wait until the first audio samples are played
    SoundPlayer reference = songplayer->playbacks[reference_index];

    songinfo->timestamp = soundplayer_get_position(reference);
    songinfo->completed = completed >= songplayer->playbacks_size;

    // now restore interrupts
    irq_restore(irq);
}

void songplayer_pause(SongPlayer songplayer) {
    if (songplayer->paused) return;
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_pause(songplayer->playbacks[i]);
    }
    songplayer->paused = true;
}

void songplayer_seek(SongPlayer songplayer, float64 timestamp) {
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_seek(songplayer->playbacks[i], timestamp);
    }
}

void songplayer_poll(SongPlayer songplayer, SongPlayerInfo* songinfo) {
    int32_t ended = 0;
    float64 timestamp = 0.0;

    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        float64 position = soundplayer_get_position(songplayer->playbacks[i]);
        if (soundplayer_has_ended(songplayer->playbacks[i])) ended++;
        timestamp += position;
    }

    songinfo->timestamp = timestamp / songplayer->playbacks_size;
    songinfo->completed = ended >= songplayer->playbacks_size;
}

float64 songplayer_get_duration(SongPlayer songplayer) {
    float64 duration = 0.0;

    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        float64 playback_duration = soundplayer_get_duration(songplayer->playbacks[i]);

        if (playback_duration < 0.0) continue;
        if (playback_duration > duration) duration = playback_duration;
    }

    return duration;
}

bool songplayer_changesong(SongPlayer songplayer, const char* src, bool prefer_alternative) {
    SongPlayer song = songplayer_init(src, prefer_alternative);

    if (!song) return false;

    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_destroy(&songplayer->playbacks[i]);
    }

    songplayer->index_instrumental = song->index_instrumental;
    songplayer->index_voices = song->index_voices;
    songplayer->paused = true;
    songplayer->playbacks[0] = song->playbacks[0];
    songplayer->playbacks[1] = song->playbacks[1];
    songplayer->playbacks_size = song->playbacks_size;
    free_chk(song);

    return true;
}

bool songplayer_is_completed(SongPlayer songplayer) {
    int32_t completed = 0;
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        if (soundplayer_has_ended(songplayer->playbacks[i])) completed++;
    }

    return completed >= songplayer->playbacks_size;
}

float64 songplayer_get_timestamp(SongPlayer songplayer) {
    float64 timestamp = 0.0;
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        timestamp += soundplayer_get_position(songplayer->playbacks[i]);
    }
    return timestamp / songplayer->playbacks_size;
}

void songplayer_mute_track(SongPlayer songplayer, bool vocals_or_instrumental, bool muted) {
    int32_t target = vocals_or_instrumental ? songplayer->index_voices : songplayer->index_instrumental;
    if (target < 0) return;
    soundplayer_set_mute(songplayer->playbacks[target], muted);
}

void songplayer_mute(SongPlayer songplayer, bool muted) {
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_set_mute(songplayer->playbacks[i], muted);
    }
}

void songplayer_set_volume(SongPlayer songplayer, float volume) {
    for (int32_t i = 0; i < songplayer->playbacks_size; i++) {
        soundplayer_set_volume(songplayer->playbacks[i], volume);
    }
}

void songplayer_set_volume_track(SongPlayer songplayer, bool vocals_or_instrumental, float volume) {
    int32_t target = vocals_or_instrumental ? songplayer->index_voices : songplayer->index_instrumental;
    if (target < 0) return;
    soundplayer_set_volume(songplayer->playbacks[target], volume);
}


bool songplayer_helper_get_tracks(const char* src, bool prefer_alternative, char** path_voices, char** path_instrumental) {
    *path_voices = NULL;
    *path_instrumental = NULL;
    bool is_not_splitted = false;

    if (string_is_empty(src)) return is_not_splitted;

    int32_t separator_index = string_index_of_char(src, 0, SONGPLAYER_TRACKS_SEPARATOR);
    if (separator_index >= 0) {
        // parse "voices.ogg|inst.ogg" format
        *path_voices = songplayer_internal_separe_paths(0, separator_index, src, true);
        *path_instrumental = songplayer_internal_separe_paths(separator_index + 1, (int32_t)strlen(src), src, true);

        return is_not_splitted;
    }

    int32_t dot_index = string_last_index_of_char(src, '.');
    if (dot_index < 0) {
        logger_error("missing file extension : %s", src);
        assert(dot_index >= 0);
    }


    if (prefer_alternative) {
        src = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_ALTERNATIVE);
    }

    if (fs_file_exists(src)) {
        is_not_splitted = true;
    } else {
        // check if the song is splited in voices and instrumental
        char* voices;
        char* instrumental;

        if (dot_index == 0 || src[dot_index - 1] == FS_CHAR_SEPARATOR) {
            if (dot_index > 0) {
                char* folder_path = string_substring(src, 0, dot_index - 1);
                if (!fs_folder_exists(folder_path)) {
                    logger_warn("songplayer_init() folder not found: %s", src);
                }
                free_chk(folder_path);
            }
            // src points to a folder, load files with names "Voices.ogg" and "Inst.ogg"
            voices = string_copy_and_insert(src, dot_index, SONGPLAYER_NAME_VOICES);
            instrumental = string_copy_and_insert(src, dot_index, SONGPLAYER_NAME_INSTRUMENTAL);
        } else {
            // absolute filenames "songame-voices.ogg" and "songname-inst.ogg"
            voices = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_VOICES);
            instrumental = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_INSTRUMENTAL);
        }

        if (fs_file_exists(voices)) {
            *path_voices = voices;
        } else {
            logger_warn("songplayer_init() missing voices: %s", voices);
            free_chk(voices);
        }

        if (fs_file_exists(instrumental)) {
            *path_instrumental = instrumental;
        } else {
            logger_warn("songplayer_init() missing instrumental: %s", instrumental);
            free_chk(instrumental);
        }
    }

    if (!*path_instrumental && !*path_voices && !is_not_splitted) {
        if (prefer_alternative) free_chk((char*)src);
    }

    return is_not_splitted;
}

char* songplayer_helper_get_tracks_full_path(const char* src) {
    int32_t separator_index = string_index_of_char(src, 0, SONGPLAYER_TRACKS_SEPARATOR);

    if (separator_index < 0) {
        char* path = fs_get_full_path(src);
        return path;
    }

    // parse "voices.ogg|inst.ogg" format
    char* path_voices = songplayer_internal_separe_paths(0, separator_index, src, false);
    char* path_instrumental = songplayer_internal_separe_paths(separator_index + 1, (int32_t)strlen(src), src, false);

    if (path_voices) {
        char* tmp = fs_get_full_path(path_voices);
        free_chk(path_voices);
        path_voices = tmp;
    }

    if (path_instrumental) {
        char* tmp = fs_get_full_path(path_instrumental);
        free_chk(path_instrumental);
        path_instrumental = tmp;
    }

    char* new_src = string_concat(3, path_voices, (char[2]){SONGPLAYER_TRACKS_SEPARATOR, '\0'}, path_instrumental);

    free_chk(path_voices);
    free_chk(path_instrumental);
    return new_src;
}

static char* songplayer_internal_separe_paths(int32_t start, int32_t end, const char* src, bool check_if_exists) {
    if (start == end) return NULL;

    char* path = string_substring(src, start, end);

    if (!check_if_exists || fs_file_exists(path)) return path;

    logger_warn("songplayer_internal_separe_paths() missing: %s", path);

    free_chk(path);
    return NULL;
}
