"use strict";

var mastervolume_disabled = false;
var mastervolume_current_volume = 1.0;
var /**@type {Map<HTMLMediaElement, null>} */ mastervolume_mediaelements = new Map();

function mastervolume_add_mediaelement(/**@type {HTMLMediaElement}*/ mediaelement) {
    if (mastervolume_mediaelements.has(mediaelement)) return;

    mediaelement["volume__original"] = 1.0;
    mediaelement.volume = mastervolume_current_volume;

    mastervolume_mediaelements.set(mediaelement, null);
}

function mastervolume_remove_mediaelement(/**@type {HTMLMediaElement}*/ mediaelement) {
    mastervolume_mediaelements.delete(mediaelement);
}

const MASTERVOLUME_PADDING = 6;
const MASTERVOLUME_BAR_GAP = 4;
const MASTERVOLUME_BAR_WIDTH = 8;
const MASTERVOLUME_BAR_HEIGHT = 24;
const MASTERVOLUME_LEVELS = 10;
const MASTERVOLUME_LABEL_HEIGHT = 16;

const MASTERVOLUME_LABEL_STRING = "VOLUME";
const MASTERVOLUME_HIDE_DELAY = 1000.0;
const MASTERVOLUME_PRESS_DELAY = 100.0;
const MASTERVOLUME_STEPS = 100 / MASTERVOLUME_LEVELS;

const MASTERVOLUME_LABEL_Y_OFFSET = MASTERVOLUME_BAR_HEIGHT + (MASTERVOLUME_PADDING * 2.0);
const MASTERVOLUME_BACK_WIDTH = (MASTERVOLUME_PADDING * 2) + (MASTERVOLUME_LEVELS * MASTERVOLUME_BAR_WIDTH) + (MASTERVOLUME_BAR_GAP * (MASTERVOLUME_LEVELS - 1));
const MASTERVOLUME_BACK_HEIGHT = (MASTERVOLUME_PADDING * 3) + MASTERVOLUME_LABEL_HEIGHT + MASTERVOLUME_BAR_HEIGHT;
const MASTERVOLUME_BACK_OFFSET_X = (FUNKIN_SCREEN_RESOLUTION_WIDTH - MASTERVOLUME_BACK_WIDTH) / 2;
const MASTERVOLUME_BACK_OFFSET_Y = 0;

const MASTERVOLUME_BACK_COLOR = [0.0, 0.0, 0.0, 0.5];
const MASTERVOLUME_LABEL_COLOR = [1.0, 1.0, 1.0, 1.0];
const MASTERVOLUME_BAR_COLOR1 = [1.0, 1.0, 1.0, 1.0];
const MASTERVOLUME_BAR_COLOR2 = [0.5, 0.5, 0.5, 1.0];

var mastervolume_label_x_offset;
var mastervolume_label_font;
var mastervolume_is_visible;
var mastervolume_full_alpha;
var mastervolume_hide_timestamp;
//var mastervolume_antibounce_timestamp;
var mastervolume_beep;
var mastervolume_volume = 100;

async function mastervolume_init() {
    mastervolume_label_font = await fonttype_init("/assets/common/font/vcr.ttf");

    let width = fonttype_measure(
        mastervolume_label_font,
        MASTERVOLUME_LABEL_HEIGHT, MASTERVOLUME_LABEL_STRING, 0, MASTERVOLUME_LABEL_STRING.length
    );

    mastervolume_is_visible = false;
    mastervolume_label_x_offset = (FUNKIN_SCREEN_RESOLUTION_WIDTH - width) / 2.0;
    mastervolume_beep = await soundplayer_init("/assets/common/sound/volume_beep.ogg");
    mastervolume_hide_timestamp = 0.0;
    //mastervolume_antibounce_timestamp = 0.0;

    mastervolume_volume = SETTINGS.storage_load_number(SETTINGS.INI_MISC_SECTION, "master_volume", 100);
    mastervolume_current_volume = mastervolume_volume / 100;

    // check if necessary warn the player if muted
    if (mastervolume_volume < 1) {
        mastervolume_is_visible = true;
        mastervolume_hide_timestamp = timer_ms_gettime64() + (MASTERVOLUME_HIDE_DELAY * 3.0);
    }
}

