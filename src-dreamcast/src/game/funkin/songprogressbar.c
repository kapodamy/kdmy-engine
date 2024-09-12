#include "game/funkin/songprogressbar.h"

#include "beatwatcher.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "fs.h"
#include "game/funkin/playerstats.h"
#include "imgutils.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "pvrcontext.h"
#include "songplayer.h"
#include "statesprite.h"
#include "stringbuilder.h"
#include "textsprite.h"


struct SongProgressbar_s {
    SongPlayer songplayer;
    float64 duration;
    Drawable drawable;
    bool manual_update;
    bool show_time;
    bool show_time_elapsed;
    AnimSprite drawable_animation;
    float64 last_elapsed_seconds;
    StringBuilder stringbuilder;
    bool is_vertical;
    float bar_length;
    StateSprite statesprite_background;
    StateSprite statesprite_back;
    StateSprite statesprite_progress;
    TextSprite textsprite_time;
};


static const char* SONGPROGRESSBAR_MODEL_BACKGROUND = "/assets/common/image/week-round/songprogressbar_background.xml";
static const char* SONGPROGRESSBAR_MODEL_BAR = "/assets/common/image/week-round/songprogressbar_bar.xml";
static const char* SONGPROGRESSBAR_NAME_BACKGROUND = "songprogressbar background";
static const char* SONGPROGRESSBAR_NAME_BAR_BACK = "songprogressbar bar back";
static const char* SONGPROGRESSBAR_NAME_BAR_FRONT = "songprogressbar bar front";


static void songprogressbar_internal_calc_state(StateSprite statesprite, const char* modelholder_src, const char* name, float width, float height);
static void songprogressbar_internal_seconds_to_string(StringBuilder stringbuilder, float64 seconds);


