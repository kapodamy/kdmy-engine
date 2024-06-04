"use strict";

const SONGPROGRESSBAR_MODEL_BACKGROUND = "/assets/common/image/week-round/songprogressbar_background.xml";
const SONGPROGRESSBAR_MODEL_BAR = "/assets/common/image/week-round/songprogressbar_bar.xml";
const SONGPROGRESSBAR_NAME_BACKGROUND = "songprogressbar background";
const SONGPROGRESSBAR_NAME_BAR_BACK = "songprogressbar bar back";
const SONGPROGRESSBAR_NAME_BAR_FRONT = "songprogressbar bar front";

async function songprogressbar_init(x, y, z, width, height, align, border_size, is_vertical, show_time, fontholder, font_size, font_border_size, color_rgba8_text, color_rgba8_background, color_rgba8_back, color_rgba8_progress) {
    let songprogressbar = {
        songplayer: null,
        duration: NaN,
        drawable: null,
        manual_update: 0,
        show_time: show_time,
        show_time_elapsed: 1,
        drawable_animation: null,
        last_elapsed_seconds: NaN,
        stringbuilder: stringbuilder_init(16),

        is_vertical: is_vertical,
        bar_length: is_vertical ? height : width,

        statesprite_background: statesprite_init_from_vertex_color(color_rgba8_background >> 8),
        statesprite_back: statesprite_init_from_vertex_color(color_rgba8_back >> 8),
        statesprite_progress: statesprite_init_from_vertex_color(color_rgba8_progress >> 8),
        textsprite_time: textsprite_init2(fontholder, font_size, color_rgba8_text >> 8),
    };

    songprogressbar.drawable = drawable_init(
        z, songprogressbar, songprogressbar_draw, songprogressbar_animate
    );

    // set alphas
    textsprite_set_alpha(songprogressbar.textsprite_time, (color_rgba8_text & 0xFF) / 0xFF);
    statesprite_set_alpha(songprogressbar.statesprite_background, (color_rgba8_background & 0xFF) / 0xFF);
    statesprite_set_alpha(songprogressbar.statesprite_back, (color_rgba8_back & 0xFF) / 0xFF);
    statesprite_set_alpha(songprogressbar.statesprite_progress, (color_rgba8_progress & 0xFF) / 0xFF);

    // guess border color (invert font color)
    const rgba = [0.0, 0.0, 0.0, 0.0];
    math2d_color_bytes_to_floats(color_rgba8_text, 1, rgba);
    textsprite_border_set_color(songprogressbar.textsprite_time, 1.0 - rgba[0], 1.0 - rgba[1], 1.0 - rgba[2], rgba[3]);
    textsprite_border_set_size(songprogressbar.textsprite_time, font_border_size);
    textsprite_border_enable(songprogressbar.textsprite_time, 1);

    // background location&size
    let background_x = x - border_size;
    let background_y = y - border_size;
    let background_width = width + (border_size * 2);
    let background_height = height + (border_size * 2);
    statesprite_set_draw_location(songprogressbar.statesprite_background, background_x, background_y);
    statesprite_set_draw_size(songprogressbar.statesprite_background, background_width, background_height);

    // bar back+progress location&size
    statesprite_set_draw_location(songprogressbar.statesprite_back, x, y);
    statesprite_set_draw_size(songprogressbar.statesprite_back, width, height);
    statesprite_set_draw_location(songprogressbar.statesprite_progress, x, y);
    statesprite_set_draw_size(songprogressbar.statesprite_progress, width, height);


    // set textsprite bounds
    if (is_vertical) {
        textsprite_set_paragraph_align(songprogressbar.textsprite_time, align);
        textsprite_set_align(songprogressbar.textsprite_time, align, ALIGN_CENTER);
    } else {
        textsprite_set_paragraph_align(songprogressbar.textsprite_time, align);
        textsprite_set_align(songprogressbar.textsprite_time, ALIGN_CENTER, align);
    }
    textsprite_set_draw_location(songprogressbar.textsprite_time, x - border_size, y - border_size);
    border_size *= 2.0;
    textsprite_set_max_draw_size(songprogressbar.textsprite_time, width + border_size, height + border_size);

    statesprite_crop_enable(songprogressbar.statesprite_progress, 1);

    //
    // import models (if exists)
    //
    await songprogressbar_internal_calc_state(
        songprogressbar.statesprite_background, SONGPROGRESSBAR_MODEL_BACKGROUND,
        SONGPROGRESSBAR_NAME_BACKGROUND, background_width, background_height
    );
    await songprogressbar_internal_calc_state(
        songprogressbar.statesprite_back, SONGPROGRESSBAR_MODEL_BAR,
        SONGPROGRESSBAR_NAME_BAR_BACK, width, height
    );
    await songprogressbar_internal_calc_state(
        songprogressbar.statesprite_back, SONGPROGRESSBAR_MODEL_BAR,
        SONGPROGRESSBAR_NAME_BAR_FRONT, width, height
    );

    return songprogressbar;
}

