"use strict";

////////////////////////
// Note: some various are functions are reused from soundplayer_*
///////////////////////

async function videoplayer_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    if (!await fs_file_exists(full_path)) { return null; }

    const gl = pvr_context.webopengl.gl;

    let gl_texture = gl.createTexture();
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, gl_texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);

    let video = document.createElement("video");
    video.playsInline = true;
    video.preload = "metadata";
    video.src = await io_native_get_path(full_path);

    let videoplayer = {
        handler: video,
        texture: null,
        sprite: sprite_init_from_rgb8(0x000000),
        fade_id: 0,
        fade_status: FADING_NONE,
        last_time: -1
    };

    try {
        await new Promise(function (resolve, reject) {
            let first_seek = true;

            video.onseeked = function (evt) {
                if (!videoplayer.texture) {
                    return;
                }
                if (first_seek) {
                    // anti-bounce for "oncanplay" callback
                    first_seek = false;
                    return;
                }

                //
                // in theory calling videoplayer_internal_update_texture() should be enough
                // but in the reality does not work, at least in firefox where the following
                // error is shown " Driver error unexpected by WebGL: 0x0502".
                //
                // Here a canvas is created to pick the current video frame to later update
                // the webgl texture.
                //
                const canvas = document.createElement("canvas");
                canvas.width = video.videoWidth;
                canvas.height = video.videoHeight;

                const ctx = canvas.getContext("2d", { alpha: false, desynchronized: false, willReadFrequently: false });
                ctx.drawImage(video, 0, 0);

                // now update texture using a canvas
                videoplayer_internal_update_texture(videoplayer, canvas);
            }
            video.oncanplay = function (evt) {
                video.oncanplay = null;
                video.onerror = null;

                const width = video.videoWidth, height = video.videoHeight;

                if (width > 0 && height > 0) {
                    const size = width * height * 4/*RGBA*/;
                    const texture = texture_init_from_raw(gl_texture, size, 1, width, height, width, height);

                    videoplayer.texture = texture;
                    videoplayer_internal_update_texture(videoplayer, video);
                    sprite_set_texture(videoplayer.sprite, texture, true);
                } else {
                    gl.deleteTexture(gl_texture);
                }

                mastervolume_add_mediaelement(video);

                resolve();
            };
            video.onerror = function (evt) {
                video.oncanplay = null;
                video.onerror = null;
                video.onseeked = null;
                reject(this.error);
            }
        });
    } catch (e) {
        console.error("videoplayer_init() failed to load: " + src, e);
        return null;
    }

    return videoplayer;
}

function videoplayer_destroy(videoplayer) {
    videoplayer.handler.pause();
    videoplayer.handler.src = null;
    videoplayer.handler = undefined;
    texture_destroy(videoplayer.texture);

    mastervolume_remove_mediaelement(videoplayer.handler);

    luascript_drop_shared(videoplayer);
    videoplayer = undefined;
}

function videoplayer_get_sprite(videoplayer) {
    return videoplayer.sprite;
}

var videoplayer_replay = soundplayer_replay;

var videoplayer_play = soundplayer_play;

var videoplayer_pause = soundplayer_pause;

var videoplayer_stop = soundplayer_stop;

var videoplayer_loop_enable = soundplayer_loop_enable;

var videoplayer_fade_audio = soundplayer_fade;

var videoplayer_set_volume = soundplayer_set_volume;

var videoplayer_set_mute = soundplayer_set_mute;

function videoplayer_seek(videoplayer, timestamp) {
    soundplayer_seek(videoplayer, timestamp);
    videoplayer.last_time = videoplayer.handler.currentTime;
}

function videoplayer_set_property(videoplayer, property_id, value) {
    switch (parseInt(property_id)) {
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

    videoplayer.last_time = time;

    // do texture update
    videoplayer_internal_update_texture(videoplayer, video);
}

function videoplayer_internal_update_texture(videoplayer, /**@type {TexImageSource}*/src) {
    const gl = pvr_context.webopengl.gl;

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, videoplayer.texture.data_vram);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, src);
    gl.bindTexture(gl.TEXTURE_2D, null);
}