SongProgressbar songprogressbar_init(float x, float y, float z, float width, float height, Align align, float border_size, bool is_vertical, bool show_time, FontHolder fontholder, float font_size, float font_border_size, uint32_t color_rgba8_text, uint32_t color_rgba8_background, uint32_t color_rgba8_back, uint32_t color_rgba8_progress) {
    SongProgressbar songprogressbar = malloc_chk(sizeof(struct SongProgressbar_s));
    malloc_assert(songprogressbar, SongProgressbar);

    *songprogressbar = (struct SongProgressbar_s){
        .songplayer = NULL,
        .duration = DOUBLE_NaN,
        .drawable = NULL,
        .manual_update = false,
        .show_time = show_time,
        .show_time_elapsed = true,
        .drawable_animation = NULL,
        .last_elapsed_seconds = DOUBLE_NaN,
        .stringbuilder = stringbuilder_init(16),

        .is_vertical = is_vertical,
        .bar_length = is_vertical ? height : width,

        .statesprite_background = statesprite_init_from_vertex_color(color_rgba8_background >> 8),
        .statesprite_back = statesprite_init_from_vertex_color(color_rgba8_back >> 8),
        .statesprite_progress = statesprite_init_from_vertex_color(color_rgba8_progress >> 8),
        .textsprite_time = textsprite_init2(fontholder, font_size, color_rgba8_text >> 8),
    };

    songprogressbar->drawable = drawable_init(
        z, songprogressbar, (DelegateDraw)songprogressbar_draw, (DelegateAnimate)songprogressbar_animate
    );

    // set alphas
    textsprite_set_alpha(songprogressbar->textsprite_time, (color_rgba8_text & 0xFF) / 255.0f);
    statesprite_set_alpha(songprogressbar->statesprite_background, (color_rgba8_background & 0xFF) / 255.0f);
    statesprite_set_alpha(songprogressbar->statesprite_back, (color_rgba8_back & 0xFF) / 255.0f);
    statesprite_set_alpha(songprogressbar->statesprite_progress, (color_rgba8_progress & 0xFF) / 255.0f);

    // guess border color (invert font color)
    RGBA rgba = {0.0f, 0.0f, 0.0f, 0.0f};
    math2d_color_bytes_to_floats(color_rgba8_text, true, rgba);
    textsprite_border_set_color(songprogressbar->textsprite_time, 1.0 - rgba[0], 1.0 - rgba[1], 1.0 - rgba[2], rgba[3]);
    textsprite_border_set_size(songprogressbar->textsprite_time, font_border_size);
    textsprite_border_enable(songprogressbar->textsprite_time, true);

    // background location&size
    float background_x = x - border_size;
    float background_y = y - border_size;
    float background_width = width + (border_size * 2.0f);
    float background_height = height + (border_size * 2.0f);
    statesprite_set_draw_location(songprogressbar->statesprite_background, background_x, background_y);
    statesprite_set_draw_size(songprogressbar->statesprite_background, background_width, background_height);

    // bar back+progress location&size
    statesprite_set_draw_location(songprogressbar->statesprite_back, x, y);
    statesprite_set_draw_size(songprogressbar->statesprite_back, width, height);
    statesprite_set_draw_location(songprogressbar->statesprite_progress, x, y);
    statesprite_set_draw_size(songprogressbar->statesprite_progress, width, height);


    // set textsprite bounds
    if (is_vertical) {
        textsprite_set_paragraph_align(songprogressbar->textsprite_time, align);
        textsprite_set_align(songprogressbar->textsprite_time, align, ALIGN_CENTER);
    } else {
        textsprite_set_paragraph_align(songprogressbar->textsprite_time, align);
        textsprite_set_align(songprogressbar->textsprite_time, ALIGN_CENTER, align);
    }
    textsprite_set_draw_location(songprogressbar->textsprite_time, x - border_size, y - border_size);
    border_size *= 2.0;
    textsprite_set_max_draw_size(songprogressbar->textsprite_time, width + border_size, height + border_size);

    statesprite_crop_enable(songprogressbar->statesprite_progress, true);

    //
    // import models (if exists)
    //
    songprogressbar_internal_calc_state(
        songprogressbar->statesprite_background, SONGPROGRESSBAR_MODEL_BACKGROUND,
        SONGPROGRESSBAR_NAME_BACKGROUND, background_width, background_height
    );
    songprogressbar_internal_calc_state(
        songprogressbar->statesprite_back, SONGPROGRESSBAR_MODEL_BAR,
        SONGPROGRESSBAR_NAME_BAR_BACK, width, height
    );
    songprogressbar_internal_calc_state(
        songprogressbar->statesprite_back, SONGPROGRESSBAR_MODEL_BAR,
        SONGPROGRESSBAR_NAME_BAR_FRONT, width, height
    );

    return songprogressbar;
}

void songprogressbar_destroy(SongProgressbar* songprogressbar_ptr) {
    if (!songprogressbar_ptr || !*songprogressbar_ptr) return;

    SongProgressbar songprogressbar = *songprogressbar_ptr;

    statesprite_destroy(&songprogressbar->statesprite_background);
    statesprite_destroy(&songprogressbar->statesprite_back);
    statesprite_destroy(&songprogressbar->statesprite_progress);
    textsprite_destroy(&songprogressbar->textsprite_time);

    if (songprogressbar->drawable_animation) animsprite_destroy(&songprogressbar->drawable_animation);
    drawable_destroy(&songprogressbar->drawable);
    stringbuilder_destroy(&songprogressbar->stringbuilder);

    luascript_drop_shared(&songprogressbar);

    free_chk(songprogressbar);
    *songprogressbar_ptr = NULL;
}


void songprogressbar_set_songplayer(SongProgressbar songprogressbar, SongPlayer songplayer) {
    songprogressbar->songplayer = songplayer;
}

void songprogressbar_set_duration(SongProgressbar songprogressbar, float64 duration) {
    songprogressbar->duration = duration;
}

Drawable songprogressbar_get_drawable(SongProgressbar songprogressbar) {
    return songprogressbar->drawable;
}

void songprogressbar_set_visible(SongProgressbar songprogressbar, bool visible) {
    drawable_set_visible(songprogressbar->drawable, visible);
}

void songprogressbar_set_background_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    statesprite_set_vertex_color(songprogressbar->statesprite_background, r, g, b);
    if (!math2d_is_float_NaN(a)) statesprite_set_alpha(songprogressbar->statesprite_background, a);
}

