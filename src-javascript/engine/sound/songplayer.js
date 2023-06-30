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
        paused: 1,
        index_instrumental: -1,
        index_voices: -1,
    };

    if (is_not_splitted) {
        let player = await songplayer_internal_init_player(path_voices ?? path_instrumental);
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
            player_voices = await songplayer_internal_init_player(path_voices);
        }

        if (path_instrumental) {
            player_instrumentals = await songplayer_internal_init_player(path_instrumental);
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
    if (songplayer.playbacks) {
        // javacript only (needs the C counterpart)
        for (let i = 0; i < songplayer.playbacks_size; i++) {
            if (IO_CHROMIUM_DETECTED) URL.revokeObjectURL(songplayer.playbacks[i].src);
            mastervolume_remove_mediaelement(songplayer.playbacks[i]);
            songplayer.playbacks[i].pause();
            songplayer.playbacks[i].srcObject = null;
            songplayer.playbacks[i].remove();
        }

        songplayer.playbacks = undefined;
    }

    ModuleLuaScript.kdmyEngine_drop_shared_object(songplayer);
    songplayer = undefined;
}

async function songplayer_play(songplayer, songinfo) {
    if (songplayer.playbacks_size < 1 || !songplayer.paused) return 0;

    let lowest_timestamp = Infinity;
    let reference_index = 0;
    let timestamp;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        timestamp = songplayer.playbacks[i].currentTime
        if (timestamp < lowest_timestamp) {
            lowest_timestamp = timestamp;
            reference_index = i;
        }
    }

    let completed = 0;
    let playback_success = 0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (songplayer.playbacks[i].currentTime >= songplayer.playbacks[i].duration) completed++;
        songplayer.playbacks[i].play().catch(function () { playback_success = 0; });
    }

    // wait until the first audio samples are played

    let reference = songplayer.playbacks[reference_index];
    timestamp = 0;

    while (playback_success && timestamp == reference.currentTime) await thd_pass();

    songplayer.paused = 0;
    let seconds = reference.currentTime;

    songinfo.timestamp = (seconds * songplayer.playbacks_size) / 1000;
    songinfo.completed = completed >= songplayer.playbacks_size;

    return (seconds - timestamp) / 1000;
}

function songplayer_pause(songplayer) {
    if (songplayer.playbacks_size < 1 || songplayer.paused) return;
    for (let i = 0; i < songplayer.playbacks_size; i++) songplayer.playbacks[i].pause();
    songplayer.paused = 1;
}

function songplayer_seek(songplayer, timestamp) {
    timestamp /= 1000.0;

    for (let i = 0; i < songplayer.playbacks_size; i++)
        songplayer.playbacks[i].currentTime = timestamp;
}

function songplayer_poll(songplayer, songinfo) {
    //
    // IMPORTANT: in the C version poll every stream (similar to animate() ) in
    // order to send audio samples to the AICA.
    //
    let ended = 0;
    let seconds = 0;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (songplayer.playbacks[i].ended) ended++;
        seconds += songplayer.playbacks[i].currentTime;
    }

    songinfo.timestamp = (seconds / songplayer.playbacks_size) * 1000;
    songinfo.completed = ended >= songplayer.playbacks_size;
}

function songplayer_get_duration(songplayer) {
    let duration = 0;

    for (let i = 0; i < songplayer.playbacks_size; i++) {
        /**@type {HTMLAudioElement} */
        let playback = songplayer.playbacks[i];
        let playback_duration = playback.duration;

        if (!Number.isFinite(playback_duration)) continue;
        if (playback_duration > duration) duration = playback_duration * 1000;
    }

    return duration;
}

async function songplayer_changesong(songplayer, src, prefer_alternative) {
    let song = await songplayer_init(src, prefer_alternative);

    if (!song) return 0;

    if (songplayer.playbacks) {
        // javacript only (needs the C counterpart)
        for (let i = 0; i < songplayer.playbacks_size; i++) {
            if (IO_CHROMIUM_DETECTED) URL.revokeObjectURL(songplayer.playbacks[i].src);
            mastervolume_remove_mediaelement(songplayer.playbacks[i]);
            songplayer.playbacks[i].pause();
            songplayer.playbacks[i].srcObject = null;
            songplayer.playbacks[i].remove();
        }
        songplayer.playbacks = undefined;
    }

    songplayer.index_instrumental = song.index_instrumental;
    songplayer.index_voices = song.index_voices;
    songplayer.paused = song.paused;
    songplayer.playbacks = song.playbacks;
    songplayer.playbacks_size = song.playbacks_size;
    song = undefined;

    return 1;
}

