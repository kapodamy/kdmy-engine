
async function soundplayer_init(src) {
    // TODO: C version
    let full_path = await fs_get_full_path_and_override(src);

    if (!await fs_file_exists(full_path)) { return null; }
    if (!IO_CHROMIUM_DETECTED) return await soundplayer_init2(full_path);

    /** @type {ArrayBuffer} */
    let arraybuffer = await fs_readarraybuffer(full_path);
    let type = "application/octet-stream";

    if (full_path.endsWith(".ogg")) type = "audio/ogg";
    else if (full_path.endsWith(".wav")) type = "audio/wav";
    else if (full_path.endsWith(".mp3")) type = "audio/mp3";

    let blob = new Blob([arraybuffer], { type: type });
    return soundplayer_init2(blob);
}

async function soundplayer_init2(src) {
    //
    // TODO: C version
    //
    let blob_url = null;
    if (src instanceof Blob) src = blob_url = URL.createObjectURL(src);

    let soundplayer = {
        handler: new Audio(src),
        fade_id: 0,
        blob_url
    };

    soundplayer.handler.preload = "metadata";

    await new Promise(function (resolve, reject) {
        soundplayer.handler.oncanplay = function (evt) {
            soundplayer.handler.currentTime = 0;
            soundplayer.handler.oncanplay = null;
            soundplayer.handler.onerror = null;
            resolve();
        };
        soundplayer.handler.onerror = function (evt) {
            soundplayer.handler.oncanplay = null;
            soundplayer.handler.onerror = null;
            reject(this.error);
        }
    });

    return soundplayer;
}

function soundplayer_destroy(soundplayer) {
    if (soundplayer.handler) {
        soundplayer.fade_id++;
        soundplayer.handler.pause();
        soundplayer.handler.src = null;
        soundplayer.handler = undefined;
    }

    if (soundplayer.blob_url) URL.revokeObjectURL(soundplayer.blob_url);
}


function soundplayer_replay(soundplayer) {
    soundplayer.fade_id++;
    if (soundplayer.handler.paused)
        soundplayer_internal_handle_play(soundplayer);
    else
        soundplayer.handler.currentTime = 0;
}

function soundplayer_play(soundplayer) {
    soundplayer.fade_id++;
    if (!soundplayer.handler.paused) return;
    soundplayer_internal_handle_play(soundplayer);
}

function soundplayer_pause(soundplayer) {
    soundplayer.fade_id++;
    if (soundplayer.handler.paused) return;
    soundplayer.handler.pause();
}

function soundplayer_stop(soundplayer) {
    soundplayer.fade_id++;
    soundplayer.handler.pause();
    soundplayer.handler.currentTime = 0;
}

function soundplayer_loop_enable(soundplayer, enable) {
    soundplayer.handler.loop = enable;
}

function soundplayer_fade(soundplayer, in_or_out, duration) {
    soundplayer.handler.volume = in_or_out ? 0.0 : 1.0;

    let last_timestamp = -1;
    const fade_out = !in_or_out;
    let fade_id = ++soundplayer.fade_id;

    function ontimeupdate(timestamp) {
        if (soundplayer.fade_id != fade_id) return;
        if (last_timestamp < 0) last_timestamp = timestamp;

        let elapsed = timestamp - last_timestamp;
        let percent = elapsed / duration;
        if (fade_out) percent = 1.0 - percent;

        if (elapsed > duration) return;

        soundplayer.handler.volume = percent;// macroexecutor_calc_log(percent);
        requestAnimationFrame(ontimeupdate);
    };

    requestAnimationFrame(ontimeupdate);
}


function soundplayer_set_volume(soundplayer, volume) {
    soundplayer.handler.volume = volume;
}

function soundplayer_set_mute(soundplayer, muted) {
    soundplayer.handler.muted = muted;
}

function soundplayer_set_property(soundplayer, property_id, value) {
    switch (property_id) {
        case MEDIA_PROP_VOLUME:
            soundplayer_set_volume(soundplayer, value);
            break;
        case MEDIA_PROP_SEEK:
            soundplayer.handler.currentTime = value / 1000.0;
            break;
        case MEDIA_PROP_PLAYBACK:
            switch (value) {
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
                    soundplayer_set_mute(soundplayer, 1);
                    break;
                case PLAYBACK_UNMUTE:
                    soundplayer_set_mute(soundplayer, 0);
                    break;
            }
            break;
    }
}

function soundplayer_seek(soundplayer, timestamp) {
    soundplayer.handler.currentTime = timestamp / 1000.0;
}


function soundplayer_is_muted(soundplayer) {
    return soundplayer.handler.muted;
}

function soundplayer_is_playing(soundplayer) {
    return !soundplayer.handler.paused;
}

function soundplayer_get_duration(soundplayer) {
    return soundplayer.handler.duration * 1000.0;
}

function soundplayer_get_position(soundplayer) {
    return soundplayer.handler.currentTime * 1000.0;
}


function soundplayer_internal_handle_play(soundplayer) {
    soundplayer.handler.play().catch(function (e) {
        if (e.name === 'NotAllowedError') return;
        console.error(e);
    });
}

