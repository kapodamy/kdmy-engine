"use strict";

const SONGPLAYER_SUFFIX_INSTRUMENTAL = "-inst";
const SONGPLAYER_SUFFIX_VOICES = "-voices";
const SONGPLAYER_SUFFIX_ALTERNATIVE = "-alt";
const SONGPLAYER_NAME_INSTRUMENTAL = "Inst";
const SONGPLAYER_NAME_VOICES = "Voices";
const SONGPLAYER_TRACKS_SEPARATOR = '|';


async function songplayer_init(src, prefer_alternative) {
    const output_paths = [null, null];
    let is_not_splitted = await songplayer_helper_get_tracks(
        src, prefer_alternative, output_paths
    );
    let path_voices = output_paths[0], path_instrumental = output_paths[1];

    if (path_voices == null && path_instrumental == null && !is_not_splitted) {
        console.error("songplayer_init() fallback failed, missing file: " + src);
        console.error("songplayer_init() cannot load any file, there will only be silence.");

        return { playbacks: null, playbacks_size: 0, paused: 1, index_instrumental: -1, index_voices: -1 };
    }

    let songplayer;

    if (is_not_splitted) {
        songplayer = await songplayer_init2(1, src, null);
    } else {
        songplayer = await songplayer_init2(0, path_voices, path_instrumental);
        path_voices = undefined;
        path_instrumental = undefined;
    }

    return songplayer;
}

async function songplayer_init2(is_not_splitted, path_voices, path_instrumental) {
    let songplayer = {
        playbacks: null,
        playbacks_size: 0,
        paused: true,
        index_instrumental: -1,
        index_voices: -1,
    };

    if (is_not_splitted) {
        let player = await soundplayer_init(path_voices ?? path_instrumental);
        if (player) {
            songplayer.playbacks = new Array(1);
            songplayer.playbacks_size = 1;
            songplayer.playbacks[0] = player;
            songplayer.index_instrumental = 0;
            songplayer.index_voices = 0;
        }
    } else {
        let player_voices = null;
        let player_instrumentals = null;

        if (path_voices) {
            player_voices = await soundplayer_init(path_voices);
        }

        if (path_instrumental) {
            player_instrumentals = await soundplayer_init(path_instrumental);
        }

        if (player_voices) {
            songplayer.index_voices = songplayer.playbacks_size;
            songplayer.playbacks_size++;
        }
        if (player_instrumentals) {
            songplayer.index_instrumental = songplayer.playbacks_size;
            songplayer.playbacks_size++;
        }

        let index = 0;
        songplayer.playbacks = new Array(songplayer.playbacks_size);

        if (player_voices) songplayer.playbacks[index++] = player_voices;
        if (player_instrumentals) songplayer.playbacks[index++] = player_instrumentals;

    }

    return songplayer;
}

function songplayer_destroy(songplayer) {
    luascript_drop_shared(songplayer);

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_destroy(songplayer.playbacks[i]);
    }
    songplayer.playbacks = undefined;

    songplayer = undefined;
}

async function songplayer_play(songplayer, songinfo) {
    if (songplayer.playbacks_size < 1 || !songplayer.paused) return;

    let lowest_duration = Infinity;
    let reference_index = 0;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        let duration = soundplayer_get_duration(songplayer.playbacks[i]);
        if (duration < lowest_duration) {
            lowest_duration = duration;
            reference_index = i;
        }
    }

    // just in case, allow any I/O background operation to run/complete
    await thd_pass();

    //
    // Before playing enable queuing (from KallistiOS snd_stream api) this
    // halts the AICA commands queue which is necessary to start both
    // sound streams in sync. Later call snd_sh4_to_aica_start() function
    // to resume the queue processing which has the same effect as
    // snd_stream_queue_go() function (at least internally).
    //

    let completed = 0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (soundplayer_has_ended(songplayer.playbacks[i])) completed++;
        soundplayer_set_queueing(songplayer.playbacks[i], true);
        soundplayer_play(songplayer.playbacks[i]);
        soundplayer_set_queueing(songplayer.playbacks[i], false);
    }

    /*
    // dreamcast only

    // resume AICA queue processing
    snd_sh4_to_aica_start();
    timer_spin_sleep(1);

    //disable interrupts to ensure the same start timestamp of each track
    int irq = irq_disable();

    // resync all tracks start timestamps
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_force_resync(songplayer.playbacks[i]);
    }
    */

    songplayer.paused = false;

    // wait until the first audio samples are played
    let reference = songplayer.playbacks[reference_index];

    songinfo.timestamp = soundplayer_get_position(reference);
    songinfo.completed = completed >= songplayer.playbacks_size;

    // (dreamcast only) now restore interrupts
    //irq_restore(irq);
}

function songplayer_pause(songplayer) {
    if (songplayer.paused) return;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_pause(songplayer.playbacks[i]);
    }
    songplayer.paused = true;
}

function songplayer_seek(songplayer, timestamp) {
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_seek(songplayer.playbacks[i], timestamp);
    }
}

function songplayer_poll(songplayer, songinfo) {
    let ended = 0;
    let timestamp = 0.0;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (soundplayer_has_ended(songplayer.playbacks[i])) ended++;
        timestamp += soundplayer_get_position(songplayer.playbacks[i]);
    }

    songinfo.timestamp = timestamp / songplayer.playbacks_size;
    songinfo.completed = ended >= songplayer.playbacks_size;
}

function songplayer_get_duration(songplayer) {
    let duration = 0.0;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        let playback_duration = soundplayer_get_duration(songplayer.playbacks[i]);

        if (!Number.isFinite(playback_duration)) continue;
        if (playback_duration > duration) duration = playback_duration;
    }

    return duration;
}

