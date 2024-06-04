"use strict";

////////////////////////
// Note: some various are functions are reused from soundplayer_*
///////////////////////

async function videoplayer_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    if (!await fs_file_exists(full_path)) { return null; }

    let video = document.createElement("video");
    video.playsInline = true;
    video.preload = "metadata";
    video.src = await io_get_absolute_path(full_path);

    try {
        await new Promise(function (resolve, reject) {
            video.oncanplay = function (evt) {
                video.currentTime = 0;
                mastervolume_add_mediaelement(video);
                video.oncanplay = null;
                video.onerror = null;
                resolve();
            };
            video.onerror = function (evt) {
                video.oncanplay = null;
                video.onerror = null;
                reject(this.error);
            }
        });
    } catch (e) {
        console.error("videoplayer_init() failed to load: " + src, e);
        return null;
    }

    const gl = pvr_context.webopengl.gl;
    const width = video.videoWidth, height = video.videoHeight;

    let size = width * height * 4/*RGBA*/;
    let texture = null;

    if (width > 0 && height > 0) {
        let gl_texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, gl_texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.bindTexture(gl.TEXTURE_2D, null);

        texture = texture_init_from_raw(gl_texture, size, 1, width, height, width, height);
    }

    let sprite = sprite_init_from_rgb8(0x000000);
    sprite_set_draw_size(sprite, width, height);
    if (texture) sprite_set_texture(sprite, texture, 0);

    return {
        handler: video,
        texture: texture,
        sprite: sprite,
        fade_id: 0,
        fade_status: FADING_NONE,
        last_time: -1
    };
}

function videoplayer_destroy(videoplayer) {
    videoplayer.handler.pause();
    videoplayer.handler.src = null;
    videoplayer.handler = undefined;
    texture_destroy(videoplayer.texture);

    mastervolume_remove_mediaelement(videoplayer.handler);

    ModuleLuaScript.kdmyEngine_drop_shared_object(videoplayer);
    videoplayer = undefined;
}

function videoplayer_get_sprite(videoplayer) {
    return videoplayer.sprite;
}

var videoplayer_replay = soundplayer_replay;

var videoplayer_play = soundplayer_play;

var videoplayer_pause = soundplayer_pause;

function videoplayer_stop(videoplayer) {
    soundplayer_stop(videoplayer);
    videoplayer.last_time = NaN;
}

var videoplayer_loop_enable = soundplayer_loop_enable;

var videoplayer_fade_audio = soundplayer_fade;

var videoplayer_set_volume = soundplayer_set_volume;

var videoplayer_set_mute = soundplayer_set_mute;

function videoplayer_seek(videoplayer, timestamp) {
    soundplayer_seek(videoplayer, timestamp);
    videoplayer.last_time = NaN;
}

function videoplayer_set_property(videoplayer, property_id, value) {
    switch (property_id) {
        case MEDIA_PROP_VOLUME:
        case MEDIA_PROP_SEEK:
        case MEDIA_PROP_PLAYBACK:
            soundplayer_set_property(videoplayer, property_id, value);
            break;
        default:
            sprite_set_property(videoplayer.sprite, property_id, value);
            break;
    }
}

var videoplayer_is_muted = soundplayer_is_muted;

var videoplayer_is_playing = soundplayer_is_playing;

var videoplayer_has_fading_audio = soundplayer_has_fading;

var videoplayer_get_duration = soundplayer_get_duration;

var videoplayer_get_position = soundplayer_get_position;

var videoplayer_has_ended = soundplayer_has_ended;

function videoplayer_has_video_track(videoplayer) {
    return videoplayer.texture != null;
}

function videoplayer_has_audio_track(videoplayer) {
    const handler = videoplayer.handler;
    return handler.mozHasAudio || Boolean(handler.webkitAudioDecodedByteCount) || Boolean(handler.audioTracks?.length);
}

function videoplayer_poll_streams(videoplayer) {
    const video = videoplayer.handler;

    if (!videoplayer.texture) return;

    // check if time to show the next frame
    const time = video.currentTime;
    if (time == videoplayer.last_time) return;

    if (videoplayer.last_time < 0) {
        // force seeking to adquire the first frame of the video
        video.currentTime = 0.001;
    }

    // do texture update
    const gl = pvr_context.webopengl.gl;
    const width = video.videoWidth, height = video.videoHeight;

    gl.bindTexture(gl.TEXTURE_2D, videoplayer.texture.data_vram);
    gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, video);
    gl.bindTexture(gl.TEXTURE_2D, null);

    videoplayer.last_time = time;
}

