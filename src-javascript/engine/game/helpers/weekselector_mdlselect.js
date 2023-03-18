"use strict";

const WEEKSELECTOR_MDLSELECT_HEY = "choosen";
const WEEKSELECTOR_MDLSELECT_IDLE = "idle";

const WEEKSELECTOR_MDLSELECT_UI_BF_LABEL = "ui_character_selector_player";
const WEEKSELECTOR_MDLSELECT_UI_GF_LABEL = "ui_character_selector_girlfriend";
const WEEKSELECTOR_MDLSELECT_UI_BF_PREVIEW = "ui_character_player";
const WEEKSELECTOR_MDLSELECT_UI_GF_PREVIEW = "ui_character_girlfriend";

const WEEKSELECTOR_MDLSELECT_MODELS_BF = "/assets/common/data/boyfriend_models.json";
const WEEKSELECTOR_MDLSELECT_MODELS_GF = "/assets/common/data/girlfriend_models.json";


async function weekselector_mdlselect_init(animlist, modelholder, layout, texturepool, is_boyfriend) {
    const draw_size = [0, 0];

    let name;
    let mdlselect = {
        animsprite: animsprite_init_from_animlist(animlist, WEEKSELECTOR_ARROW_SPRITE_NAME),

        index: -1,
        list: null,
        list_size: 0,

        preview: statesprite_init_from_texture(null),

        icon_locked: modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, 1),
        icon_up: modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, 1),
        icon_down: modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, 1),

        has_up: 1,
        has_down: 1,
        is_locked: 0,
        is_enabled: 0,
        is_boyfriend: is_boyfriend,
        hey_playing: 0,

        load_thread_id: 0,

        drawable: null,
        drawable_character: null,
        placeholder_character: null,
        label: null,

        beatwatcher: {},

        texturepool: texturepool
    };

    beatwatcher_reset(mdlselect.beatwatcher, 1, 100);

    mdlselect.drawable = drawable_init(
        -1, mdlselect, weekselector_mdlselect_draw, weekselector_mdlselect_animate
    );
    mdlselect.drawable_character = drawable_init(
        -1, mdlselect.preview, statesprite_draw, statesprite_animate
    );

    sprite_set_visible(mdlselect.icon_locked, 0);
    sprite_set_visible(mdlselect.icon_up, 0);
    sprite_set_visible(mdlselect.icon_down, 0);

    name = is_boyfriend ? WEEKSELECTOR_MDLSELECT_UI_BF_PREVIEW : WEEKSELECTOR_MDLSELECT_UI_GF_PREVIEW;
    let placeholder_character = layout_get_placeholder(layout, name);
    console.assert(placeholder_character, "missing :" + name);

    let index = 0;

    let models = is_boyfriend ? WEEKSELECTOR_MDLSELECT_MODELS_BF : WEEKSELECTOR_MDLSELECT_MODELS_GF;
    let models_array = await json_load_from(models);
    let models_array_size = json_read_array_length(models_array);

    // count all character models
    for (let i = 0; i < models_array_size; i++) {
        let json_obj = json_read_array_item_object(models_array, i);
        let unlock_directive = json_read_string(json_obj, "unlockDirectiveName", null);
        let hide_if_locked = json_read_boolean(json_obj, "hideIfLocked", false);
        let is_locked = !funkinsave_contains_unlock_directive(unlock_directive);
        unlock_directive = undefined;

        if (is_locked && hide_if_locked) continue;
        index++;
    }
    for (let i = 0; i < weeks_array.size; i++) {
        let array, array_size;

        if (is_boyfriend) {
            array = weeks_array.array[i].unlockables.boyfriend_models;
            array_size = weeks_array.array[i].unlockables.boyfriend_models_size;
        } else {
            array = weeks_array.array[i].unlockables.girlfriend_models;
            array_size = weeks_array.array[i].unlockables.girlfriend_models_size;
        }

        for (let j = 0; j < array_size; j++) {
            let is_locked = !funkinsave_contains_unlock_directive(array[j].unlock_directive);
            if (is_locked && array[j].hide_if_locked) continue;
            index++;
        }
    }

    // build list
    mdlselect.list_size = index;
    mdlselect.list = new Array(mdlselect.list_size);
    index = 0;
    for (let i = 0; i < models_array_size; i++) {
        let json_obj = json_read_array_item_object(models_array, i);
        let unlock_directive = json_read_string(json_obj, "unlockDirectiveName", null);
        let hide_if_locked = json_read_boolean(json_obj, "hideIfLocked", false);
        let is_locked = !funkinsave_contains_unlock_directive(unlock_directive);

        if (is_locked && hide_if_locked) {
            unlock_directive = undefined;
            continue;
        }

        // get path of the manifest file
        let manifest = json_read_string(json_obj, "manifest", null);
        if (!manifest) {
            throw new Error(`weekselector_mdlselect_init() invalid/missing 'manifest' in ${models}`);
        }
        let manifest_path = fs_build_path2(models, manifest);
        manifest = undefined;

        mdlselect.list[index] = {
            name: json_read_string(json_obj, "name", null),
            is_locked: is_locked,
            imported: 1,
            week_selector_left_facing: 0,
            week_selector_enable_beat: 0,
            week_selector_model: null,
            week_selector_idle_anim_name: null,
            week_selector_choosen_anim_name: null,
            manifest: manifest_path
        };
        await weekselector_mdlselect_helper_import_from_manifest(
            manifest_path, mdlselect.list[index]
        );
        index++;

    }
    json_destroy(models_array);
    for (let i = 0; i < weeks_array.size; i++) {
        let array, array_size;

        if (is_boyfriend) {
            array = weeks_array.array[i].unlockables.boyfriend_models;
            array_size = weeks_array.array[i].unlockables.boyfriend_models_size;
        } else {
            array = weeks_array.array[i].unlockables.girlfriend_models;
            array_size = weeks_array.array[i].unlockables.girlfriend_models_size;
        }

        for (let j = 0; j < array_size; j++) {
            let is_locked = !funkinsave_contains_unlock_directive(array[j].unlock_directive);
            if (is_locked && array[j].hide_if_locked) continue;

            mdlselect.list[index] = {
                name: array[j].name,
                is_locked: is_locked,
                imported: 0,
                week_selector_left_facing: 0,
                week_selector_enable_beat: 0,
                week_selector_model: null,
                week_selector_idle_anim_name: null,
                week_selector_choosen_anim_name: null,
                manifest: array[j].manifest
            };
            await weekselector_mdlselect_helper_import_from_manifest(
                array[j].manifest, mdlselect.list[index]
            );
            index++;
        }
    }

    let arrows_height = layout_get_attached_value(
        layout, "ui_character_selector_arrowsHeight", LAYOUT_TYPE_FLOAT, 0
    );
    let font_size = layout_get_attached_value(
        layout, "ui_character_selector_fontSize", LAYOUT_TYPE_FLOAT, 18
    );
    let font_name = layout_get_attached_value(
        layout, "ui_character_selector_fontName", LAYOUT_TYPE_STRING, null
    );
    let font_color = layout_get_attached_value(
        layout, "ui_character_selector_fontColor", LAYOUT_TYPE_HEX, 0x00FFFF
    );

    name = is_boyfriend ? WEEKSELECTOR_MDLSELECT_UI_BF_LABEL : WEEKSELECTOR_MDLSELECT_UI_GF_LABEL;
    let placeholder = layout_get_placeholder(layout, name);
    console.assert(placeholder, "missing :" + name);

    // resize&rotate&locate arrows
    weekselector_mdlselect_internal_place_arrow(mdlselect.icon_up, arrows_height, placeholder, 0);
    weekselector_mdlselect_internal_place_arrow(mdlselect.icon_down, arrows_height, placeholder, 1);

    sprite_resize_draw_size(
        mdlselect.icon_locked,
        placeholder.width, placeholder.height - arrows_height * 1.5, draw_size
    );
    sprite_center_draw_location(
        mdlselect.icon_locked,
        placeholder.x, placeholder.y,
        placeholder.width, placeholder.height,
        null
    );

    mdlselect.label = textsprite_init2(
        layout_get_attached_font(layout, font_name), font_size, font_color
    );
    textsprite_set_draw_location(mdlselect.label, placeholder.x, placeholder.y);
    textsprite_set_max_draw_size(mdlselect.label, placeholder.width, placeholder.height);
    textsprite_set_align(mdlselect.label, ALIGN_CENTER, ALIGN_CENTER);
    textsprite_set_visible(mdlselect.label, 0);

    mdlselect.placeholder_character = placeholder_character;
    placeholder_character.vertex = mdlselect.drawable_character;
    placeholder.vertex = mdlselect.drawable;
    drawable_helper_update_from_placeholder(
        mdlselect.drawable_character, placeholder_character
    );
    drawable_helper_update_from_placeholder(mdlselect.drawable, placeholder);

    drawable_set_visible(mdlselect.drawable_character, 0);
    statesprite_set_draw_location(mdlselect.preview, placeholder_character.x, placeholder_character.y);
    statesprite_set_draw_size(
        mdlselect.preview, placeholder_character.width, placeholder_character.height
    );
    statesprite_state_add2(mdlselect.preview, null, null, null, 0XFFFFFF, null);

    weekselector_mdlselect_select(mdlselect, 0);

    return mdlselect;
}

