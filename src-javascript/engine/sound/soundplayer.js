"use strict";

const FADING_NONE = 0;
const FADING_IN = 1;
const FADING_OUT = 2;

async function soundplayer_init(src) {
    // TODO: C version
    let full_path = await fs_get_full_path_and_override(src);

    if (!await fs_file_exists(full_path)) { return null; }
    if (!IO_WEBKIT_DETECTED) return await soundplayer_init2(full_path, null);

    /** @type {ArrayBuffer} */
    let arraybuffer = await fs_readarraybuffer(full_path);
    let type = "application/octet-stream";

    if (full_path.endsWith(".ogg") || full_path.endsWith(".logg")) type = "audio/ogg";
    else if (full_path.endsWith(".wav")) type = "audio/wav";
    else if (full_path.endsWith(".mp3")) type = "audio/mp3";

    let blob = new Blob([arraybuffer], { type: type });
    return soundplayer_init2(blob, arraybuffer);
}

async function soundplayer_init2(src, arraybuffer) {
    //
    // TODO: C version
    //
    let blob_url = null;
    if (src instanceof Blob) src = blob_url = URL.createObjectURL(src);

    let soundplayer = {
        handler: new Audio(src),
        fade_id: 0,
        fade_status: FADING_NONE,
        blob_url,
        sample_rate: Infinity,
        loop_start: -1,
        loop_length: -1,
        seek_request: false
    };

    soundplayer.handler.preload = "metadata";

    await new Promise(function (resolve, reject) {
        soundplayer.handler.oncanplay = async function (evt) {
            soundplayer.handler.currentTime = 0;
            soundplayer.handler.oncanplay = null;
            soundplayer.handler.onerror = null;
            mastervolume_add_mediaelement(soundplayer.handler);
            try {
                let points = await soundplayer_internal_read_looping_points(arraybuffer ?? src);
                soundplayer.sample_rate = points.sample_rate;
                soundplayer.loop_start = points.loop_start;
                soundplayer.loop_length = points.loop_length;
                soundplayer_internal_loop_listener(soundplayer);
            } catch (e) {
                console.warn("soundplayer_init2() failed to read looping points or the file is not OGG", e);
            }
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
        mastervolume_remove_mediaelement(soundplayer.handler);
    }

    if (soundplayer.blob_url) URL.revokeObjectURL(soundplayer.blob_url);

    luascript_drop_shared(soundplayer);
    soundplayer = undefined;
}


function soundplayer_replay(soundplayer) {
    soundplayer.fade_id++;
    soundplayer.fade_status = FADING_NONE;
    soundplayer.handler.currentTime = 0;
    soundplayer.seek_request = true;
    if (soundplayer.handler.paused)
        soundplayer_internal_handle_play(soundplayer);
}

function soundplayer_play(soundplayer) {
    soundplayer.fade_id++;
    soundplayer.fade_status = FADING_NONE;
    if (!soundplayer.handler.paused) return;
    soundplayer_internal_handle_play(soundplayer);
}

function soundplayer_pause(soundplayer) {
    soundplayer.fade_id++;
    soundplayer.fade_status = FADING_NONE;
    if (soundplayer.handler.paused) return;
    soundplayer.handler.pause();
}

function soundplayer_stop(soundplayer) {
    soundplayer.fade_id++;
    soundplayer.fade_status = FADING_NONE;
    soundplayer.seek_request = true;
    soundplayer.handler.pause();
    soundplayer.handler.currentTime = 0;
}

function soundplayer_loop_enable(soundplayer, enable) {
    soundplayer.handler.loop = enable;
}

function soundplayer_fade(soundplayer, in_or_out, duration) {
    let initial_volume = in_or_out ? 0.0 : 1.0;
    soundplayer.handler["volume__original"] = initial_volume;
    soundplayer.handler.volume = initial_volume * mastervolume_current_volume;
    soundplayer.fade_status = in_or_out ? FADING_IN : FADING_OUT;

    let last_timestamp = -1;
    const fade_out = !in_or_out;
    let fade_id = ++soundplayer.fade_id;

    function ontimeupdate(timestamp) {
        if (soundplayer.fade_id != fade_id) return;
        if (last_timestamp < 0) last_timestamp = timestamp;

        let elapsed = timestamp - last_timestamp;
        let percent = elapsed / duration;
        if (fade_out) percent = 1.0 - percent;

        if (elapsed > duration) {
            soundplayer.fade_status = FADING_NONE;
            return;
        }

        //percent = macroexecutor_calc_log(percent);

        soundplayer.handler.volume = percent * mastervolume_current_volume;
        soundplayer.handler["volume__original"] = percent;
        requestAnimationFrame(ontimeupdate);
    };

    requestAnimationFrame(ontimeupdate);
}


function soundplayer_set_volume(soundplayer, volume) {
    soundplayer.handler.volume = volume * mastervolume_current_volume;
    soundplayer.handler["volume__original"] = volume;
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
            switch (parseInt(value)) {
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
    soundplayer.seek_request = true;
    soundplayer.handler.currentTime = timestamp / 1000.0;
}

function soundplayer_has_ended(soundplayer) {
    return soundplayer.handler.ended
}


function soundplayer_is_muted(soundplayer) {
    return soundplayer.handler.muted;
}

function soundplayer_is_playing(soundplayer) {
    return !soundplayer.handler.paused;
}

function soundplayer_has_fading(soundplayer) {
    return soundplayer.fade_status;
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

async function soundplayer_internal_read_looping_points(src) {
    const PACKET_SIGNATURE = 0x4F676753;
    const PACKET_VERSION = 0x00000000;
    const PACKET_TYPE0 = 0x00;
    const PACKET_TYPE2 = 0x02;
    const HEADER_TYPE_METADATA = 0x01;
    const HEADER_TYPE_COMMENT = 0x03;
    const HEADER_VORBIS_SIGNATURE = [0x76, 0x6F, 0x72, 0x62, 0x69, 0x73];

    /**@type {ArrayBuffer}*/let arraybuffer;
    if (src instanceof ArrayBuffer)
        arraybuffer = src;
    else
        arraybuffer = await fs_readarraybuffer(src);

    let dataview = new DataView(arraybuffer);
    let offset = 0;
    /**@type {string[]}*/let comments = new Array();
    let sample_rate = Infinity;

    while (offset < arraybuffer.byteLength && comments.length < 1) {
        if (dataview.getInt32(offset, false) == PACKET_SIGNATURE) {
            offset += 4;
        } else {
            offset++;
            continue;
        }

        let valid_packet = dataview.getInt8(offset++) == PACKET_VERSION;
        let packet_type = dataview.getInt8(offset++);
        valid_packet &&= packet_type == PACKET_TYPE0 || packet_type == PACKET_TYPE2;

        // skip granulate position
        offset += 8;
        // skip bitstream serial number
        offset += 4;
        // skip page sequence number
        offset += 4;
        //skip  page checksum
        offset += 4;

        let segment_table_size = dataview.getUint8(offset++);

        let packet_size = 0;
        /*let comments_info_size = 0;
        let magic_0xDF_found = false;*/
        for (let i = 0; i < segment_table_size; i++) {
            let entry = dataview.getUint8(offset++);
            packet_size += entry;

            /*if (!magic_0xDF_found) {
                comments_info_size += entry;
                if (entry == 0xDF) magic_0xDF_found = false;
            }*/
        }

        if (!valid_packet) {
            // skip current packet
            offset += packet_size;
            continue;
        }

        // first bytes should belong to comments
        if (packet_type == PACKET_TYPE0 && dataview.getInt8(offset) == HEADER_TYPE_COMMENT) {
            offset++;
        } else if (packet_type == PACKET_TYPE2 && dataview.getInt8(offset) == HEADER_TYPE_METADATA) {
            offset++;
        } else {
            // failed, skip current packet
            offset += packet_size;
            continue;
        }

        let next_packet_offset = offset - 1 + packet_size;

        // check for "vorbis" header signature. // TODO: implement for opus streams
        for (let i = 0; i < HEADER_VORBIS_SIGNATURE.length; i++) {
            if (dataview.getUint8(offset + i) != HEADER_VORBIS_SIGNATURE[i]) {
                valid_packet = false;
                break;
            }
        }
        if (valid_packet) {
            // correct signature found
            offset += HEADER_VORBIS_SIGNATURE.length;
        }

        if (packet_type == PACKET_TYPE2) {
            if (Number.isNaN(sample_rate)) {
                // pick sample rate
                sample_rate = dataview.getUint32(offset + 5, true);
            }

            offset = next_packet_offset;
            continue;
        }

        //
        // Note: all following integers fields are litte-endian
        //
        let text_decoder = new TextDecoder("utf-8", { fatal: true, ignoreBOM: true });

        // skip vendor string
        let vendor_size = dataview.getUint32(offset, true); offset += 4;
        offset += vendor_size;

        let comments_count = dataview.getUint32(offset, true); offset += 4;
        for (let i = 0; i < comments_count; i++) {
            let str_length = dataview.getUint32(offset, true); offset += 4;
            let str = text_decoder.decode(new Uint8Array(arraybuffer, offset, str_length), { stream: false });

            offset += str_length;
            comments.push(str);
        }

        if (offset >= next_packet_offset || dataview.getUint8(offset) != 0x01) {
            // issued by OGG specification
            throw new Error("missing framing bit");
        }

        // point next packet
        offset = next_packet_offset;
    }

    const LOOPSTART = "LOOPSTART=";
    const LOOPLENGTH = "LOOPLENGTH=";
    let loop_start = -1;
    let loop_length = -1;

    for (let comment of comments) {
        if (comment.startsWith(LOOPSTART)) {
            let str = comment.substring(LOOPSTART.length);
            loop_start = parseInt(str, 10);
        }
        if (comment.startsWith(LOOPLENGTH)) {
            let str = comment.substring(LOOPLENGTH.length);
            loop_length = parseInt(str, 10);
        }
    }

    return { sample_rate, loop_start, loop_length };
}

function soundplayer_internal_loop_listener(soundplayer) {
    /**@type {HTMLAudioElement}*/let audio = soundplayer.handler;
    /**@type {number}*/let loop_start_ms = soundplayer.loop_start;
    /**@type {number}*/let loop_end_ms = soundplayer.loop_length;

    if (soundplayer.sample_rate < 0 || Number.isNaN(soundplayer.sample_rate)) {
        return;
    }
    if (loop_start_ms < 0 || loop_start_ms == loop_end_ms) {
        return;
    }
    if (loop_end_ms < 1) {
        //return;
        loop_end_ms = (audio.duration * soundplayer.sample_rate) - loop_start_ms;
    }

    // convert samples to seconds
    loop_start_ms /= soundplayer.sample_rate / 1000.0;
    loop_end_ms /= soundplayer.sample_rate / 1000.0;

    loop_end_ms += loop_start_ms;

    // enable loop by default
    audio.loop = true;

    //
    // Note: the precission of the loop end point is limited in how fast the web browser
    //       dispatchs the looper.
    //
    let loop_ts = -1;
    let last_ts = -1;
    let calc_diff = false;
    function looper(ts) {
        if (!soundplayer.handler) return;

        L_runtime: {
            if (soundplayer.seek_request) {
                loop_ts = -1;
                soundplayer.seek_request = false;
                break L_runtime;
            }
            if (audio.paused || !audio.loop) {
                loop_ts = -1;
                break L_runtime;
            }
            if (loop_ts < 0) {
                loop_ts = ts + loop_end_ms - (audio.currentTime * 1000.0);
                calc_diff = true;
                last_ts = ts;
                break L_runtime;
            }
            if (calc_diff) {
                // frame correction (works if the engine runs at 60fps)
                loop_ts -= (ts - last_ts) / 2;
                calc_diff = false;
            }
            if (ts > loop_ts) {
                loop_ts = ts + loop_end_ms - loop_start_ms;
                audio.currentTime = loop_start_ms / 1000.0;
            }
        }
        requestAnimationFrame(looper);
    }

    looper();
}