function songprogressbar_destroy(songprogressbar) {
    statesprite_destroy(songprogressbar.statesprite_background);
    statesprite_destroy(songprogressbar.statesprite_back);
    statesprite_destroy(songprogressbar.statesprite_progress);
    textsprite_destroy(songprogressbar.textsprite_time);

    if (songprogressbar.drawable_animation) animsprite_destroy(songprogressbar.drawable_animation);
    drawable_destroy(songprogressbar.drawable);
    stringbuilder_destroy(songprogressbar.stringbuilder);

    ModuleLuaScript.kdmyEngine_drop_shared_object(songprogressbar);
    songprogressbar = undefined;
}


function songprogressbar_set_songplayer(songprogressbar, songplayer) {
    songprogressbar.songplayer = songplayer;
}

function songprogressbar_set_duration(songprogressbar, duration) {
    songprogressbar.duration = duration;
}

function songprogressbar_get_drawable(songprogressbar) {
    return songprogressbar.drawable;
}

function songprogressbar_set_visible(songprogressbar, visible) {
    drawable_set_visible(songprogressbar.drawable, visible);
}

function songprogressbar_set_background_color(songprogressbar, r, g, b, a) {
    statesprite_set_vertex_color(songprogressbar.statesprite_background, r, g, b);
    if (!Number.isNaN(a)) statesprite_set_alpha(songprogressbar.statesprite_background, a);
}

function songprogressbar_set_bar_back_color(songprogressbar, r, g, b, a) {
    statesprite_set_vertex_color(songprogressbar.statesprite_back, r, g, b);
    if (!Number.isNaN(a)) statesprite_set_alpha(songprogressbar.statesprite_back, a);
}

function songprogressbar_set_bar_progress_color(songprogressbar, r, g, b, a) {
    statesprite_set_vertex_color(songprogressbar.statesprite_progress, r, g, b);
    if (!Number.isNaN(a)) statesprite_set_alpha(songprogressbar.statesprite_progress, a);
}

function songprogressbar_set_text_color(songprogressbar, r, g, b, a) {
    textsprite_set_color(songprogressbar.textsprite_time, r, g, b);
    if (!Number.isNaN(a)) textsprite_set_alpha(songprogressbar.textsprite_time, a);
}

function songprogressbar_hide_time(songprogressbar, hidden) {
    songprogressbar.show_time = !hidden;
}

function songprogressbar_show_elapsed(songprogressbar, elapsed_or_remain_time) {
    songprogressbar.show_time_elapsed = !!elapsed_or_remain_time;
}

function songprogressbar_manual_update_enable(songprogressbar, enabled) {
    songprogressbar.manual_update = !!enabled;
    songprogressbar.last_elapsed_seconds = NaN;
}

function songprogressbar_manual_set_text(songprogressbar, text) {
    songprogressbar.last_elapsed_seconds = NaN;// force update
    textsprite_set_text(songprogressbar.textsprite_time, text);
}

function songprogressbar_manual_set_position(songprogressbar, elapsed, duration, should_update_time_text) {
    let percent = elapsed / duration;
    if (Number.isNaN(percent)) percent = 0.0;

    if (should_update_time_text) {
        let elapsed_seconds = Math.trunc(elapsed / 1000.0);
        if (elapsed_seconds != songprogressbar.last_elapsed_seconds) {
            songprogressbar.last_elapsed_seconds = elapsed_seconds;

            if (!songprogressbar.show_time_elapsed) elapsed_seconds = (duration / 1000.0) - elapsed_seconds;
            elapsed_seconds = math2d_clamp_double(elapsed_seconds, 0.0, duration);

            // compute text
            songprogressbar_internal_seconds_to_string(songprogressbar.stringbuilder, elapsed_seconds);
            let str = stringbuilder_intern(songprogressbar.stringbuilder);

            textsprite_set_text_intern(songprogressbar.textsprite_time, 1, str);
        }
    }

    // calculate bar cropping
    let length = songprogressbar.bar_length * math2d_clamp_double(percent, 0.0, 1.0);
    let crop_width = -1.0, crop_height = -1.0;

    if (songprogressbar.is_vertical)
        crop_height = length;
    else
        crop_width = length;

    statesprite_crop(songprogressbar.statesprite_progress, 0, 0, crop_width, crop_height);

    return percent;
}