function songplayer_is_completed(songplayer) {
    let completed = 0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        if (songplayer.playbacks[i].currentTime >= songplayer.playbacks[i].duration) completed++;
    }

    return completed >= songplayer.playbacks_size;
}

function songplayer_get_timestamp(songplayer) {
    let timestamp = 0;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        timestamp += songplayer.playbacks[i].currentTime;
    }

    return (timestamp / songplayer.playbacks_size) * 1000;
}

function songplayer_mute_track(songplayer, vocals_or_instrumental, muted) {
    if (songplayer.playbacks_size < 1) return;
    let target = vocals_or_instrumental ? songplayer.index_voices : songplayer.index_instrumental;
    if (target < 0) return;
    songplayer.playbacks[target].muted = muted;
}

function songplayer_mute(songplayer, muted) {
    if (songplayer.playbacks_size < 1) return;
    muted = !!muted;
    for (let i = 0; i < songplayer.playbacks_size; i++) songplayer.playbacks[i].muted = muted;
}

function songplayer_set_volume(songplayer, volume) {
    if (songplayer.playbacks_size < 1) return;
    for (let i = 0; i < songplayer.playbacks_size; i++) {
        songplayer.playbacks[i].volume = volume * mastervolume_current_volume;
        songplayer.playbacks[i]["volume__original"] = volume;
    }
}

function songplayer_set_volume_track(songplayer, vocals_or_instrumental, volume) {
    if (songplayer.playbacks_size < 1) return;
    let target = vocals_or_instrumental ? songplayer.index_voices : songplayer.index_instrumental;
    if (target < 0) return;

    songplayer.playbacks[target].volume = volume * mastervolume_current_volume;
    songplayer.playbacks[target]["volume__original"] = volume;
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
        src = undefined;
        return path;
    }

    // parse "voices.ogg|inst.ogg" format
    let path_voices = await songplayer_internal_separe_paths(0, separator_index, src, false);
    let path_instrumental = await songplayer_internal_separe_paths(separator_index + 1, src.length, src, false);

    src = undefined;

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




// JS only
async function songplayer_internal_chrome_url(path) {
    let arraybuffer = await fs_readarraybuffer(path);
    let type = "";

    if (path.endsWith(".ogg") || path.endsWith(".logg")) type = "audio/ogg";
    else if (path.endsWith(".wav")) type = "audio/wav";
    else if (path.endsWith(".mp3")) type = "audio/mp3";

    let blob = new Blob([arraybuffer], { type: type });

    let obj_src = URL.createObjectURL(blob);

    if (obj_src == null) throw new Error("URL.createObjectURL() failed");

    return obj_src;
}

async function songplayer_internal_init_player(path) {
    if (IO_CHROMIUM_DETECTED) {
        try {
            path = await songplayer_internal_chrome_url(path);
        } catch (e) {
            console.error("songplayer_internal_init_player() failed to load the file: " + path, e);
            return null;
        }
    }

    try {
        return await (new Promise(function (resolve, reject) {
            let player = new Audio(path);
            player.preload = "metadata";

            player.oncanplay = function () {
                mastervolume_add_mediaelement(player);

                player.oncanplay = player.onerror = null;
                if (!IO_CHROMIUM_DETECTED) player.currentTime = 0;
                player.onerror = function () {
                    console.error(
                        `songplayer_internal_init_player() playback error: code=${player.error.code} message=${player.error.message}`
                    );
                };
                resolve(player);
            };
            player.onerror = function () {
                player.oncanplay = player.onerror = null;
                reject(this.error);
            }
        }));
    } catch (e) {
        if (IO_CHROMIUM_DETECTED) URL.revokeObjectURL(path);
        console.error("songplayer_internal_init_player() failed to initialize the audio: " + path, e);
        return null;
    }
}