void songprogressbar_set_bar_back_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    statesprite_set_vertex_color(songprogressbar->statesprite_back, r, g, b);
    if (!math2d_is_float_NaN(a)) statesprite_set_alpha(songprogressbar->statesprite_back, a);
}

void songprogressbar_set_bar_progress_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    statesprite_set_vertex_color(songprogressbar->statesprite_progress, r, g, b);
    if (!math2d_is_float_NaN(a)) statesprite_set_alpha(songprogressbar->statesprite_progress, a);
}

void songprogressbar_set_text_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    textsprite_set_color(songprogressbar->textsprite_time, r, g, b);
    if (!math2d_is_float_NaN(a)) textsprite_set_alpha(songprogressbar->textsprite_time, a);
}

void songprogressbar_hide_time(SongProgressbar songprogressbar, bool hidden) {
    songprogressbar->show_time = !hidden;
}

void songprogressbar_show_elapsed(SongProgressbar songprogressbar, bool elapsed_or_remain_time) {
    songprogressbar->show_time_elapsed = elapsed_or_remain_time;
}

void songprogressbar_manual_update_enable(SongProgressbar songprogressbar, bool enabled) {
    songprogressbar->manual_update = enabled;
    songprogressbar->last_elapsed_seconds = DOUBLE_NaN;
}

void songprogressbar_manual_set_text(SongProgressbar songprogressbar, const char* text) {
    songprogressbar->last_elapsed_seconds = DOUBLE_NaN; // force update
    textsprite_set_text(songprogressbar->textsprite_time, text);
}

float songprogressbar_manual_set_position(SongProgressbar songprogressbar, float64 elapsed, float64 duration, bool should_update_time_text) {
    float64 percent = elapsed / duration;
    if (math2d_is_double_NaN(percent)) percent = 0.0;

    if (should_update_time_text) {
        float64 elapsed_seconds = float64_trunc(elapsed / 1000.0);
        if (elapsed_seconds != songprogressbar->last_elapsed_seconds) {
            songprogressbar->last_elapsed_seconds = elapsed_seconds;

            if (!songprogressbar->show_time_elapsed) elapsed_seconds = (duration / 1000.0) - elapsed_seconds;
            elapsed_seconds = math2d_clamp_double(elapsed_seconds, 0.0, duration);

            // compute text
            songprogressbar_internal_seconds_to_string(songprogressbar->stringbuilder, elapsed_seconds);
            const char* const* str = stringbuilder_intern(songprogressbar->stringbuilder);

            textsprite_set_text_intern(songprogressbar->textsprite_time, true, str);
        }
    }

    // calculate bar cropping
    float length = (float)(songprogressbar->bar_length * math2d_clamp_double(percent, 0.0, 1.0));
    float crop_width = -1.0f, crop_height = -1.0f;

    if (songprogressbar->is_vertical)
        crop_height = length;
    else
        crop_width = length;

    statesprite_crop(songprogressbar->statesprite_progress, 0.0f, 0.0f, crop_width, crop_height);

    return (float)percent;
}

void songprogressbar_animation_set(SongProgressbar songprogressbar, AnimSprite animsprite) {
    if (songprogressbar->drawable_animation) animsprite_destroy(&songprogressbar->drawable_animation);
    songprogressbar->drawable_animation = animsprite ? animsprite_clone(animsprite) : NULL;
}

void songprogressbar_animation_restart(SongProgressbar songprogressbar) {
    if (songprogressbar->drawable_animation) animsprite_restart(songprogressbar->drawable_animation);
    statesprite_animation_restart(songprogressbar->statesprite_background);
    statesprite_animation_restart(songprogressbar->statesprite_back);
    statesprite_animation_restart(songprogressbar->statesprite_progress);
    textsprite_animation_restart(songprogressbar->textsprite_time);
}