function weekselector_mdlselect_destroy(mdlselect) {
    if (mdlselect.animsprite) animsprite_destroy(mdlselect.animsprite);

    drawable_destroy(mdlselect.drawable);
    drawable_destroy(mdlselect.drawable_character);

    statesprite_destroy(mdlselect.preview);
    textsprite_destroy(mdlselect.label);

    sprite_destroy_full(mdlselect.icon_locked);
    sprite_destroy_full(mdlselect.icon_up);
    sprite_destroy_full(mdlselect.icon_down);

    for (let i = 0; i < mdlselect.list_size; i++) {
        if (mdlselect.list[i].imported) {
            mdlselect.list[i].name = undefined;
            mdlselect.list[i].manifest = undefined;
        }
        mdlselect.list[i].week_selector_model = undefined;
        mdlselect.list[i].week_selector_idle_anim_name = undefined;
        mdlselect.list[i].week_selector_choosen_anim_name = undefined;
    }

    mdlselect.list = undefined;
    mdlselect = undefined;
}


function weekselector_mdlselect_draw(mdlselect, pvrctx) {
    pvr_context_save(pvrctx);

    if (mdlselect.is_locked) sprite_draw(mdlselect.icon_locked, pvrctx);

    if (mdlselect.is_enabled) {
        sprite_draw(mdlselect.icon_up, pvrctx);
        sprite_draw(mdlselect.icon_down, pvrctx);
    }

    if (mdlselect.is_locked)
        pvr_context_set_global_alpha(pvrctx, WEEKSELECTOR_PREVIEW_DISABLED_ALPHA);

    textsprite_draw(mdlselect.label, pvrctx);

    pvr_context_restore(pvrctx);
}