function songprogressbar_animation_set(songprogressbar, animsprite) {
    if (songprogressbar.drawable_animation) animsprite_destroy(songprogressbar.drawable_animation);
    songprogressbar.drawable_animation = animsprite ? animsprite_clone(animsprite) : null;
}

function songprogressbar_animation_restart(songprogressbar) {
    if (songprogressbar.drawable_animation) animsprite_restart(songprogressbar.drawable_animation);
    statesprite_animation_restart(songprogressbar.statesprite_background);
    statesprite_animation_restart(songprogressbar.statesprite_back);
    statesprite_animation_restart(songprogressbar.statesprite_progress);
    textsprite_animation_restart(songprogressbar.textsprite_time);
}

function songprogressbar_animation_end(songprogressbar) {
    if (songprogressbar.drawable_animation) {
        animsprite_force_end(songprogressbar.drawable_animation);
        animsprite_update_drawable(songprogressbar.drawable_animation, songprogressbar.drawable, 1);
    }
    statesprite_animation_end(songprogressbar.statesprite_background);
    statesprite_animation_end(songprogressbar.statesprite_back);
    statesprite_animation_end(songprogressbar.statesprite_progress);
    textsprite_animation_end(songprogressbar.textsprite_time);
}


function songprogressbar_animate(songprogressbar, elapsed) {
    let completed = 0;

    if (songprogressbar.drawable_animation) {
        completed += animsprite_animate(songprogressbar.drawable_animation, elapsed);
        if (!completed) animsprite_update_drawable(songprogressbar.drawable_animation, songprogressbar.drawable, 1);
    }

    completed += statesprite_animate(songprogressbar.statesprite_background, elapsed);
    completed += statesprite_animate(songprogressbar.statesprite_back, elapsed);
    completed += statesprite_animate(songprogressbar.statesprite_progress, elapsed);
    completed += textsprite_animate(songprogressbar.textsprite_time, elapsed);

    if (!songprogressbar.manual_update) {
        let current = 0, duration = 0;

        if (songprogressbar.songplayer) {
            current = songplayer_get_timestamp(songprogressbar.songplayer);
            duration = songprogressbar.duration >= 0.0 ? songprogressbar.duration : songplayer_get_duration(songprogressbar.songplayer);
        } else {
            current = beatwatcher_global_timestamp;// peek global beatwatcher time
            duration = songprogressbar.duration;
        }
        songprogressbar_manual_set_position(songprogressbar, current, duration, 1);
    }

    return completed;
}

function songprogressbar_draw(songprogressbar, pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(songprogressbar.drawable, pvrctx);

    statesprite_draw(songprogressbar.statesprite_background, pvrctx);
    statesprite_draw(songprogressbar.statesprite_back, pvrctx);
    statesprite_draw(songprogressbar.statesprite_progress, pvrctx);
    if (songprogressbar.show_time) textsprite_draw(songprogressbar.textsprite_time, pvrctx);

    pvr_context_restore(pvrctx);
}


async function songprogressbar_internal_calc_state(statesprite, modelholder_src, name, width, height) {
    if (!await fs_file_exists(modelholder_src)) return;

    let modelholder = await modelholder_init(modelholder_src);
    if (!modelholder) return;

    let statesprite_state = statesprite_state_add(statesprite, modelholder, name, null);
    modelholder_destroy(modelholder);

    if (!statesprite_state) return;

    imgutils_calc_rectangle_in_statesprite_state(0, 0, width, height, ALIGN_CENTER, ALIGN_CENTER, statesprite_state);
    statesprite_state_toggle(statesprite, null);
}

function songprogressbar_internal_seconds_to_string(stringbuilder, seconds) {
    const TIME_SECONDS = "$is";
    const TIME_MINUTES = "$im";
    const TIME_HOURS = "$ih";

    if (Number.isNaN(seconds)) {
        stringbuilder_clear(stringbuilder);
        stringbuilder_add("--m--s");
        return;
    }

    let h = Math.floor(seconds / 3600.0);
    let m = Math.floor((seconds - (h * 3600.0)) / 60.0);
    let s = seconds - (h * 3600.0) - (m * 60.0);

    stringbuilder_clear(stringbuilder);

    if (h > 0.0) {
        if (h < 10) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
        stringbuilder_add_format(stringbuilder, TIME_HOURS, Math.trunc(h));
    }

    if (m < 10) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
    stringbuilder_add_format(stringbuilder, TIME_MINUTES, Math.trunc(m));

    if (s < 10) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
    stringbuilder_add_format(stringbuilder, TIME_SECONDS, Math.trunc(s));

}