void songprogressbar_animation_end(SongProgressbar songprogressbar) {
    if (songprogressbar->drawable_animation) {
        animsprite_force_end(songprogressbar->drawable_animation);
        animsprite_update_drawable(songprogressbar->drawable_animation, songprogressbar->drawable, true);
    }
    statesprite_animation_end(songprogressbar->statesprite_background);
    statesprite_animation_end(songprogressbar->statesprite_back);
    statesprite_animation_end(songprogressbar->statesprite_progress);
    textsprite_animation_end(songprogressbar->textsprite_time);
}


int32_t songprogressbar_animate(SongProgressbar songprogressbar, float elapsed) {
    int32_t completed = 0;

    if (songprogressbar->drawable_animation) {
        completed += animsprite_animate(songprogressbar->drawable_animation, elapsed);
        if (!completed) animsprite_update_drawable(songprogressbar->drawable_animation, songprogressbar->drawable, true);
    }

    completed += statesprite_animate(songprogressbar->statesprite_background, elapsed);
    completed += statesprite_animate(songprogressbar->statesprite_back, elapsed);
    completed += statesprite_animate(songprogressbar->statesprite_progress, elapsed);
    completed += textsprite_animate(songprogressbar->textsprite_time, elapsed);

    if (!songprogressbar->manual_update) {
        float64 current = 0.0, duration = 0.0;

        if (songprogressbar->songplayer) {
            current = songplayer_get_timestamp(songprogressbar->songplayer);
            duration = songprogressbar->duration >= 0.0 ? songprogressbar->duration : songplayer_get_duration(songprogressbar->songplayer);
        } else {
            current = beatwatcher_global_timestamp; // peek global beatwatcher time
            duration = songprogressbar->duration;
        }
        songprogressbar_manual_set_position(songprogressbar, current, duration, true);
    }

    return completed;
}

void songprogressbar_draw(SongProgressbar songprogressbar, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(songprogressbar->drawable, pvrctx);

    statesprite_draw(songprogressbar->statesprite_background, pvrctx);
    statesprite_draw(songprogressbar->statesprite_back, pvrctx);
    statesprite_draw(songprogressbar->statesprite_progress, pvrctx);
    if (songprogressbar->show_time) textsprite_draw(songprogressbar->textsprite_time, pvrctx);

    pvr_context_restore(pvrctx);
}


static void songprogressbar_internal_calc_state(StateSprite statesprite, const char* modelholder_src, const char* name, float width, float height) {
    if (!fs_file_exists(modelholder_src)) return;

    ModelHolder modelholder = modelholder_init(modelholder_src);
    if (!modelholder) return;

    StateSpriteState* statesprite_state = statesprite_state_add(statesprite, modelholder, name, NULL);
    modelholder_destroy(&modelholder);

    if (!statesprite_state) return;

    imgutils_calc_rectangle_in_statesprite_state(0.0f, 0.0f, width, height, ALIGN_CENTER, ALIGN_CENTER, statesprite_state);
    statesprite_state_toggle(statesprite, NULL);
}

static void songprogressbar_internal_seconds_to_string(StringBuilder stringbuilder, float64 seconds) {
    const char* TIME_SECONDS = "$is";
    const char* TIME_MINUTES = "$im";
    const char* TIME_HOURS = "$ih";

    stringbuilder_clear(stringbuilder);

    if (math2d_is_double_NaN(seconds)) {
        stringbuilder_add(stringbuilder, "--m--s");
        return;
    }

    float64 h = float64_floor(seconds / 3600.0);
    float64 m = float64_floor((seconds - (h * 3600.0)) / 60.0);
    float64 s = seconds - (h * 3600.0) - (m * 60.0);

    //
    // The 0x30 codepoint means the number zero "0" character
    //

    if (h > 0.0) {
        if (h < 10.0) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
        stringbuilder_add_format(stringbuilder, TIME_HOURS, (int32_t)h);
    }

    if (m < 10.0) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
    stringbuilder_add_format(stringbuilder, TIME_MINUTES, (int32_t)m);

    if (s < 10.0) stringbuilder_add_char_codepoint(stringbuilder, 0x30);
    stringbuilder_add_format(stringbuilder, TIME_SECONDS, (int32_t)s);
}