async function songplayer_changesong(songplayer, src, prefer_alternative) {
    let song = await songplayer_init(src, prefer_alternative);
    if (!song) return false;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_destroy(songplayer.playbacks[i]);
    }

    songplayer.index_instrumental = song.index_instrumental;
    songplayer.index_voices = song.index_voices;
    songplayer.paused = true;
    songplayer.playbacks = song.playbacks;
    songplayer.playbacks_size = song.playbacks_size;
    song = undefined;

    return true;
}

function songplayer_is_completed(songplayer) {
    let completed = 0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (soundplayer_has_ended(songplayer.playbacks[i])) completed++;
    }
    return completed >= songplayer.playbacks_size;
}

function songplayer_get_timestamp(songplayer) {
    let timestamp = 0.0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        timestamp += soundplayer_get_position(songplayer.playbacks[i]);
    }
    return timestamp / songplayer.playbacks_size;
}

function songplayer_mute_track(songplayer, vocals_or_instrumental, muted) {
    let target = vocals_or_instrumental ? songplayer.index_voices : songplayer.index_instrumental;
    if (target < 0) return;
    soundplayer_set_mute(songplayer.playbacks[target], muted);
}

function songplayer_mute(songplayer, muted) {
    muted = !!muted;
    for (let i = 0; i < songplayer.playbacks_size; i++) soundplayer_set_mute(songplayer.playbacks[i], muted);
}

function songplayer_set_volume(songplayer, volume) {
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        soundplayer_set_volume(songplayer.playbacks[i], volume);
    }
}

function songplayer_set_volume_track(songplayer, vocals_or_instrumental, volume) {
    let target = vocals_or_instrumental ? songplayer.index_voices : songplayer.index_instrumental;
    if (target < 0) return;
    soundplayer_set_volume(songplayer.playbacks[target], volume);
}


async function songplayer_helper_get_tracks(src, prefer_alternative, output_paths) {
    let path_voices = null;
    let path_instrumental = null;
    let is_not_splitted = 0;

    if (!src) return is_not_splitted;

    let separator_index = src.indexOf(SONGPLAYER_TRACKS_SEPARATOR);
    if (separator_index >= 0) {
        // parse "voices.ogg|inst.ogg" format
        path_voices = await songplayer_internal_separe_paths(0, separator_index, src, 1);
        path_instrumental = await songplayer_internal_separe_paths(separator_index + 1, src.length, src, 1);

        output_paths[0] = path_voices;
        output_paths[1] = path_instrumental;
        return is_not_splitted;
    }

    let dot_index = src.lastIndexOf('.');
    if (dot_index < 0) throw new Error("missing file extension : " + src);


    if (prefer_alternative) {
        src = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_ALTERNATIVE);
    }

    if (await fs_file_exists(src)) {
        is_not_splitted = 1;
    } else {
        // check if the song is splited in voices and instrumental
        let voices, instrumental;

        if (dot_index == 0 || src[dot_index - 1] == FS_CHAR_SEPARATOR) {
            if (dot_index > 0) {
                let folder_path = src.substring(0, dot_index - 1);
                if (!await fs_folder_exists(folder_path)) {
                    console.warn("songplayer_init() folder not found: " + src);
                }
                folder_path = undefined;
            }
            // src points to a folder, load files with names "Voices.ogg" and "Inst.ogg"
            voices = string_copy_and_insert(src, dot_index, SONGPLAYER_NAME_VOICES);
            instrumental = string_copy_and_insert(src, dot_index, SONGPLAYER_NAME_INSTRUMENTAL);
        } else {
            // absolute filenames "songame-voices.ogg" and "songname-inst.ogg"
            voices = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_VOICES);
            instrumental = string_copy_and_insert(src, dot_index, SONGPLAYER_SUFFIX_INSTRUMENTAL);
        }

        if (await fs_file_exists(voices)) {
            path_voices = voices;
        } else {
            console.warn("songplayer_init() missing voices: " + voices);
            voices = undefined;
        }

        if (await fs_file_exists(instrumental)) {
            path_instrumental = instrumental;
        } else {
            console.warn("songplayer_init() missing instrumental: " + instrumental);
            instrumental = undefined;
        }
    }

    if (path_instrumental == null && path_voices == null && !is_not_splitted) {
        if (prefer_alternative) src = undefined;
    }

    output_paths[0] = path_voices;
    output_paths[1] = path_instrumental;
    return is_not_splitted;
}


async function songplayer_helper_get_tracks_full_path(src) {
    let separator_index = src.indexOf(SONGPLAYER_TRACKS_SEPARATOR);

    if (separator_index < 0) {
        let path = fs_get_full_path(src);
        return path;
    }

    // parse "voices.ogg|inst.ogg" format
    let path_voices = await songplayer_internal_separe_paths(0, separator_index, src, false);
    let path_instrumental = await songplayer_internal_separe_paths(separator_index + 1, src.length, src, false);

    if (path_voices != null) {
        let tmp = fs_get_full_path(path_voices);
        path_voices = undefined;
        path_voices = tmp;
    }

    if (path_instrumental != null) {
        let tmp = fs_get_full_path(path_instrumental);
        path_instrumental = undefined;
        path_instrumental = tmp;
    }

    src = string_concat(3, path_voices, SONGPLAYER_TRACKS_SEPARATOR, path_instrumental);

    path_voices = undefined;
    path_instrumental = undefined;
    return src;
}

async function songplayer_internal_separe_paths(start, end, src, check_if_exists) {
    if (start == end) return null;

    let path = src.substring(start, end);

    if (!check_if_exists || await fs_file_exists(path)) return path;

    console.warn("songplayer_internal_separe_paths() missing: " + path);

    path = undefined;
    return null;
}