function weekselector_mdlselect_animate(mdlselect, elapsed) {
    if (mdlselect.animsprite) animsprite_animate(mdlselect.animsprite, elapsed);

    sprite_animate(mdlselect.icon_up, elapsed);
    sprite_animate(mdlselect.icon_down, elapsed);

    if (!mdlselect.hey_playing && beatwatcher_poll(mdlselect.beatwatcher)) {
        elapsed = mdlselect.beatwatcher.since;
        if (statesprite_animation_completed(mdlselect.preview)) {
            statesprite_animation_restart(mdlselect.preview);
        }
    }

    statesprite_animate(mdlselect.preview, elapsed);

    if (mdlselect.has_up)
        animsprite_update_sprite(mdlselect.animsprite, mdlselect.icon_up, 0);
    if (mdlselect.has_down)
        animsprite_update_sprite(mdlselect.animsprite, mdlselect.icon_down, 1);

    return 0;
}


function weekselector_mdlselect_get_manifest(mdlselect) {
    if (mdlselect.index < 0 || mdlselect.index >= mdlselect.list_size) return null;
    let mdl = mdlselect.list[mdlselect.index];
    return mdl.is_locked ? null : strdup(mdl.manifest);
}

function weekselector_mdlselect_is_selected_locked(mdlselect) {
    if (mdlselect.index < 0 || mdlselect.index >= mdlselect.list_size) return 1;
    return mdlselect.list[mdlselect.index].is_locked;
}

function weekselector_mdlselect_toggle_choosen(mdlselect) {
    mdlselect.hey_playing = statesprite_state_toggle(mdlselect.preview, WEEKSELECTOR_MDLSELECT_HEY);
}

function weekselector_mdlselect_enable_arrows(mdlselect, enabled) {
    mdlselect.is_enabled = enabled;
}

function weekselector_mdlselect_select_default(mdlselect) {
    if (mdlselect.index == 0) return;
    weekselector_mdlselect_select(mdlselect, 0);
}

function weekselector_mdlselect_select(mdlselect, new_index) {
    if (new_index < 0 || new_index >= mdlselect.list_size) return 0;
    if (new_index == mdlselect.index) return 1;

    mdlselect.is_locked = mdlselect.list[new_index].is_locked;
    mdlselect.index = new_index;
    mdlselect.has_up = new_index > 0;
    mdlselect.has_down = new_index < (mdlselect.list_size - 1);
    mdlselect.load_thread_id++;

    statesprite_state_toggle(mdlselect.preview, null);
    statesprite_state_remove(mdlselect.preview, WEEKSELECTOR_MDLSELECT_IDLE);
    statesprite_state_remove(mdlselect.preview, WEEKSELECTOR_MDLSELECT_HEY);
    drawable_set_visible(mdlselect.drawable_character, 0);
    textsprite_set_text_intern(mdlselect.label, 1, mdlselect.list[new_index].name);

    sprite_set_alpha(
        mdlselect.icon_up, mdlselect.has_up ? 1.0 : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );
    sprite_set_alpha(
        mdlselect.icon_down, mdlselect.has_down ? 1.0 : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );

    thd_helper_spawn(weekselector_mdlselect_internal_load_async, mdlselect);

    return 1;
}