function mastervolume_volume_step(up_or_down) {
    if (mastervolume_disabled) return;

    let now = timer_ms_gettime64();

    //if (now < mastervolume_antibounce_timestamp) return;
    //mastervolume_antibounce_timestamp += MASTERVOLUME_PRESS_DELAY;

    let new_volume = mastervolume_volume;

    if (up_or_down)
        new_volume += MASTERVOLUME_STEPS;
    else
        new_volume -= MASTERVOLUME_STEPS;

    new_volume = math2d_clamp_int(new_volume, 0, 100);
    if (new_volume != mastervolume_volume) {
        mastervolume_current_volume = new_volume / 100;
        for (const [mediaelement, _] of mastervolume_mediaelements) {
            mediaelement.volume = mediaelement["volume__original"] * mastervolume_current_volume;
        }

        if (mastervolume_beep) soundplayer_replay(mastervolume_beep);
    }

    mastervolume_volume = new_volume;
    mastervolume_full_alpha = mastervolume_is_visible;
    mastervolume_is_visible = true;
    mastervolume_hide_timestamp = now + MASTERVOLUME_HIDE_DELAY;
}

function mastervolume_draw(/**@type {PVRContext}*/pvrctx) {
    if (!mastervolume_is_visible) return;

    pvr_context_save(pvrctx);

    let x = MASTERVOLUME_BACK_OFFSET_X + MASTERVOLUME_PADDING;
    let y = MASTERVOLUME_BACK_OFFSET_Y + MASTERVOLUME_PADDING;
    let scale_x = pvrctx.screen_width / FUNKIN_SCREEN_RESOLUTION_WIDTH;
    let scale_y = pvrctx.screen_height / FUNKIN_SCREEN_RESOLUTION_HEIGHT;
    let matrix = pvrctx.current_matrix;

    //
    // low-level rendering
    //

    // adjust volume control to viewport
    sh4matrix_scale(matrix, scale_x, scale_y);

    // render background
    pvr_context_set_vertex_alpha(pvrctx, mastervolume_full_alpha ? 1.0 : MASTERVOLUME_BACK_COLOR[3]);
    pvr_context_draw_solid_color(
        pvrctx,
        MASTERVOLUME_BACK_COLOR,
        MASTERVOLUME_BACK_OFFSET_X, MASTERVOLUME_BACK_OFFSET_Y,
        MASTERVOLUME_BACK_WIDTH, MASTERVOLUME_BACK_HEIGHT
    );

    // render bars
    let master_volume = mastervolume_current_volume * 100;
    for (let i = 0; i < MASTERVOLUME_LEVELS; i++) {
        let volume = MASTERVOLUME_STEPS * i;
        let color = volume < master_volume ? MASTERVOLUME_BAR_COLOR1 : MASTERVOLUME_BAR_COLOR2;

        volume += MASTERVOLUME_STEPS;
        if (volume > 100) volume = 100;
        let bar_height = MASTERVOLUME_BAR_HEIGHT * (volume / 100.0);

        pvr_context_set_vertex_alpha(pvrctx, color[3]);
        pvr_context_draw_solid_color(
            pvrctx,
            color,
            x, y + (MASTERVOLUME_BAR_HEIGHT - bar_height),
            MASTERVOLUME_BAR_WIDTH, bar_height
        );

        x += MASTERVOLUME_BAR_WIDTH + MASTERVOLUME_BAR_GAP;
    }

    // render text
    if (mastervolume_label_font) {
        let label_color = MASTERVOLUME_LABEL_COLOR;
        fonttype_enable_border(mastervolume_label_font, false);
        //fonttype_enable_color_by_difference(mastervolume_label_font, false);
        fonttype_set_alpha(mastervolume_label_font, label_color[3]);
        fonttype_set_color(mastervolume_label_font, label_color[0], label_color[1], label_color[2]);
        fonttype_draw_text(mastervolume_label_font,
            pvrctx,
            MASTERVOLUME_LABEL_HEIGHT,
            mastervolume_label_x_offset, MASTERVOLUME_LABEL_Y_OFFSET,
            0, MASTERVOLUME_LABEL_STRING.length, MASTERVOLUME_LABEL_STRING
        );
    }


    // check if time to hide the volume control
    let now = timer_ms_gettime64();
    if (now >= mastervolume_hide_timestamp) {
        // hide volume control
        mastervolume_is_visible = false;

        // flush engine settings to ini file ONLY IF the settings menu is not active
        if (!settingsmenu_is_running) {
            SETTINGS.storage_save_number(SETTINGS.INI_MISC_SECTION, "master_volume", master_volume);
        }
    }

    pvr_context_restore(pvrctx);
}