function weekselector_mdlselect_scroll(mdlselect, offset) {
    return weekselector_mdlselect_select(mdlselect, mdlselect.index + offset);
}

function weekselector_mdlselect_get_drawable(mdlselect) {
    return mdlselect.drawable;
}

function weekselector_mdlselect_set_beats(mdlselect, bpm) {
    beatwatcher_change_bpm(mdlselect.beatwatcher, bpm);
}


function weekselector_mdlselect_internal_place_arrow(sprite, arrow_size, placeholder, end) {
    const draw_size = [0, 0];

    sprite_resize_draw_size(sprite, arrow_size, -1, draw_size);

    // note: the sprite is rotated (corner rotation)
    let x = placeholder.x + ((placeholder.width - draw_size[1]) / 2.0);
    let y = placeholder.y;
    if (end) y += placeholder.height - draw_size[0];

    sprite_set_draw_location(sprite, x, y);
    sprite_matrix_corner_rotation(sprite, end ? CORNER_TOPRIGHT : CORNER_BOTTOMLEFT);
}

async function weekselector_mdlselect_internal_load_async(mdlselect) {
    // remember the statesprite id to check if "weekselector_mdlselect" was disposed
    let id = mdlselect.preview.id;
    let load_thread_id = mdlselect.load_thread_id;
    let character_info = mdlselect.list[mdlselect.index];
    let week_selector_left_facing = character_info.week_selector_left_facing;
    let week_selector_enable_beat = character_info.week_selector_enable_beat;
    let flip_x = 0;

    if (mdlselect.is_boyfriend && !week_selector_left_facing) flip_x = 1;
    //else if (!mdlselect.is_boyfriend && week_selector_left_facing) flip_x = 1;

    let modelholder = await modelholder_init(character_info.week_selector_model);

    if (!STATESPRITE_POOL.has(id) || load_thread_id != mdlselect.load_thread_id) {
        if (modelholder) modelholder_destroy(modelholder);
        return null;
    }

    if (!modelholder) {
        console.error("weekselector_mdlselect_internal_load_async() modelholder_init failed");
        return null;
    }

    weekselector_mdlselect_helper_import(
        mdlselect.preview, modelholder,
        mdlselect.placeholder_character,
        week_selector_enable_beat,
        character_info.week_selector_idle_anim_name ?? WEEKSELECTOR_MDLSELECT_IDLE,
        WEEKSELECTOR_MDLSELECT_IDLE
    );
    weekselector_mdlselect_helper_import(
        mdlselect.preview, modelholder,
        mdlselect.placeholder_character,
        0,
        character_info.week_selector_choosen_anim_name ?? WEEKSELECTOR_MDLSELECT_HEY,
        WEEKSELECTOR_MDLSELECT_HEY
    );
    modelholder_destroy(modelholder);

    texturepool_add(mdlselect.texturepool, modelholder_get_texture(modelholder, 0));

    statesprite_state_toggle(mdlselect.preview, WEEKSELECTOR_MDLSELECT_IDLE);
    statesprite_animate(mdlselect.preview, beatwatcher_remaining_until_next(mdlselect.beatwatcher));
    statesprite_flip_texture(mdlselect.preview, flip_x, 0);
    drawable_set_visible(mdlselect.drawable_character, 1);

    return null;
}

function weekselector_mdlselect_helper_import(statesprite, mdlhldr, placeholder, enable_beat, anim_name, name) {
    if (!anim_name) return;
    let statesprite_state = statesprite_state_add(statesprite, mdlhldr, anim_name, name);
    if (!statesprite_state) return;

    imgutils_calc_rectangle_in_statesprite_state(
        0,
        0,
        placeholder.width,
        placeholder.height,
        placeholder.align_horizontal,
        placeholder.align_vertical,
        statesprite_state
    );

    if (!statesprite_state.animation) return;

    if (enable_beat || name == WEEKSELECTOR_MDLSELECT_HEY) {
        animsprite_set_loop(statesprite_state.animation, 1);
    }
}

async function weekselector_mdlselect_helper_import_from_manifest(src, list_item) {
    let charactermanifest = await charactermanifest_init(src, 0);
    list_item.week_selector_left_facing = charactermanifest.week_selector_left_facing;
    list_item.week_selector_enable_beat = charactermanifest.week_selector_enable_beat;
    list_item.week_selector_model = charactermanifest.week_selector_model;
    list_item.week_selector_idle_anim_name = charactermanifest.week_selector_idle_anim_name;
    list_item.week_selector_choosen_anim_name = charactermanifest.week_selector_choosen_anim_name;
    charactermanifest_destroy(charactermanifest);
}

