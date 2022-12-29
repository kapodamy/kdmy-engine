"use strict";


const DIALOGUE_IDLE = "idle";
const DIALOGUE_SPEAK = "speak";
const DIALOGUE_ICON = "icon";
const DIALOGUE_OPEN = "open";
//const DIALOGUE_LAYOUT = "/assets/common/image/dialogue/layout.xml";
//const DIALOGUE_LAYOUT_DREAMCAST = "/assets/common/image/dialogue/layout~dreamcast.xml";
const DIALOGUE_DEFAULTS_DEFINITIONS = "/assets/common/data/dialogue_defaults.xml";



const DIALOGUE_TYPE_AUDIO_PLAY = 0;
const DIALOGUE_TYPE_AUDIO_PAUSE = 1;
const DIALOGUE_TYPE_AUDIO_FADEIN = 2;
const DIALOGUE_TYPE_AUDIO_FADEOUT = 3;
const DIALOGUE_TYPE_AUDIO_STOP = 4;
const DIALOGUE_TYPE_AUDIO_VOLUME = 5;
const DIALOGUE_TYPE_BACKGROUND_SET = 6;
const DIALOGUE_TYPE_BACKGROUND_CHANGE = 7;
const DIALOGUE_TYPE_BACKGROUND_REMOVE = 8;
const DIALOGUE_TYPE_LUA = 9;
const DIALOGUE_TYPE_EXIT = 10;
const DIALOGUE_TYPE_PORTRAIT_ADD = 11;
const DIALOGUE_TYPE_PORTRAIT_REMOVE = 12;
const DIALOGUE_TYPE_PORTRAIT_REMOVEALL = 13;
const DIALOGUE_TYPE_AUDIO_UI = 14;
const DIALOGUE_TYPE_TEXT_SPEED = 15;
const DIALOGUE_TYPE_TEXT_SKIP = 16;
const DIALOGUE_TYPE_TEXT_INSTANTPRINT = 17;
const DIALOGUE_TYPE_SPEECH_BACKGROUND = 18;
const DIALOGUE_TYPE_TEXT_FONT = 19;
const DIALOGUE_TYPE_TEXT_COLOR = 20;
const DIALOGUE_TYPE_TEXT_BORDERCOLOR = 21;
const DIALOGUE_TYPE_TEXT_BORDEROFFSET = 22;
const DIALOGUE_TYPE_TEXT_PARAGRAPHSPACE = 23;
const DIALOGUE_TYPE_TEXT_SIZE = 24;
const DIALOGUE_TYPE_TEXT_ALIGN = 25;
const DIALOGUE_TYPE_RUNMULTIPLECHOICE = 26;
const DIALOGUE_TYPE_TITLE = 27;
const DIALOGUE_TYPE_NOWAIT = 28;
const DIALOGUE_TYPE_TEXT_BORDERSIZE = 29;
const DIALOGUE_TYPE_TEXT_BORDERENABLE = 30;

const DIALOGUE_REPEATANIM_NONE = 0;
const DIALOGUE_REPEATANIM_ONCE = 1;
const DIALOGUE_REPEATANIM_WHILESPEAKS = 2;
const DIALOGUE_REPEATANIM_ALWAYS = 3;



async function dialogue_init(src) {
    let viewport_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    let viewport_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;

    src = fs_get_full_path(src);
    let xml = null;
    try {
        xml = await fs_readxml(src);
    } catch (e) {
        console.error(e);
    }
    if (!xml || !xml.documentElement) {
        if (xml) xml = undefined;
        console.error("dialogue_init() can not load dialogue xml file: " + src);
        src = undefined;
        return null;
    }

    let xml_root = xml.documentElement;
    let animlist = await animlist_init("/assets/common/anims/dialogue-ui.xml");
    let audios = arraylist_init();
    let fonts = arraylist_init();
    let backgrounds = arraylist_init();
    let portraits = arraylist_init();
    let dialogs = arraylist_init();
    let states = arraylist_init();
    let multiplechoices = arraylist_init();
    let speechimages = arraylist_init();

    const anims_ui = {
        portrait_left_in: null,
        portrait_center_in: null,
        portrait_right_in: null,
        portrait_left_out: null,
        portrait_center_out: null,
        portrait_right_out: null,
        background_in: null,
        background_out: null
    };

    if (animlist) {
        anims_ui.portrait_left_in = animsprite_init_from_animlist(animlist, "portrait-add-left");
        anims_ui.portrait_center_in = animsprite_init_from_animlist(animlist, "portrait-add-center");
        anims_ui.portrait_right_in = animsprite_init_from_animlist(animlist, "portrait-add-right");
        anims_ui.portrait_left_out = animsprite_init_from_animlist(animlist, "portrait-remove-left");
        anims_ui.portrait_center_out = animsprite_init_from_animlist(animlist, "portrait-remove-center");
        anims_ui.portrait_right_out = animsprite_init_from_animlist(animlist, "portrait-remove-right");
        anims_ui.background_in = animsprite_init_from_animlist(animlist, "background-in");
        anims_ui.background_out = animsprite_init_from_animlist(animlist, "background-out");
        anims_ui.open = animsprite_init_from_animlist(animlist, "open");
        anims_ui.close = animsprite_init_from_animlist(animlist, "close");
        animlist_destroy(animlist);
    }

    fs_folder_stack_push();
    fs_set_working_folder(src, 1);
    src = undefined;

    let import_defaults = vertexprops_parse_boolean(xml_root, "importDefaultDefinition", 1);
    let xml_defaults = null;
    let childs_defaults = null;

    // import default character portraits and speech images
    if (import_defaults) {
        try {
            xml_defaults = await fs_readxml(DIALOGUE_DEFAULTS_DEFINITIONS);
        } catch (e) {
            console.error(e);
        }

        if (xml_defaults && xml_defaults.documentElement) {
            childs_defaults = xml_defaults.documentElement.children;
        }
    }

    let index = 0;
    let self_parse = 1;
    let childs = xml_root.children;

    while (true) {
        let node;

        // parse first the desired xml, and later the default xml
        if (self_parse) {
            if (index >= childs.length) {
                if (childs_defaults == null) break;
                index = 0;
                self_parse = 0;
                continue;
            }
            node = childs[index++];
        } else {
            if (index >= childs_defaults.length) break;
            node = childs_defaults[index++];
        }

        switch (node.tagName) {
            case "Definition":
                for (let node2 of node.children) {
                    switch (node2.tagName) {
                        case "AudioList":
                            await dialogue_internal_parse_audiolist(node2, audios);
                            break;
                        case "BackgroundList":
                            await dialogue_internal_parse_backgroundlist(node2, viewport_width, viewport_height, backgrounds);
                            break;
                        case "PortraitList":
                            await dialogue_internal_parse_portraitlist(node2, portraits);
                            break;
                        case "AnimationsUI":
                            await dialogue_internal_parse_animationui(node2, anims_ui);
                            break;
                        case "Font":
                            await dialogue_internal_parse_font(node2, fonts);
                            break;
                        case "SpeechImageList":
                            await dialogue_internal_parse_speechimagelist(node2, speechimages);
                            break;
                        case "ImportPortraitList":
                            await dialogue_internal_parse_importportraitlist(node2, portraits);
                            break;
                        default:
                            console.error("dialogue_init() unknown definition: " + node2.tagName);
                            break;
                    }
                }
                break;
            case "State":
                dialogue_internal_parse_state(node, states);
                break;
            case "MultipleChoice":
                await dialogue_internal_parse_multiplechoice(node, animlist, dialogs, multiplechoices);
                break;
        }
    }

    if (xml_defaults) xml_defaults = undefined;

    fs_folder_stack_pop();
    xml = undefined;

    let dialogue = {
        audios: null,
        audios_size: 0,
        fonts: null,
        fonts_size: 0,
        backgrounds: null,
        backgrounds_size: 0,
        portraits: null,
        portraits_size: 0,
        dialogs: null,
        dialogs_size: 0,
        states: null,
        states_size: 0,
        multiplechoices: null,
        multiplechoices_size: 0,
        speechimages: null,
        speechimages_size: 0,

        anims_ui: anims_ui,
        visible_portraits: linkedlist_init(),
        texsprite_speech: null,
        texsprite_title: null,
        draw_portraits_on_top: 0,

        current_background: -1,
        change_background_from: -1,
        script: null,
        do_exit: 0,
        click_text: null,
        click_char: null,
        char_delay: 0,
        do_skip: 0,
        do_instant_print: 0,
        do_no_wait: 0,
        do_multiplechoice: null,
        current_speechimage: null,
        current_speechimage_is_opening: 0,
        current_speechimage_repeat: DIALOGUE_REPEATANIM_ALWAYS,
        is_speaking: 0,
        current_dialog_codepoint_index: 0,
        current_dialog_codepoint_length: 0,
        current_dialog_duration: 0.0,
        current_dialog_elapsed: 0.0,
        current_dialog_mask: null,
        current_dialog_buffer: stringbuilder_init(64),
        current_dialog: null,
        current_dialog_line: -1,
        gamepad: gamepad_init(-1),
        dialog_external: null,
        is_completed: 1,
        chars_per_second: 0,
        self_drawable: null,
        self_hidden: 0
    };

    dialogue.self_drawable = drawable_init(300, dialogue, dialogue_draw, dialogue_animate);

    gamepad_set_buttons_delay(dialogue.gamepad, 200);

    arraylist_destroy2(audios, dialogue, "audios_size", "audios");
    arraylist_destroy2(fonts, dialogue, "fonts_size", "fonts");
    arraylist_destroy2(backgrounds, dialogue, "backgrounds_size", "backgrounds");
    arraylist_destroy2(portraits, dialogue, "portraits_size", "portraits");
    arraylist_destroy2(dialogs, dialogue, "dialogs_size", "dialogs");
    arraylist_destroy2(states, dialogue, "states_size", "states");
    arraylist_destroy2(multiplechoices, dialogue, "multiplechoices_size", "multiplechoices");
    arraylist_destroy2(speechimages, dialogue, "speechimages_size", "speechimages");

    // set defaults
    let tmp_audio;

    tmp_audio = dialogue_internal_get_audio(dialogue, "click_text");
    if (tmp_audio) dialogue.click_text = tmp_audio.soundplayer;

    tmp_audio = dialogue_internal_get_audio(dialogue, "click_char");
    if (tmp_audio) dialogue.click_char = tmp_audio.soundplayer;

    dialogue.current_background = dialogue_internal_get_background_index(dialogue, "default_background");
    dialogue.chars_per_second = 0;
    dialogue.char_delay = 40;

    for (let i = 0; i < dialogue.speechimages_size; i++) {
        let speechimage = dialogue.speechimages[i];
        if (speechimage.name != "normal-left") continue;

        dialogue.current_speechimage = speechimage;
        let toggle_default = statesprite_state_toggle(dialogue.current_speechimage.statesprite, null);
        let toggle_open = statesprite_state_toggle(dialogue.current_speechimage.statesprite, DIALOGUE_OPEN);
        dialogue.current_speechimage_is_opening = toggle_default || toggle_open;

        if (!dialogue.current_speechimage_is_opening) {
            // switch to idle animation
            statesprite_state_toggle(dialogue.current_speechimage.statesprite, DIALOGUE_IDLE);
        }

        break;
    }

    // create textsprite speech if not customized
    dialogue.texsprite_speech = textsprite_init(null, 0, 28.0, 0x00000);
    textsprite_set_paragraph_space(dialogue.texsprite_speech, 8.0);
    textsprite_set_wordbreak(dialogue.texsprite_speech, FONT_WORDBREAK_LOOSE);

    let fontholder = dialogue_internal_get_font(dialogue, "font");
    if (fontholder) textsprite_change_font(dialogue.texsprite_speech, fontholder);


    // create textsprite title
    dialogue.texsprite_title = textsprite_init(null, 0, 24.0, 0x00000);
    textsprite_set_paragraph_space(dialogue.texsprite_title, 8.0);
    textsprite_set_wordbreak(dialogue.texsprite_title, FONT_WORDBREAK_LOOSE);

    fontholder = dialogue_internal_get_font(dialogue, "font");
    if (fontholder) textsprite_change_font(dialogue.texsprite_speech, fontholder);

    return dialogue;
}

function dialogue_destroy(dialogue) {
    dialogue_internal_destroy_external_dialog(dialogue);

    for (let i = 0; i < dialogue.audios_size; i++) {
        dialogue.audios[i].name = undefined;
        soundplayer_destroy(dialogue.audios[i].soundplayer);
    }
    for (let i = 0; i < dialogue.fonts_size; i++) {
        dialogue.fonts[i].name = undefined;
        fontholder_destroy(dialogue.fonts[i].fontholder);
    }
    for (let i = 0; i < dialogue.backgrounds_size; i++) {
        dialogue.backgrounds[i].name = undefined;
        sprite_destroy_full(dialogue.backgrounds[i].sprite);
    }
    for (let i = 0; i < dialogue.portraits_size; i++) {
        dialogue.portraits[i].name = undefined;
        dialogue.portraits[i].random_from_prefix = undefined;
        statesprite_destroy_texture_if_stateless(dialogue.portraits[i].statesprite);
        statesprite_destroy(dialogue.portraits[i].statesprite);
    }
    for (let i = 0; i < dialogue.dialogs_size; i++) {
        for (let j = 0; j < dialogue.dialogs[i].lines_size; j++) {
            dialogue.dialogs[i].lines[j].target_state_name = undefined;
            dialogue.dialogs[i].lines[j].text = undefined;
        }
        dialogue.dialogs[i].lines = undefined;
        dialogue.dialogs[i].full_path = undefined;
    }
    for (let i = 0; i < dialogue.states_size; i++) {
        for (let j = 0; j < dialogue.states[i].actions_size; j++) {
            dialogue.states[i].actions[j].name = undefined;
            dialogue.states[i].actions[j].click_text = undefined;
            dialogue.states[i].actions[j].click_char = undefined;
            dialogue.states[i].actions[j].lua_eval = undefined;
            dialogue.states[i].actions[j].lua_function = undefined;
            dialogue.states[i].actions[j].random_from_prefix = undefined;
            dialogue.states[i].actions[j].speech_name = undefined;
            dialogue.states[i].actions[j].title = undefined;
        }
        dialogue.states[i].name = undefined;
        dialogue.states[i].actions = undefined;
    }
    for (let i = 0; i < dialogue.multiplechoices_size; i++) {
        for (let j = 0; j < dialogue.multiplechoices[i].choices_size; j++) {
            dialogue.multiplechoices[i].choices[j].lua_eval = undefined;
            dialogue.multiplechoices[i].choices[j].lua_function = undefined;
            dialogue.multiplechoices[i].choices[j].run_multiple_choice = undefined;
            dialogue.multiplechoices[i].choices[j].text = undefined;
        }
        sprite_destroy_full(dialogue.multiplechoices[i].hint);
        sprite_destroy_full(dialogue.multiplechoices[i].icon);
        dialogue.multiplechoices[i].state_on_leave = undefined;
        dialogue.multiplechoices[i].title = undefined;
        dialogue.multiplechoices[i].name = undefined;
        dialogue.multiplechoices[i].choices = undefined;
    }
    for (let i = 0; i < dialogue.speechimages_size; i++) {
        dialogue.speechimages[i].name = undefined;
        statesprite_destroy_texture_if_stateless(dialogue.speechimages[i].statesprite);
        statesprite_destroy(dialogue.speechimages[i].statesprite);
    }

    dialogue.audios = undefined;
    dialogue.fonts = undefined;
    dialogue.backgrounds = undefined;
    dialogue.portraits = undefined;
    dialogue.dialogs = undefined;
    dialogue.states = undefined;
    dialogue.multiplechoices = undefined;
    dialogue.speechimages = undefined;

    if (dialogue.anims_ui.portrait_left_in) animsprite_destroy(dialogue.anims_ui.portrait_left_in);
    if (dialogue.anims_ui.portrait_center_in) animsprite_destroy(dialogue.anims_ui.portrait_center_in);
    if (dialogue.anims_ui.portrait_right_in) animsprite_destroy(dialogue.anims_ui.portrait_right_in);
    if (dialogue.anims_ui.portrait_left_out) animsprite_destroy(dialogue.anims_ui.portrait_left_out);
    if (dialogue.anims_ui.portrait_center_out) animsprite_destroy(dialogue.anims_ui.portrait_center_out);
    if (dialogue.anims_ui.portrait_right_out) animsprite_destroy(dialogue.anims_ui.portrait_right_out);
    if (dialogue.anims_ui.background_in) animsprite_destroy(dialogue.anims_ui.background_in);
    if (dialogue.anims_ui.background_out) animsprite_destroy(dialogue.anims_ui.background_out);
    if (dialogue.anims_ui.open) animsprite_destroy(dialogue.anims_ui.open);
    if (dialogue.anims_ui.close) animsprite_destroy(dialogue.anims_ui.close);

    textsprite_destroy(dialogue.texsprite_speech);
    textsprite_destroy(dialogue.texsprite_title);

    linkedlist_destroy(dialogue.visible_portraits);
    gamepad_destroy(dialogue.gamepad);
    drawable_destroy(dialogue.self_drawable);
    stringbuilder_destroy(dialogue.current_dialog_buffer);
    dialogue.current_dialog_mask = undefined;

    free(dialogue);
}


function dialogue_apply_state(dialogue, state_name) {
    if (dialogue.do_exit) return 0;

    let state = dialogue_internal_get_state(dialogue, state_name);
    if (!state) return 0;

    return dialogue_internal_apply_state(dialogue, state);
}

function dialogue_animate(dialogue, elapsed) {
    if (dialogue.self_hidden || dialogue.is_completed) return 1;

    if (dialogue.anims_ui.open && animsprite_animate(dialogue.anims_ui.open, elapsed) < 1) {
        animsprite_update_drawable(dialogue.anims_ui.open, dialogue.self_drawable, 1);
    }

    let anim_portrait_left_in = !!dialogue.anims_ui.portrait_left_in;
    let anim_portrait_center_in = !!dialogue.anims_ui.portrait_center_in;
    let anim_portrait_right_in = !!dialogue.anims_ui.portrait_right_out;
    let anim_portrait_left_out = !!dialogue.anims_ui.portrait_left_out;
    let anim_portrait_center_out = !!dialogue.anims_ui.portrait_center_out;
    let anim_portrait_right_out = !!dialogue.anims_ui.portrait_right_out;

    for (let i = 0; i < dialogue.backgrounds_size; i++) {
        sprite_animate(dialogue.backgrounds[i].sprite, elapsed);
    }

    if (anim_portrait_left_in) {
        if (animsprite_animate(dialogue.anims_ui.portrait_left_in, elapsed) > 0) anim_portrait_left_in = false;
    }
    if (anim_portrait_center_in) {
        if (animsprite_animate(dialogue.anims_ui.portrait_center_in, elapsed) > 0) anim_portrait_center_in = false;
    }
    if (anim_portrait_right_in) {
        if (animsprite_animate(dialogue.anims_ui.portrait_right_in, elapsed) > 0) anim_portrait_right_in = false;
    }
    if (anim_portrait_left_out) {
        if (animsprite_animate(dialogue.anims_ui.portrait_left_out, elapsed) > 0) anim_portrait_left_out = false;
    }
    if (anim_portrait_center_out) {
        if (animsprite_animate(dialogue.anims_ui.portrait_center_out, elapsed) > 0) anim_portrait_center_out = false;
    }
    if (anim_portrait_right_out) {
        if (animsprite_animate(dialogue.anims_ui.portrait_right_out, elapsed) > 0) anim_portrait_right_out = false;
    }

    for (let portrait of linkedlist_iterate4(dialogue.visible_portraits)) {
        let exists;
        let anim;

        let completed = statesprite_animate(portrait.statesprite, elapsed);

        if (portrait.is_added) {
            // guess the correct animation direction
            if (portrait.position < 0.5) {
                exists = anim_portrait_left_in;
                anim = dialogue.anims_ui.portrait_left_in;
            } else if (portrait.position == 0.5) {
                exists = anim_portrait_center_in;
                anim = dialogue.anims_ui.portrait_center_in;
            } else if (portrait.position > 0.5) {
                exists = anim_portrait_right_in;
                anim = dialogue.anims_ui.portrait_right_in;
            } else {
                // this never should happen
                continue;
            }

            if (exists) {
                animsprite_update_statesprite(anim, portrait.statesprite, 0);
            } else {
                portrait.is_added = 0;
                let toggled_default = statesprite_state_toggle(portrait.statesprite, null);
                let toggled_speak = statesprite_state_toggle(portrait.statesprite, DIALOGUE_SPEAK);

                if (dialogue.is_speaking && (toggled_default || toggled_speak)) {
                    portrait.is_speaking = 1;
                    statesprite_animation_restart(portrait.statesprite);
                } else {
                    // no speak animation, fallback to idle
                    portrait.is_speaking = 0;
                    statesprite_state_toggle(portrait.statesprite, DIALOGUE_IDLE);
                }
            }
        } else if (portrait.is_removed) {
            // guess the correct animation direction
            if (portrait.position < 0.5) {
                exists = anim_portrait_left_out;
                anim = dialogue.anims_ui.portrait_left_out;
            } else if (portrait.position == 0.5) {
                exists = anim_portrait_center_out;
                anim = dialogue.anims_ui.portrait_center_out;
            } else if (portrait.position > 0.5) {
                exists = anim_portrait_right_out;
                anim = dialogue.anims_ui.portrait_right_out;
            } else {
                // this never should happen
                continue;
            }

            if (exists) {
                animsprite_update_statesprite(anim, portrait.statesprite, 0);
            } else {
                portrait.is_removed = 0;
                linkedlist_remove_item(dialogue.visible_portraits, portrait);
            }
        } else if (completed < 1) {
            // the animation is not completed, nothing to do
            continue;
        }

        // if the speak animation is completed and there not longer speech switch to idle
        if (!dialogue.is_speaking && portrait.is_speaking) {
            portrait.is_speaking = false;
            dialogue_internal_stop_portrait_animation(portrait);
            continue;
        }

        // check if the animation should be looped again
        if (!(portrait.is_speaking ? portrait.speak_anim_looped : portrait.idle_anim_looped)) {
            continue;
        }

        // only loop if the desired state is applied and exists
        let can_loop = portrait.is_speaking ? portrait.has_speak : portrait.has_idle;

        // restart the animation if necessary
        if (can_loop || (!portrait.has_speak && !portrait.has_idle)) {
            statesprite_animation_restart(portrait.statesprite);
        }
    }

    if (dialogue.current_background >= 0 && dialogue.anims_ui.background_in) {
        let sprite = dialogue.backgrounds[dialogue.current_background].sprite;
        if (sprite_animate(dialogue.anims_ui.background_in, elapsed) < 1) {
            animsprite_update_sprite(dialogue.anims_ui.background_in, sprite, 1);
        }
    }

    if (dialogue.change_background_from >= 0 && dialogue.anims_ui.background_out) {
        let sprite = dialogue.backgrounds[dialogue.change_background_from].sprite;
        if (sprite_animate(dialogue.anims_ui.background_out, elapsed) < 1) {
            animsprite_update_sprite(dialogue.anims_ui.background_out, sprite, 1);
        } else {
            dialogue.change_background_from = -1;
        }
    }

    if (dialogue.current_speechimage) {
        let is_opening = dialogue.current_speechimage_is_opening;
        let sprite = dialogue.current_speechimage.statesprite;

        let completed = sprite_animate(sprite, elapsed) > 0;

        // once opening animation is done, switch to idle animation
        if (completed && is_opening && statesprite_state_toggle(sprite, DIALOGUE_IDLE)) {
            statesprite_animation_restart(sprite);
            dialogue.current_speechimage_is_opening = 0;
        } else if (completed && !is_opening) {
            switch (dialogue.current_speechimage_repeat) {
                case DIALOGUE_REPEATANIM_ALWAYS:
                    statesprite_animation_restart(sprite);
                    break;
                case DIALOGUE_REPEATANIM_WHILESPEAKS:
                    if (dialogue.is_speaking) statesprite_animation_restart(sprite);
                    break;
            }
        }
    }

    if (dialogue.do_exit) {
        if (dialogue.anims_ui.close && animsprite_animate(dialogue.anims_ui.close, elapsed) < 1) {
            animsprite_update_drawable(dialogue.anims_ui.close, dialogue.self_drawable, 1);
        } else {
            if (dialogue.script) _dialogue_lua_call(dialogue.script, "luascript_call_function", "f_dialogue_exit");
            dialogue.is_completed = 1;
            for (let i = 0; i < dialogue.audios_size; i++) soundplayer_stop(dialogue.audios[i].soundplayer);
            return 0;
        }
    }

    if (dialogue.current_dialog) {
        dialogue.current_dialog_elapsed += elapsed;
        let preapare_next_line = 0;

        let buttons = gamepad_has_pressed_delayed(
            dialogue.gamepad, GAMEPAD_A | GAMEPAD_START | GAMEPAD_X | GAMEPAD_BACK
        );
        if ((buttons & (GAMEPAD_A | GAMEPAD_X)) != 0x00) {
            dialogue.current_dialog_elapsed = dialogue.current_dialog_duration;
            preapare_next_line = 1;
        } else if ((buttons & (GAMEPAD_START | GAMEPAD_BACK)) != 0x00) {
            if (dialogue.is_speaking)
                dialogue.do_no_wait = 1;
            else
                dialogue.do_exit = 1;
        }

        if (dialogue.is_speaking)
            dialogue_internal_print_text(dialogue);
        else if (!dialogue.do_exit && preapare_next_line)
            dialogue_internal_prepare_print_text(dialogue);
    }

    return 0;
}

function dialogue_draw(dialogue, pvrctx) {
    if (dialogue.self_hidden || dialogue.is_completed) return;

    pvr_context_save(pvrctx);

    // JS only
    if (!pvrctx_is_widescreen()) {
        const scale = 640 / 1280;
        const translate = (480 - (720 * scale)) / 2;
        sh4matrix_scale(pvrctx.current_matrix, scale, scale);
        sh4matrix_translate(pvrctx.current_matrix, 0, translate);
    }

    drawable_helper_apply_in_context(dialogue.self_drawable, pvrctx);
    pvr_context_save(pvrctx);

    dialogue_internal_draw_background(dialogue, pvrctx);
    if (!dialogue.draw_portraits_on_top) dialogue_internal_draw_portraits(dialogue, pvrctx);
    dialogue_internal_draw_speech(dialogue, pvrctx);
    if (dialogue.draw_portraits_on_top) dialogue_internal_draw_portraits(dialogue, pvrctx);

    pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}

function dialogue_is_completed(dialogue) {
    return dialogue.is_completed;
}

function dialogue_is_hidden(dialogue) {
    return dialogue.self_hidden;
}

async function dialogue_show_dialog(dialogue, src_dialog) {
    if (src_dialog == null) return 0;

    let dialogs = arraylist_init2(dialogue.dialogs_size);
    for (let i = 0; i < dialogue.dialogs_size; i++) arraylist_add(dialogs, dialogue.dialogs[i]);
    if (dialogue.dialog_external) arraylist_add(dialogs, dialogue.dialog_external);

    let total = arraylist_size(dialogs);
    let id = await dialogue_internal_parse_dialog(src_dialog, dialogs);
    if (id < 0) {
        arraylist_destroy(dialogs, 0);
        return 0;
    } else if (id >= total) {
        dialogue_internal_destroy_external_dialog(dialogue);
        dialogue.dialog_external = arraylist_get(dialogs, id);
        arraylist_destroy(dialogs, 0);
    }

    dialogue.current_dialog = dialogue.dialog_external;
    dialogue.do_skip = 0;
    dialogue.do_instant_print = 0;
    dialogue.do_multiplechoice = null;
    dialogue.do_exit = 0;
    dialogue.current_dialog_line = 0;
    dialogue.self_hidden = 0;
    dialogue.is_completed = 0;
    drawable_set_antialiasing(dialogue.self_drawable, PVR_FLAG_DEFAULT);
    drawable_set_alpha(dialogue.self_drawable, 1.0);
    drawable_set_offsetcolor_to_default(dialogue.self_drawable);
    pvrctx_helper_clear_modifier(drawable_get_modifier(dialogue.self_drawable));
    linkedlist_clear(dialogue.visible_portraits);

    // apply any initial state
    dialogue_apply_state(dialogue, null);
    for (let i = 0; i < dialogue.states_size; i++) {
        if (dialogue.states[i].initial) await dialogue_internal_apply_state(dialogue, dialogue.states[i]);
    }

    if (!textsprite_has_font(dialogue.texsprite_speech)) {
        console.error("dialogue_show_dialog() speech textsprite does not have font");
        dialogue.do_exit = 1;
        return 0;
    }

    dialogue_internal_prepare_print_text(dialogue);

    if (dialogue.anims_ui.open) animsprite_restart(dialogue.anims_ui.open);
    if (dialogue.anims_ui.close) animsprite_restart(dialogue.anims_ui.close);

    if (dialogue.current_speechimage == null && dialogue.speechimages_size > 0) {
        console.warn("dialogue_show_dialog() no speech background choosen, auto-choosing the first one declared");
        dialogue.current_speechimage = dialogue.speechimages[0];
    }

    return 1;
}

async function dialogue_close(dialogue) {
    if (dialogue.script) _dialogue_lua_call(dialogue.script, "luascript_call_function", "f_dialogue_closing");

    dialogue.do_exit = 1;
    dialogue.current_dialog = null;

    for (let i = 0; i < dialogue.audios_size; i++) {
        if (soundplayer_is_playing(dialogue.audios[i].soundplayer)) {
            if (dialogue.anims_ui.close)
                soundplayer_fade(dialogue.audios[i].soundplayer, 0, 500.0);
            else
                soundplayer_stop(dialogue.audios[i].soundplayer);
        }
    }
}

function dialogue_hide(dialogue, hidden) {
    dialogue.self_hidden = hidden;
}

function dialogue_suspend(dialogue) {
    if (dialogue.is_completed) return;
    for (let i = 0; i < dialogue.audios_size; i++) {
        dialogue.audios[i].was_playing = soundplayer_is_playing(dialogue.audios[i].soundplayer);
        if (dialogue.audios[i].was_playing) soundplayer_pause(dialogue.audios[i].soundplayer);
    }
}

function dialogue_resume(dialogue) {
    if (dialogue.is_completed) return;
    for (let i = 0; i < dialogue.audios_size; i++) {
        if (dialogue.audios[i].was_playing) soundplayer_play(dialogue.audios[i].soundplayer);
    }
    gamepad_clear_buttons(dialogue.gamepad);
}

function dialogue_get_modifier(dialogue) {
    return drawable_get_modifier(dialogue.self_drawable);
}

function dialogue_get_drawable(dialogue) {
    return dialogue.self_drawable;
}

function dialogue_set_offsetcolor(dialogue, r, g, b, a) {
    drawable_set_offsetcolor(dialogue.self_drawable, r, g, b, a);
}

function dialogue_set_antialiasing(dialogue, pvrflag) {
    drawable_set_antialiasing(dialogue.self_drawable, pvrflag);
}

function dialogue_set_alpha(dialogue, alpha) {
    dialogue_set_alpha(dialogue.self_drawable, alpha);
}

function dialogue_set_script(dialogue, weekscript) {
    if (weekscript)
        dialogue.script = weekscript_get_luascript(weekscript);
    else
        dialogue.script = null;
}



function dialogue_internal_apply_state(dialogue, state) {
    if (dialogue.do_exit) return 0;

    let audio;
    let background;
    let background_changed = 0;
    let speechimage_changed = 0;

    for (let action_index = 0; action_index < state.actions_size; action_index++) {
        let action = state.actions[action_index];

        switch (action.type) {
            case DIALOGUE_TYPE_AUDIO_PLAY:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) soundplayer_play(audio.soundplayer);
                break;
            case DIALOGUE_TYPE_AUDIO_PAUSE:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) {
                    soundplayer_pause(audio.soundplayer);
                } else if (action.name == null) {
                    for (let i = 0; i < dialogue.audios_size; i++) {
                        soundplayer_pause(dialogue.audios[i].soundplayer);
                    }
                }
                break;
            case DIALOGUE_TYPE_AUDIO_FADEIN:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) soundplayer_fade(audio.soundplayer, 1, action.duration);
                break;
            case DIALOGUE_TYPE_AUDIO_FADEOUT:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) soundplayer_fade(audio.soundplayer, 0, action.duration);
                break;
            case DIALOGUE_TYPE_AUDIO_STOP:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) {
                    soundplayer_stop(audio.soundplayer);
                } else if (action.name == null) {
                    for (let i = 0; i < dialogue.audios_size; i++) {
                        soundplayer_stop(dialogue.audios[i].soundplayer);
                    }
                }
                break;
            case DIALOGUE_TYPE_AUDIO_VOLUME:
                audio = dialogue_internal_get_audio(dialogue, action.name);
                if (audio) soundplayer_set_volume(audio.soundplayer, action.volume);
                break;
            case DIALOGUE_TYPE_BACKGROUND_SET:
                background = dialogue_internal_get_background_index(dialogue, action.name);
                if (background >= 0) {
                    dialogue.current_background = background;
                    dialogue.change_background_from = -1;
                    background_changed = 1;
                }
                break;
            case DIALOGUE_TYPE_BACKGROUND_CHANGE:
                dialogue.change_background_from = dialogue.current_background;
                dialogue.current_background = dialogue_internal_get_background_index(dialogue, action.name);
                background_changed = 1;
                break;
            case DIALOGUE_TYPE_BACKGROUND_REMOVE:
                dialogue.current_background = -1;
                dialogue.change_background_from = -1;
                background_changed = 1;
                break;
            case DIALOGUE_TYPE_LUA:
                if (dialogue.script) _dialogue_lua_call(dialogue.script, "luascript_eval", action.luascript_eval);
                break;
            case DIALOGUE_TYPE_EXIT:
                dialogue_close(dialogue);
                return 1;
            case DIALOGUE_TYPE_PORTRAIT_ADD:
                let portrait_index = -1;
                if (action.random_from_prefix && dialogue.portraits_size > 0) {
                    let index;
                    if (action.random_from_prefix.length == 0) {
                        // random choose
                        index = math2d_random_int(0, dialogue.portraits_size - 1);
                    } else {
                        index = -1;
                        let count = 0;
                        let j = 0;

                        for (let i = 0; i < dialogue.portraits_size; i++) {
                            if (dialogue.portraits[i].name.startsWith(action.name, 0)) count++;
                        }

                        // random choose
                        let choosen = math2d_random_int(0, count - 1);

                        for (let i = 0; i < dialogue.portraits_size; i++) {
                            if (dialogue.portraits[i].name.startsWith(action.name, 0)) {
                                if (j == choosen) {
                                    index = i;
                                    break;
                                }
                                j++;
                            }
                        }
                    }
                }
                for (let i = 0; i < dialogue.portraits_size; i++) {
                    if (dialogue.portraits[i].name == action.name) {
                        portrait_index = i;
                        break;
                    }
                }
                if (portrait_index < 0 || portrait_index >= dialogue.portraits_size) {
                    break;
                }

                if (dialogue.anims_ui.portrait_left_in) animsprite_restart(dialogue.anims_ui.portrait_left_in);
                if (dialogue.anims_ui.portrait_center_in) animsprite_restart(dialogue.anims_ui.portrait_center_in);
                if (dialogue.anims_ui.portrait_right_in) animsprite_restart(dialogue.anims_ui.portrait_right_in);

                linkedlist_remove_item(dialogue.visible_portraits, dialogue.portraits[portrait_index]);
                linkedlist_add_item(dialogue.visible_portraits, dialogue.portraits[portrait_index]);
                dialogue.portraits[portrait_index].is_added = 1;
                dialogue.portraits[portrait_index].is_removed = 0;
                dialogue.portraits[portrait_index].is_speaking = 0;
                statesprite_state_toggle(dialogue.portraits[portrait_index].statesprite, null);
                statesprite_state_toggle(
                    dialogue.portraits[portrait_index].statesprite, action.no_speak ? DIALOGUE_IDLE : DIALOGUE_SPEAK
                );
                break;
            case DIALOGUE_TYPE_PORTRAIT_REMOVE:
                for (let i = 0; i < dialogue.portraits_size; i++) {
                    if (dialogue.portraits[i].name == action.name) {
                        dialogue.portraits[i].is_added = 0;
                        dialogue.portraits[i].is_removed = 1;

                        if (dialogue.anims_ui.portrait_left_out)
                            animsprite_restart(dialogue.anims_ui.portrait_left_out);
                        if (dialogue.anims_ui.portrait_center_out)
                            animsprite_restart(dialogue.anims_ui.portrait_center_out);
                        if (dialogue.anims_ui.portrait_right_out)
                            animsprite_restart(dialogue.anims_ui.portrait_right_out);

                        break;
                    }
                }
                break;
            case DIALOGUE_TYPE_PORTRAIT_REMOVEALL:
                if (action.animate_remove) {
                    for (let portrait of linkedlist_iterate4(dialogue.visible_portraits)) {
                        portrait.is_added = false;
                        portrait.is_removed = true;
                    }

                    if (dialogue.anims_ui.portrait_left_out)
                        animsprite_restart(dialogue.anims_ui.portrait_left_out);
                    if (dialogue.anims_ui.portrait_center_out)
                        animsprite_restart(dialogue.anims_ui.portrait_center_out);
                    if (dialogue.anims_ui.portrait_right_out)
                        animsprite_restart(dialogue.anims_ui.portrait_right_out);

                } else {
                    linkedlist_clear(dialogue.visible_portraits, null);
                }
                break;
            case DIALOGUE_TYPE_AUDIO_UI:
                if (action.click_char != null) {
                    audio = dialogue_internal_get_audio(dialogue, action.name);
                    if (dialogue.click_char) soundplayer_stop(dialogue.click_char);

                    if (audio)
                        dialogue.click_char = audio.soundplayer;
                    else
                        dialogue.click_char = null;
                }
                if (action.click_text != null) {
                    audio = dialogue_internal_get_audio(dialogue, action.name);
                    if (dialogue.click_text) soundplayer_stop(dialogue.click_text);

                    if (audio)
                        dialogue.click_text = audio.soundplayer;
                    else
                        dialogue.click_text = null;
                }
                break;
            case DIALOGUE_TYPE_TEXT_SPEED:
                if (action.chars_per_second >= 0) dialogue.chars_per_second = action.chars_per_second;
                if (action.char_delay >= 0) dialogue.char_delay = action.char_delay;
                break;
            case DIALOGUE_TYPE_TEXT_SKIP:
                dialogue.do_skip = 1;
                break;
            case DIALOGUE_TYPE_TEXT_INSTANTPRINT:
                dialogue.do_instant_print = 1;
                break;
            case DIALOGUE_TYPE_SPEECH_BACKGROUND:
                if (action.name == null) {
                    //random choose
                    let index = math2d_random_int(0, dialogue.backgrounds_size - 1);
                    dialogue.current_speechimage = dialogue.speechimages[index];
                    speechimage_changed = 1;
                } else if (action.name == "none") {
                    speechimage_changed = 0;
                    dialogue.current_speechimage = null;
                    dialogue.current_speechimage_is_opening = 0;
                } else {
                    for (let i = 0; i < dialogue.speechimages_size; i++) {
                        if (dialogue.speechimages[i].name == action.name) {
                            dialogue.current_speechimage = dialogue.speechimages[i];
                            speechimage_changed = 1;
                            break;
                        }
                    }
                }
                if (speechimage_changed && action.repeat_anim != DIALOGUE_REPEATANIM_NONE)
                    dialogue.current_speechimage_repeat = action.repeat_anim;
                break;
            case DIALOGUE_TYPE_TEXT_FONT:
                if (dialogue.texsprite_speech == null) break;
                let fontholder = dialogue_internal_get_font(dialogue, action.name);
                if (fontholder) textsprite_change_font(dialogue.texsprite_speech, fontholder);
                break;
            case DIALOGUE_TYPE_TEXT_COLOR:
                if (dialogue.texsprite_speech == null) break;
                textsprite_set_color(dialogue.texsprite_speech, action.rgba[0], action.rgba[1], action.rgba[2]);
                if (!Number.isNaN(action.rgba[3])) textsprite_set_alpha(dialogue.texsprite_speech, action.rgba[3]);
                break;
            case DIALOGUE_TYPE_TEXT_BORDERCOLOR:
                if (dialogue.texsprite_speech == null) break;
                textsprite_border_set_color(dialogue.texsprite_speech, action.rgba[0], action.rgba[1], action.rgba[2], action.rgba[3]);
                break;
            case DIALOGUE_TYPE_TEXT_BORDEROFFSET:
                if (dialogue.texsprite_speech == null) break;
                textsprite_border_set_offset(dialogue.texsprite_speech, action.offset_x, action.offset_y);
                break;
            case DIALOGUE_TYPE_TEXT_SIZE:
                if (dialogue.texsprite_speech == null) break;
                if (!Number.isNaN(action.size)) textsprite_set_font_size(dialogue.texsprite_speech, action.size);
                break;
            case DIALOGUE_TYPE_TEXT_BORDERSIZE:
                if (dialogue.texsprite_speech == null) break;
                if (!Number.isNaN(action.size)) textsprite_border_set_size(dialogue.texsprite_speech, action.size);
                break;
            case DIALOGUE_TYPE_TEXT_BORDERENABLE:
                if (dialogue.texsprite_speech == null) break;
                textsprite_border_enable(dialogue.texsprite_speech, action.enabled);
                break;
            case DIALOGUE_TYPE_TEXT_PARAGRAPHSPACE:
                if (dialogue.texsprite_speech == null) break;
                if (!Number.isNaN(action.size)) textsprite_set_paragraph_space(dialogue.texsprite_speech, action.size);
                break;
            case DIALOGUE_TYPE_TEXT_ALIGN:
                if (dialogue.texsprite_speech == null) break;
                textsprite_set_align(dialogue.texsprite_speech, action.align_vertical, action.align_horizontal);
                if (action.align_paragraph != ALIGN_NONE) textsprite_set_paragraph_align(dialogue.texsprite_speech, action.align_paragraph);
                break;
            case DIALOGUE_TYPE_RUNMULTIPLECHOICE:
                dialogue.do_multiplechoice = dialogue_internal_get_multiplechoice(action.name);
                break;
            case DIALOGUE_TYPE_TITLE:
                if (dialogue.texsprite_title == null) break;
                textsprite_set_text_intern(dialogue.texsprite_title, 1, action.title);
                break;
            case DIALOGUE_TYPE_NOWAIT:
                dialogue.do_no_wait = 1;
                break;
        }

    }

    if (background_changed) {
        if (dialogue.anims_ui.background_in) animsprite_restart(dialogue.anims_ui.background_in);
        if (dialogue.anims_ui.background_out) animsprite_restart(dialogue.anims_ui.background_out);
    }

    if (speechimage_changed && dialogue.current_speechimage) {
        let toggle_default = statesprite_state_toggle(dialogue.current_speechimage.statesprite, null);
        let toggle_open = statesprite_state_toggle(dialogue.current_speechimage.statesprite, DIALOGUE_OPEN);

        dialogue.current_speechimage_is_opening = toggle_default || toggle_open;

        if (!dialogue.current_speechimage_is_opening) {
            // switch to idle animation
            statesprite_state_toggle(dialogue.current_speechimage.statesprite, DIALOGUE_IDLE);
        }

        statesprite_animation_restart(dialogue.current_speechimage.statesprite);

        let x, y;

        if (dialogue.current_speechimage.disable_vertical_center) {
            x = 0;
            y = 0;
        } else {
            x = dialogue.current_speechimage.offset_x;
            y = dialogue.current_speechimage.offset_y + (FUNKIN_SCREEN_RESOLUTION_HEIGHT / 2.0);
        }

        // set speech background location
        statesprite_set_draw_location(dialogue.current_speechimage.statesprite, x, y);

        // set speech text bounds
        textsprite_set_draw_location(
            dialogue.texsprite_speech,
            dialogue.current_speechimage.text_x + x,
            dialogue.current_speechimage.text_y + y
        );
        textsprite_set_max_draw_size(
            dialogue.texsprite_speech,
            dialogue.current_speechimage.text_width,
            dialogue.current_speechimage.text_height
        );

        // set title location
        textsprite_set_draw_location(
            dialogue.texsprite_title,
            dialogue.current_speechimage.title_x + x,
            dialogue.current_speechimage.title_y + y
        );
    }

    return 1;
}

function dialogue_internal_draw_background(dialogue, pvrctx) {
    if (dialogue.change_background_from >= 0)
        sprite_draw(dialogue.backgrounds[dialogue.change_background_from].sprite, pvrctx);

    if (dialogue.current_background >= 0)
        sprite_draw(dialogue.backgrounds[dialogue.current_background].sprite, pvrctx);
}

function dialogue_internal_draw_portraits(dialogue, pvrctx) {
    const draw_location = [0, 0];
    const draw_size = [0, 0];
    let portrait_line_width;

    if (dialogue.current_speechimage) {
        statesprite_get_draw_location(dialogue.current_speechimage.statesprite, draw_location);
        draw_location[0] += dialogue.current_speechimage.portrait_line_x;
        draw_location[1] += dialogue.current_speechimage.portrait_line_y;
        portrait_line_width = dialogue.current_speechimage.portrait_line_width;
    } else {
        draw_location[0] = 0;
        draw_location[1] = FUNKIN_SCREEN_RESOLUTION_HEIGHT / 2.0;
        portrait_line_width = 0.9 * FUNKIN_SCREEN_RESOLUTION_WIDTH;
    }

    for (let portrait of linkedlist_iterate4(dialogue.visible_portraits)) {
        statesprite_get_draw_size(portrait.statesprite, draw_size);

        let draw_x = portrait.position * portrait_line_width;
        let draw_y = 0;

        switch (dialogue.current_speechimage.align_horizontal) {
            case ALIGN_NONE:
                if (portrait.position == 0.5) {
                    draw_x -= draw_size[0] / 2.0;
                } else if (portrait.position > 0.5) {
                    draw_x -= draw_size[0];
                }
                break;
            case ALIGN_END:
                draw_x -= draw_size[0];
                break;
            case ALIGN_CENTER:
                draw_x -= draw_size[0] / 2;
                break;
        }

        switch (dialogue.current_speechimage.align_vertical) {
            case ALIGN_NONE:
            case ALIGN_CENTER:
                draw_y = draw_size[1] / -2;
                break;
            case ALIGN_END:
                draw_y -= draw_size[1];
                break;
        }

        if (portrait.is_speaking) {
            draw_x += portrait.offset_speak_x;
            draw_y += portrait.offset_speak_y;
        } else {
            draw_x += portrait.offset_idle_x;
            draw_y += portrait.offset_idle_y;
        }

        pvr_context_save(pvrctx);
        sh4matrix_translate(pvrctx.current_matrix, draw_x + draw_location[0], draw_y + draw_location[1]);
        statesprite_draw(portrait.statesprite, pvrctx);
        pvr_context_restore(pvrctx);
    }

}

function dialogue_internal_draw_speech(dialogue, pvrctx) {
    if (dialogue.current_speechimage == null) return;

    statesprite_draw(dialogue.current_speechimage.statesprite, pvrctx);
    textsprite_draw(dialogue.texsprite_title, pvrctx);
    textsprite_draw(dialogue.texsprite_speech, pvrctx);
}


function dialogue_internal_prepare_print_text(dialogue) {
    if (!dialogue.current_dialog) return;

    if (dialogue.current_dialog_line < dialogue.current_dialog.lines_size) {
        dialogue_apply_state(
            dialogue, dialogue.current_dialog.lines[dialogue.current_dialog_line].target_state_name
        );
        if (dialogue.do_exit) return;
    }

    if (dialogue.do_skip) {
        dialogue.current_dialog_line++;
        dialogue.do_skip = 0;
        dialogue_internal_prepare_print_text(dialogue);
        return;
    }

    if (dialogue.current_dialog_line >= dialogue.current_dialog.lines_size) {
        dialogue_close(dialogue);
        return;
    }

    let line = dialogue.current_dialog.lines[dialogue.current_dialog_line];

    if (dialogue.do_instant_print) {
        dialogue_internal_notify_script(dialogue, 1);
        textsprite_set_text(dialogue.texsprite_speech, line.text);
        dialogue_internal_notify_script(dialogue, 0);
        dialogue_internal_toggle_idle(dialogue);
        dialogue.do_instant_print = 0;
        dialogue.is_speaking = 0;
        return;
    }

    let buffer = dialogue.current_dialog_buffer;
    const grapheme = { code: 0x00, size: 0 };
    let index = 0;

    stringbuilder_clear(buffer);
    dialogue.current_dialog_mask = undefined;
    dialogue.current_dialog_mask = null;

    while (string_get_character_codepoint(line.text, index, grapheme)) {
        switch (grapheme.code) {
            case 0x20:// space
            case 0x09:// tab
            case 0x0A:// new line
                //case 0x2E:// dot
                //case 0x3A:// double-dot
                //case 0x2C:// comma
                //case 0x3B:// dot-comma
                stringbuilder_add_char_codepoint(buffer, grapheme.code);
                break;
            default:
                stringbuilder_add_char_codepoint(buffer, 0xA0);// hard-space
                break;
        }
        index += grapheme.size;
    }
    dialogue.current_dialog_codepoint_index = 0;
    dialogue.current_dialog_codepoint_length = index;
    dialogue.current_dialog_mask = stringbuilder_get_copy(buffer);
    dialogue.is_speaking = 1;

    if (dialogue.chars_per_second > 0)
        dialogue.current_dialog_duration = (index / dialogue.chars_per_second) * 1000.0;
    else
        dialogue.current_dialog_duration = 0;
    dialogue.current_dialog_duration += index * dialogue.char_delay;
    dialogue.current_dialog_elapsed = 0.0;

    if (dialogue.current_dialog_line > 0 && dialogue.click_text) soundplayer_replay(dialogue.click_text);

    dialogue_internal_notify_script(dialogue, 1);
    textsprite_set_text(dialogue.texsprite_speech, dialogue.current_dialog_mask);
}

function dialogue_internal_print_text(dialogue) {
    if (dialogue.current_dialog == null) return;

    let line = dialogue.current_dialog.lines[dialogue.current_dialog_line];
    let buffer = dialogue.current_dialog_buffer;
    let length = dialogue.current_dialog_codepoint_length;

    let next_index = Math.trunc(length * (dialogue.current_dialog_elapsed / dialogue.current_dialog_duration));
    if (next_index == dialogue.current_dialog_codepoint_index && !dialogue.do_no_wait) return;

    if (dialogue.click_char) soundplayer_replay(dialogue.click_char);
    dialogue.current_dialog_codepoint_index = next_index;

    if (next_index < length) {
        stringbuilder_clear(buffer);
        stringbuilder_add_substring(buffer, line.text, 0, next_index);
        stringbuilder_add_substring(buffer, dialogue.current_dialog_mask, next_index, length);

        textsprite_set_text(dialogue.texsprite_speech, stringbuilder_intern(buffer));
        return;
    }

    dialogue_internal_notify_script(dialogue, 0);
    textsprite_set_text(dialogue.texsprite_speech, line.text);

    dialogue.is_speaking = 0;
    dialogue.current_dialog_line++;

    if (dialogue.do_no_wait) {
        dialogue.do_no_wait = 0;
        dialogue_internal_prepare_print_text(dialogue);
        return;
    }

    dialogue_internal_toggle_idle(dialogue);
}



function dialogue_internal_toggle_idle(dialogue) {
    for (let portrait of linkedlist_iterate4(dialogue.visible_portraits)) {
        if (!portrait.is_speaking) continue;
        portrait.is_speaking = false;
        if (statesprite_state_toggle(portrait.statesprite, DIALOGUE_IDLE) || statesprite_state_toggle(portrait.statesprite, null)) {
            let anim = statesprite_state_get(portrait.statesprite).animation;
            if (anim) animsprite_restart(anim);
        } else {
            dialogue_internal_stop_portrait_animation(portrait);
        }
    }
}

function dialogue_internal_destroy_external_dialog(dialogue) {
    if (dialogue.dialog_external == null) return;

    for (let i = 0; i < dialogue.dialogs_size; i++) {
        if (dialogue.dialogs[i] == dialogue.dialog_external) return;
    }

    for (let i = 0; i < dialogue.dialog_external.lines_size; i++) {
        dialogue.dialog_external.lines[i].target_state_name = undefined;
        dialogue.dialog_external.lines[i].text = undefined;
    }
    dialogue.dialog_external.lines = undefined;
    dialogue.dialog_external.full_path = undefined;
    dialogue.dialog_external = undefined;
    dialogue.dialog_external = null;
}

function dialogue_internal_notify_script(dialogue, is_line_start) {
    if (dialogue.script == null) return;

    let current_dialog_line = dialogue.current_dialog_line;
    let state_name = dialogue.current_dialog.lines[dialogue.current_dialog_line].target_state_name;

    if (is_line_start)
        _dialogue_lua_call(dialogue.script, "luascript_notify_dialogue_line_starts", current_dialog_line, state_name);
    else
        _dialogue_lua_call(dialogue.script, "luascript_notify_dialogue_line_ends", current_dialog_line, state_name);

}



async function dialogue_internal_parse_audiolist(root_node, audios) {
    for (let node of root_node.children) {
        switch (node.tagName) {
            case "Audio":
                await dialogue_internal_parse_audio(node, audios);
                break;
            default:
                console.error("dialogue_internal_parse_audiolist() unknown node: " + node.tagName);
                break;
        }
    }
}

async function dialogue_internal_parse_backgroundlist(root_node, max_width, max_height, backgrounds) {
    let base_src = root_node.getAttribute("baseSrc");

    for (let node of root_node.children) {
        switch (node.tagName) {
            case "Image":
                await dialogue_internal_parse_image(node, max_width, max_height, base_src, backgrounds);
                break;
            case "Color":
                dialogue_internal_parse_color(node, max_width, max_height, backgrounds);
                break;
            default:
                console.error("dialogue_internal_parse_backgroundlist() unknown node: " + node.tagName);
                break;
        }
    }
}

async function dialogue_internal_parse_portraitlist(root_node, portraits) {
    let base_model = root_node.getAttribute("baseModel");

    for (let node of root_node.children) {
        switch (node.tagName) {
            case "Portrait":
                await dialogue_internal_parse_portrait(node, base_model, portraits);
                break;
            default:
                console.error("dialogue_internal_parse_portraitlist() unknown node: " + node.tagName);
                break;
        }
    }
}

async function dialogue_internal_parse_animationui(root_node, anims_ui) {
    let animation_list = root_node.getAttribute("animationList");
    let animlist = null;

    if (animation_list) {
        animlist = await animlist_init(animation_list);
        if (!animlist) {
            console.error("dialogue_internal_parse_animationui() can not initialize: " + root_node.outerHTML);
            return;
        }
    }

    for (let node of root_node.children) {
        switch (node.tagName) {
            case "Set":
            case "UnSet":
                dialogue_internal_parse_animationui_set(node, animlist, anims_ui);
                break;
            default:
                console.error("dialogue_internal_parse_animationui() unknown node: " + node.tagName);
                break;
        }
    }

    if (animlist) animlist_destroy(animlist);
}

function dialogue_internal_parse_state(root_node, states) {
    let initial = vertexprops_parse_boolean(root_node, "initial", 0);
    let name = root_node.getAttribute("name");
    let actions = arraylist_init();

    for (let node of root_node.children) {
        let action = {
            name: node.getAttribute("name"),
            rgba: [NaN, NaN, NaN, NaN]
        };

        switch (node.tagName) {
            case "AudioPlay":
                action.type = DIALOGUE_TYPE_AUDIO_PLAY;
                break;
            case "AudioPause":
                action.type = DIALOGUE_TYPE_AUDIO_PAUSE;
                break;
            case "AudioFadeIn":
                action.type = DIALOGUE_TYPE_AUDIO_FADEIN;
                action.duration = vertexprops_parse_float(node, "duration", 1000.0);
                break;
            case "AudioFadeOut":
                action.type = DIALOGUE_TYPE_AUDIO_FADEOUT;
                action.duration = vertexprops_parse_float(node, "duration", 1000.0);
                break;
            case "AudioStop":
                action.type = DIALOGUE_TYPE_AUDIO_STOP;
                break;
            case "AudioVolume":
                action.type = DIALOGUE_TYPE_AUDIO_VOLUME;
                action.volume = vertexprops_parse_float(node, "volume", 1.0);
                break;
            case "BackgroundSet":
                action.type = DIALOGUE_TYPE_BACKGROUND_SET;
                break;
            case "BackgroundChange":
                action.type = DIALOGUE_TYPE_BACKGROUND_CHANGE;
                break;
            case "BackgroundRemove":
                action.type = DIALOGUE_TYPE_BACKGROUND_REMOVE;
                break;
            case "Lua":
                action.type = DIALOGUE_TYPE_LUA;
                action.lua_eval = node.textContent;
                action.lua_function = node.getAttribute("function");
                break;
            case "Exit":
                action.type = DIALOGUE_TYPE_EXIT;
                break;
            case "PortraitAdd":
                action.type = DIALOGUE_TYPE_PORTRAIT_ADD;
                action.random_from_prefix = node.getAttribute("randomFromPrefix");
                action.no_speak = vertexprops_parse_boolean(node, "noSpeak", 0);
                break;
            case "PortraitRemove":
                action.type = DIALOGUE_TYPE_PORTRAIT_REMOVE;
                break;
            case "PortraitRemoveAll":
                action.type = DIALOGUE_TYPE_PORTRAIT_REMOVEALL;
                action.animate_remove = vertexprops_parse_boolean(node, "animateRemove", false);
                break;
            case "AudioUI":
                action.type = DIALOGUE_TYPE_AUDIO_UI;
                action.click_text = node.getAttribute("clickText");
                action.click_char = node.getAttribute("clickChar");
                break;
            case "TextSpeed":
                action.type = DIALOGUE_TYPE_TEXT_SPEED;
                action.chars_per_second = vertexprops_parse_integer(node, "charsPerSecond", 0);
                action.char_delay = vertexprops_parse_integer(node, "charDelay", 75);
                break;
            case "TextSkip":
                action.type = DIALOGUE_TYPE_TEXT_SKIP;
                break;
            case "TextInstantPrint":
                action.type = DIALOGUE_TYPE_TEXT_INSTANTPRINT;
                break;
            case "SpeechBackground":
                action.type = DIALOGUE_TYPE_SPEECH_BACKGROUND;
                if (action.name == null) {
                    let tmp = node.getAttribute("fromCommon");
                    if (tmp != "none") action.name = tmp;
                }
                switch (node.getAttribute("repeatAnim")) {
                    case "":
                    case null:
                        action.repeat_anim = DIALOGUE_REPEATANIM_NONE;
                        break;
                    case "once":
                        action.repeat_anim = DIALOGUE_REPEATANIM_ONCE;
                        break;
                    case "whileSpeaks":
                    case "whilespeaks":
                        action.repeat_anim = DIALOGUE_REPEATANIM_WHILESPEAKS;
                        break;
                    case "always":
                        action.repeat_anim = DIALOGUE_REPEATANIM_ALWAYS;
                        break;
                    default:
                        action.repeat_anim = DIALOGUE_REPEATANIM_NONE;
                        console.error("dialogue_internal_parse_state() unknown repeatAnim value: " + node.outerHTML);
                        break;
                }
                break;
            case "TextFont":
                action.type = DIALOGUE_TYPE_TEXT_FONT;
                break;
            case "TextColor":
                action.type = DIALOGUE_TYPE_TEXT_COLOR;
                dialogue_internal_read_color(node, action.rgba);
                break;
            case "TextBorderColor":
                action.type = DIALOGUE_TYPE_TEXT_BORDERCOLOR;
                dialogue_internal_read_color(node, action.rgba);
                break;
            case "TextBorderOffset":
                action.type = DIALOGUE_TYPE_TEXT_BORDEROFFSET;
                action.offset_x = vertexprops_parse_float(node, "x", NaN);
                action.offset_y = vertexprops_parse_float(node, "y", NaN);
                break;
            case "TextSize":
                action.type = DIALOGUE_TYPE_TEXT_SIZE;
                action.size = vertexprops_parse_float(node, "size", 18.0);
                break;
            case "TextBorderSize":
                action.type = DIALOGUE_TYPE_TEXT_BORDERSIZE;
                action.size = vertexprops_parse_float(node, "size", 2.0);
                break;
            case "TextBorderEnable":
                action.type = DIALOGUE_TYPE_TEXT_BORDERENABLE;
                action.enabled = vertexprops_parse_boolean(node, "enabled", 0);
                break;
            case "TextParagraphSpace":
                action.type = DIALOGUE_TYPE_TEXT_PARAGRAPHSPACE;
                action.size = vertexprops_parse_float(node, "size", 0.0);
                break;
            case "TextAlign":
                action.type = DIALOGUE_TYPE_TEXT_ALIGN;
                action.align_vertical = dialogue_internal_read_align(node, "vertical");
                action.align_horizontal = dialogue_internal_read_align(node, "horizontal");
                action.align_paragraph = dialogue_internal_read_align(node, "paragraph");
                break;
            case "RunMultipleChoice":
                action.type = DIALOGUE_TYPE_RUNMULTIPLECHOICE;
                break;
            case "Title":
                action.type = DIALOGUE_TYPE_TITLE;
                action.title = node.textContent;
                break;
            case "NoWait":
                action.type = DIALOGUE_TYPE_NOWAIT;
                break;
            default:
                console.error("dialogue_internal_parse_state() unknown state action: " + node.outerHTML);
                continue;
        }

        arraylist_add(actions, action);
    }

    let state = {
        name: name,
        initial: initial,
        actions: null,
        actions_size: 0
    };

    arraylist_destroy2(actions, state, "actions_size", "actions");
    arraylist_add(states, state);
}

async function dialogue_internal_parse_multiplechoice(root_node, animlist, dialogs, multiplechoices) {
    let title = root_node.getAttribute("title");
    let name = root_node.getAttribute("name");
    let orientation = root_node.getAttribute("orientation");
    let can_leave = vertexprops_parse_boolean(root_node, "canLeave", 0);
    let state_on_leave = root_node.getAttribute("stateOnLeave");
    let icon_color = vertexprops_parse_hex2(root_node.getAttribute("selectorIconColor"), 0x00FFFF, 0);
    let icon_model = root_node.getAttribute("selectorIconColor");
    let icon_model_name = root_node.getAttribute("selectorIconModelName");
    let default_index = vertexprops_parse_integer(root_node, "defaultIndex", 0);
    let font_size = vertexprops_parse_float(root_node, "fontSize", -1);

    let is_vertical;

    switch (orientation) {
        case null:
        case "":
        case "vertical":
            is_vertical = 1;
            break;
        case "horizontal":
            is_vertical = 0;
            break;
        default:
            is_vertical = 1;
            console.error("dialogue_internal_parse_multiple_choice() unknown orientation value:" + orientation);
            break;
    }

    let texture = null;
    let anim = null;

    if (icon_model) {
        if (modelholder_utils_is_known_extension(icon_model)) {
            let modeholder = await modelholder_init(icon_model);
            if (modeholder == null) {
                console.error("dialogue_internal_parse_multiple_choice() can not initialize: " + icon_model);
            } else {
                texture = modelholder_get_texture(modeholder, 1);
                anim = modelholder_create_animsprite(modeholder, icon_model_name ?? DIALOGUE_ICON, 1, 0);
                icon_color = modelholder_get_vertex_color(modeholder);
                modelholder_destroy(modeholder);
            }
        } else {
            texture = await texture_init(icon_model);
        }
    }

    if (anim == null) anim = animsprite_init_from_animlist(animlist, DIALOGUE_ICON);

    let icon = sprite_init(texture);
    sprite_set_vertex_color_rgb8(icon, icon_color);
    sprite_external_animation_set(icon, anim);

    let hint = sprite_init_from_rgb8(0xF9CF51);
    sprite_set_alpha(hint, 0.0);

    let multiplechoice = {
        icon: icon,
        hint: hint,
        name: name,
        title: title,
        can_leave: can_leave,
        state_on_leave: state_on_leave,
        default_index: default_index,
        font_size: font_size,
        choices: null,
        choices_size: 0,
        is_vertical: is_vertical
    };

    let choices = arraylist_init();

    for (let node of root_node.children) {
        switch (node.tagName) {
            case "Choice":
                await dialogue_internal_parse_choice(node, dialogs, choices);
                break;
            default:
                console.error("dialogue_internal_parse_multiplechoice() unknown: " + node.outerHTML);
                break;
        }
    }

    arraylist_destroy2(choices, multiplechoice, "choices_size", "choices");
    arraylist_add(multiplechoices, multiplechoice);
}

async function dialogue_internal_parse_speechimagelist(root_node, speechimages) {
    let base_src = root_node.getAttribute("baseSrc");
    let text_x = 0.0;
    let text_y = 0.0;
    let text_width = 0.0;
    let text_height = 0.0;
    let portrait_line_x = 0.0;
    let portrait_line_y = 0.0;
    let portrait_line_width = 0.0;
    let title_x = 0.0;
    let title_y = 0.0;
    let offset_x = 0.0;
    let offset_y = 0.0;
    let offset_idle_x = 0.0;
    let offset_idle_y = 0.0;
    let offset_open_x = 0.0;
    let offset_open_y = 0.0;
    let disable_vertical_center = 0;
    let align_vertical = ALIGN_NONE;
    let align_horizontal = ALIGN_NONE;


    for (let node of root_node.children) {
        switch (node.tagName) {
            case "SpeechImage":
                let speechimage = await dialogue_internal_parse_speechimage(node, base_src, speechimages);
                if (speechimage == null) continue;

                speechimage.text_x = text_x;
                speechimage.text_y = text_y;
                speechimage.text_width = text_width;
                speechimage.text_height = text_height;
                speechimage.portrait_line_x = portrait_line_x;
                speechimage.portrait_line_y = portrait_line_y;
                speechimage.portrait_line_width = portrait_line_width;
                speechimage.title_x = title_x;
                speechimage.title_y = title_y;
                speechimage.offset_x = offset_x;
                speechimage.offset_y = offset_y;
                speechimage.disable_vertical_center = disable_vertical_center;
                speechimage.align_vertical = align_vertical;
                speechimage.align_horizontal = align_horizontal;

                for (let state of linkedlist_iterate4(statesprite_state_list(speechimage.statesprite))) {
                    switch (state.state_name) {
                        case DIALOGUE_OPEN:
                            state.offset_x = offset_open_x;
                            state.offset_y = offset_open_y;
                            break;
                        case DIALOGUE_IDLE:
                            state.offset_x = offset_idle_x;
                            state.offset_y = offset_idle_y;
                            break;
                    }
                }
                break;
            case "TextBox":
                text_x = vertexprops_parse_float(node, "x", text_x);
                text_y = vertexprops_parse_float(node, "y", text_y);
                text_width = vertexprops_parse_float(node, "width", text_width);
                text_height = vertexprops_parse_float(node, "height", text_height);
                break;
            case "PortraitLine":
                portrait_line_x = vertexprops_parse_float(node, "x", portrait_line_x);
                portrait_line_y = vertexprops_parse_float(node, "y", portrait_line_y);
                portrait_line_width = vertexprops_parse_float(node, "width", portrait_line_width);
                break;
            case "TitleLocation":
                title_x = vertexprops_parse_float(node, "titleLeft", title_x);
                title_y = vertexprops_parse_float(node, "titleBottom", title_y);
                break;
            case "Offset":
                offset_x = vertexprops_parse_float(node, "x", offset_x);
                offset_y = vertexprops_parse_float(node, "y", offset_y);
                disable_vertical_center = vertexprops_parse_boolean(node, "disableVerticalCenter", 0);
                break;
            case "OffsetIdle":
                offset_idle_x = vertexprops_parse_float(node, "x", offset_x);
                offset_idle_y = vertexprops_parse_float(node, "y", offset_y);
                break;
            case "OffsetOpen":
                offset_open_x = vertexprops_parse_float(node, "x", offset_x);
                offset_open_y = vertexprops_parse_float(node, "y", offset_y);
                break;
            case "PortraitAlign":
                align_vertical = dialogue_internal_read_align(node, "vertical");
                align_horizontal = dialogue_internal_read_align(node, "horizontal");
                break;
            default:
                console.error("dialogue_internal_parse_speechimagelist() unknown node: " + node.tagName);
                break;
        }
    }
}

async function dialogue_internal_parse_importportraitlist(root_node, portraits) {
    let dialogue_src = root_node.getAttribute("dialogueSrc");
    if (!dialogue_src) {
        console.error("dialogue_internal_parse_importportraitlist() missing dialogueSrc: " + root_node.outerHTML);
        return;
    }

    if (string_lowercase_ends_with(dialogue_src, ".json")) {
        await dialogue_internal_load_psych_character_json(dialogue_src, portraits);
        return;
    }

    let xmlparser = null;
    try {
        xmlparser = await fs_readxml(dialogue_src);
    } catch (e) {
        console.error(e);
    }

    if (!xmlparser || !xmlparser.documentElement) {
        if (!xmlparser) xmlparser = undefined;
        console.error("dialogue_internal_parse_importportraitlist() can not load: " + dialogue_src);
        return;
    }

    fs_folder_stack_push();
    fs_set_working_folder(dialogue_src, 1);

    for (let node of root_node.children) {
        if (node.tagName != "Definition") continue;
        for (let node2 of root_node.children) {
            if (node2.tagName != "PortraitList") continue;
            await dialogue_internal_parse_importportraitlist(node2, portraits);
        }
    }

    fs_folder_stack_pop();
    xmlparser = undefined;
}

async function dialogue_internal_parse_audio(node, audios) {
    //<Audio name="bg_music2" src="/assets/music/weeb.ogg" volume="1.0" looped="true" defaultAs="clickText|clickChar" />
    let name = node.getAttribute("name");
    let src = node.getAttribute("src");
    let volume = vertexprops_parse_float(node, "volume", 1.0);
    let looped = vertexprops_parse_boolean(node, "looped", 0);

    // avoid duplicates
    for (let i = 0, size = arraylist_size(audios); i < size; i++) {
        if (arraylist_get(audios, i).name == name) return;
    }

    if (!name) {
        console.error("dialogue_internal_parse_audio() missing name: " + node.outerHTML);
        return;
    }
    if (!src) {
        console.error("dialogue_internal_parse_audio() missing src: " + node.outerHTML);
        return;
    }

    let soundplayer = await soundplayer_init(src);
    if (!soundplayer) {
        console.error("dialogue_internal_parse_audio() can not initialize: " + node.outerHTML);
        return;
    }

    soundplayer_set_volume(soundplayer, volume);
    soundplayer_loop_enable(soundplayer, looped);

    let audio = {
        name: name,
        was_playing: false,
        soundplayer: soundplayer
    };

    arraylist_add(audios, audio);
}

async function dialogue_internal_parse_image(node, max_width, max_height, base_src, backgrounds) {
    //<Image
    //          name="scene0"
    //          src="/assets/image/theater.xml"
    //          entryName="floor"
    //          center="true"
    //          cover="true"
    //          looped="true"
    //          alpha="0.7"
    //          />

    let name = node.getAttribute("name");
    let src = node.getAttribute("src");
    let entry_name = node.getAttribute("entryName");
    let center = vertexprops_parse_boolean(node, "center", 0);
    let cover = vertexprops_parse_boolean(node, "cover", 0);
    let looped = vertexprops_parse_boolean(node, "looped", 0);
    let has_looped = node.hasAttribute("looped");
    let alpha = vertexprops_parse_float(node, "alpha", 1.0);

    // avoid duplicates
    for (let i = 0, size = arraylist_size(backgrounds); i < size; i++) {
        if (arraylist_get(backgrounds, i).name == name) return;
    }

    if (!name) {
        console.error("dialogue_internal_parse_image() missing name: " + node.outerHTML);
        return;
    }

    if (src == null) src = base_src;
    if (!src) {
        console.error("dialogue_internal_parse_image() missing src: " + node.outerHTML);
        return;
    }

    let texture = null;
    let animsprite = null;
    let vertex_color_rgb8 = 0xFFFFFF;
    let init_failed;

    if (modelholder_utils_is_known_extension(src)) {
        let modelholder = await modelholder_init(src);
        init_failed = modelholder == null;
        if (modelholder != null) {
            texture = modelholder_get_texture(modelholder, 1);
            animsprite = modelholder_create_animsprite(modelholder, entry_name, 1, 0);
            vertex_color_rgb8 = modelholder_get_vertex_color(modelholder);
            if (has_looped && animsprite) animsprite_set_loop(animsprite, looped ? -1 : 1);
            modelholder_destroy(modelholder);
        }
    } else {
        texture = await texture_init(src);
        init_failed = !texture;
    }

    if (init_failed) {
        console.error("dialogue_internal_parse_image() can not initialize: " + node.outerHTML);
        return;
    }

    let sprite = sprite_init(texture);
    sprite_set_vertex_color_rgb8(sprite, vertex_color_rgb8);
    sprite_external_animation_set(sprite, animsprite);
    sprite_set_draw_location(sprite, 0.0, 0.0);
    sprite_set_alpha(sprite, alpha);
    imgutils_calc_resize_sprite(sprite, max_width, max_height, cover, center);

    let background = {
        name: name,
        sprite: sprite
    };

    arraylist_add(backgrounds, background);
}

function dialogue_internal_parse_color(node, max_width, max_height, backgrounds) {
    //<Color name="faded_white" rgba="0xFFFFFF7F" />
    //<Color name="faded_red" r="1.0" g="0.0" g="0.0" a="0.7" />

    let name = node.getAttribute("name");
    if (!name) {
        console.error("dialogue_internal_parse_image() missing name: " + node.outerHTML);
        return;
    }

    // avoid duplicates
    for (let i = 0, size = arraylist_size(backgrounds); i < size; i++) {
        if (arraylist_get(backgrounds, i).name == name) return;
    }

    let rgba = [0.0, 0.0, 0.0, 1.0];
    dialogue_internal_read_color(node, rgba);

    let sprite = sprite_init(null);
    sprite_set_vertex_color(sprite, rgba[0], rgba[1], rgba[2]);
    sprite_set_alpha(sprite, rgba[3]);
    sprite_set_draw_size(sprite, max_width, max_height);

    let background = {
        name: name,
        sprite: sprite
    };

    arraylist_add(backgrounds, background);
}

async function dialogue_internal_parse_font(node, fonts) {
    //<Font name="vcr" src="vcr.ttf" glyphSuffix="bold" glyphAnimated="false"  colorByDifference="true" />

    let name = node.getAttribute("name");
    let src = node.getAttribute("src");
    let glyph_suffix = node.getAttribute("glyphSuffix");
    let glyph_animated = vertexprops_parse_boolean(node, "glyphAnimated", 0);
    let color_by_difference = vertexprops_parse_boolean(node, "colorByDifference", 0);

    if (!name) {
        console.error("dialogue_internal_parse_font() missing name: " + node.outerHTML);
        return;
    }
    if (!src) {
        console.error("dialogue_internal_parse_font() missing src: " + node.outerHTML);
        return;
    }
    if (!(await fs_file_exists(src))) {
        console.error("dialogue_internal_parse_font() font file not found: " + node.outerHTML);
        return;
    }

    // avoid duplicates
    for (let i = 0, size = arraylist_size(fonts); i < size; i++) {
        if (arraylist_get(fonts, i).name == name) return;
    }

    let instance;
    let is_atlas;

    if (atlas_utils_is_known_extension(src)) {
        instance = await fontglyph_init(src, glyph_suffix, glyph_animated);
        fontglyph_enable_color_by_difference(instance, color_by_difference);
        is_atlas = 1;
    } else {
        instance = await fonttype_init(src);
        is_atlas = 0;
    }

    if (!instance) {
        console.error("dialogue_internal_parse_font() can not initialize: " + node.outerHTML);
        return;
    }

    let font = {
        name: name,
        fontholder: fontholder_init2(instance, is_atlas, 18.0)
    };

    arraylist_add(fonts, font);
}

async function dialogue_internal_parse_portrait(node, base_model, portraits) {
    // <Portrait
    //          name="senpai" src="/assets/image/senpai.png" scale="1.0"
    //          speakAnim="talking" idleAnim="idle_anim" anim="talking_and_idle_anim"
    //          speakAnimLooped="true|false" idleAnimLooped="true|false" animLooped="true|false"
    //          mirror="true|false"
    //          positionPercent="0.0" position="left|center|right"
    //          x="0" y="0"
    //          offestSpeakX="0" offestSpeakY="0"
    //          offestIdleX="0" offestIdleY="0" 
    // />

    let name = node.getAttribute("name");
    let src = node.getAttribute("src");
    let scale = vertexprops_parse_float(node, "scale", 1.0);
    let speak_anim = node.getAttribute("speakAnim");
    let idle_anim = node.getAttribute("idleAnim");
    let simple_anim = node.getAttribute("anim");
    let speak_anim_looped = vertexprops_parse_boolean(node, "speakAnimLooped", 1);
    let idle_anim_looped = vertexprops_parse_boolean(node, "idleAnimLooped", 1);
    let simple_anim_looped = vertexprops_parse_boolean(node, "animLooped", 1);
    let mirror = vertexprops_parse_boolean(node, "mirror", false);
    let position = vertexprops_parse_float(node, "positionPercent", 0.0);
    let position_align = node.getAttribute("position");
    let x = vertexprops_parse_float(node, "x", 0.0);
    let y = vertexprops_parse_float(node, "y", 0.0);
    let offset_speak_x = vertexprops_parse_float(node, "offestSpeakX", 0.0);
    let offset_speak_y = vertexprops_parse_float(node, "offestSpeakY", 0.0);
    let offset_idle_x = vertexprops_parse_float(node, "offestIdleX", 0.0);
    let offset_idle_y = vertexprops_parse_float(node, "offestIdleY", 0.0);

    // avoid duplicates
    for (let i = 0, size = arraylist_size(portraits); i < size; i++) {
        if (arraylist_get(portraits, i).name == name) return;
    }

    switch (position_align) {
        case "left":
            position = 0.0;
            break;
        case "center":
            position = 0.5;
            break;
        case "right":
            position = 1.0;
            break;
        case "":
        case null:
            break;
        default:
            console.warn("dialogue_internal_parse_portrait() unknown position: " + node.outerHTML);
            break;
    }

    if (!name) {
        console.error("dialogue_internal_parse_portrait() missing name: " + node.outerHTML);
        return;
    }

    if (src == null) src = base_model;
    if (!src) {
        console.error("dialogue_internal_parse_portrait() missing src: " + node.outerHTML);
        return;
    }

    let statesprite = null;

    L_load:
    if (modelholder_utils_is_known_extension(src)) {
        let modelholder = await modelholder_init(src);
        if (!modelholder) break L_load;

        statesprite = statesprite_init_from_texture(null);
        statesprite_change_draw_size_in_atlas_apply(statesprite, true, scale);
        statesprite_set_draw_location(statesprite, 0.0, 0.0);

        dialogue_internal_add_state(statesprite, modelholder, simple_anim, null, scale, simple_anim_looped);
        dialogue_internal_add_state(statesprite, modelholder, idle_anim, DIALOGUE_IDLE, scale, false);
        dialogue_internal_add_state(statesprite, modelholder, speak_anim, DIALOGUE_SPEAK, scale, false);

        modelholder_destroy(modelholder);
    } else {
        let texture = await texture_init(src);
        if (!texture) break L_load;

        statesprite = statesprite_init_from_texture(texture);
        statesprite_set_draw_location(statesprite, 0.0, 0.0);

        const draw_size = [0, 0];
        texture_get_original_dimmensions(statesprite, draw_size);
        statesprite_set_draw_size(statesprite, draw_size[0] * scale, draw_size[1] * scale);
    }


    if (!statesprite) {
        console.error("dialogue_internal_parse_portrait() can not initialize: " + node.outerHTML);
        return;
    }

    for (let state of linkedlist_iterate4(statesprite_state_list(statesprite))) {
        state.offset_x += x;
        state.offset_y += y;
    }
    statesprite_flip_texture(statesprite, mirror, null);
    statesprite_flip_texture_enable_correction(statesprite, 0);

    let portrait = {
        name: name,
        statesprite: statesprite,
        position: position,
        is_removed: 0,
        is_added: 0,
        is_speaking: 0,
        offset_speak_x: offset_speak_x,
        offset_speak_y: offset_speak_y,
        offset_idle_x: offset_idle_x,
        offset_idle_y: offset_idle_y,
        has_speak: statesprite_state_has(statesprite, DIALOGUE_SPEAK),
        has_idle: statesprite_state_has(statesprite, DIALOGUE_IDLE),
        speak_anim_looped: speak_anim_looped,
        idle_anim_looped: idle_anim_looped
    };

    arraylist_add(portraits, portrait);
}

function dialogue_internal_parse_animationui_set(node, animlist, anims_ui) {
    //<Set name="backgroundIn|backgroundOut" anim="anim123" />
    //<Set name="portraitLeftIn|portraitCenterIn|portraitRightIn" anim="anim123" />
    //<Set name="portraitIn|portraitOut" anim="anim123" />
    //<Set name="portraitLeftOut|portraitCenterOut|portraitRightOut" anim="anim123" />

    let name = node.getAttribute("name");
    let anim = node.getAttribute("anim");

    if (!name) {
        console.error("dialogue_internal_parse_animation_ui_set() missing name: " + node.outerHTML);
        return;
    }

    let animsprite = null;
    if (anim) {
        if (!animlist) {
            console.error("dialogue_internal_parse_animation_ui_set() can not initialize without animlist: " + node.outerHTML);
            return;
        }
        animsprite = animsprite_init_from_animlist(animlist, anim);
        if (!animsprite) {
            console.error("dialogue_internal_parse_animation_ui_set() can not initialize: " + node.outerHTML);
            return;
        }
    }

    let old_anim = null;

    switch (name.toLowerCase()) {
        case "portraitin":
            if (anims_ui.portrait_left_in) animsprite_destroy(anims_ui.portrait_left_in);
            if (anims_ui.portrait_center_in) animsprite_destroy(anims_ui.portrait_center_in);
            if (anims_ui.portrait_right_in) animsprite_destroy(anims_ui.portrait_right_in);

            anims_ui.portrait_left_in = animsprite ? animsprite_clone(animsprite) : null;
            anims_ui.portrait_center_in = animsprite ? animsprite_clone(animsprite) : null;
            anims_ui.portrait_right_in = animsprite ? animsprite_clone(animsprite) : null;
            break;
        case "portraitout":
            if (anims_ui.portrait_center_out) animsprite_destroy(anims_ui.portrait_center_out);
            if (anims_ui.portrait_right_out) animsprite_destroy(anims_ui.portrait_right_out);
            if (anims_ui.portrait_right_out) animsprite_destroy(anims_ui.portrait_right_out);

            anims_ui.portrait_left_out = animsprite ? animsprite_clone(animsprite) : null;
            anims_ui.portrait_center_out = animsprite ? animsprite_clone(animsprite) : null;
            anims_ui.portrait_right_out = animsprite ? animsprite_clone(animsprite) : null;
            break;
        case "portraitleftin":
            old_anim = anims_ui.portrait_left_in;
            anims_ui.portrait_left_in = animsprite;
            break;
        case "portraitcenterin":
            old_anim = anims_ui.portrait_center_in;
            anims_ui.portrait_center_in = animsprite;
            break;
        case "portrairighttin":
            old_anim = anims_ui.portrait_right_in;
            anims_ui.portrait_right_in = animsprite;
            break;
        case "portraitleftout":
            old_anim = anims_ui.portrait_left_out;
            anims_ui.portrait_left_out = animsprite;
            break;
        case "portraitcenterout":
            old_anim = anims_ui.portrait_center_out;
            anims_ui.portrait_center_out = animsprite;
            break;
        case "portraitrightout":
            old_anim = anims_ui.portrait_right_out;
            anims_ui.portrait_right_out = animsprite;
            break;
        case "backgroundin":
            old_anim = anims_ui.background_in;
            anims_ui.background_in = animsprite;
            break;
        case "backgroundout":
            old_anim = anims_ui.background_out;
            anims_ui.background_out = animsprite;
            break;
        case "open":
            old_anim = anims_ui.open;
            anims_ui.open = animsprite;
            break;
        case "close":
            old_anim = anims_ui.close;
            anims_ui.close = animsprite;
            break;
        default:
            console.error("dialogue_internal_parse_animation_ui_set() unknown name: " + node.outerHTML);
            if (animsprite) animsprite_destroy(animsprite);
            break;
    }

    if (old_anim) animlist_destroy(old_anim);
}

async function dialogue_internal_parse_choice(node, dialogs, choices) {
    // <Choice
    //          text="Exit"
    //          dialogFile="dialogs.txt"
    //          exit="true|false"
    //          runMultipleChoice="questions"
    //          luaFunction="somefunc"
    //          luaEval="doexit('from leave', true)"
    // >

    let text = node.getAttribute("text");
    let dialogs_file = node.getAttribute("dialogFile");
    let exit = vertexprops_parse_boolean(node, "exit", false);
    let run_multiple_choice = node.getAttribute("runMultipleChoice");
    let lua_function = node.getAttribute("luaFunction");
    let lua_eval = node.getAttribute("luaEval");

    if (text == null) {
        console.error("dialogue_internal_parse_choice() missing text in:" + node.outerHTML);
        return;
    }

    let dialog_id = -1;

    if (dialogs_file)
        dialog_id = await dialogue_internal_parse_dialog(dialogs_file, dialogs);

    let choice = {
        text: text,
        dialog_id: dialog_id,
        exit: exit,
        lua_eval: lua_eval,
        lua_function: lua_function,
        run_multiple_choice: run_multiple_choice
    };

    arraylist_add(choices, choice);
}

async function dialogue_internal_parse_speechimage(node, base_src, speechimages) {
    // <Image
    //          name="normal"
    //          mirror="true|false"
    //          openAnim="Speech Bubble Normal Open"
    //          idleAnim="speech bubble normal"
    //          idleLooped="true|false"
    //          src="/assets/image/model.xml"
    //          scale="1.0"
    //  >

    let name = node.getAttribute("name");
    let scale = vertexprops_parse_float(node, "scale", 1.0);
    let open_anim = node.getAttribute("openAnim");
    let idle_anim = node.getAttribute("idleAnim");
    let idle_looped = vertexprops_parse_boolean(node, "idleLooped", 1);
    let mirror = vertexprops_parse_boolean(node, "mirror", 0);
    let has_mirror = node.hasAttribute("mirror");
    let src = node.getAttribute("src");

    if (!name) {
        console.error("dialogue_internal_parse_speechimage() missing name: " + node.outerHTML);
        return null;
    }

    // avoid duplicates
    for (let i = 0, size = arraylist_size(speechimages); i < size; i++) {
        if (arraylist_get(speechimages, i).name == name) return null;
    }

    if (!src) src = base_src;
    if (!src) {
        console.error("dialogue_internal_parse_speechimage() missing src: " + node.outerHTML);
        return null;
    }

    let statesprite = null;

    L_load:
    if (modelholder_utils_is_known_extension(src)) {
        let modelholder = await modelholder_init(src);
        if (!modelholder) break L_load;

        statesprite = statesprite_init_from_texture(null);
        statesprite_set_draw_location(statesprite, 0.0, 0.0);

        dialogue_internal_add_state(statesprite, modelholder, open_anim, DIALOGUE_OPEN, scale, 0);
        dialogue_internal_add_state(statesprite, modelholder, idle_anim, DIALOGUE_IDLE, scale, idle_looped);

        if (linkedlist_count(statesprite_state_list(statesprite)) < 1) {
            let texture = modelholder_get_texture(modelholder, 1);
            if (texture) {
                const orig_size = [0, 0];
                texture_get_original_dimmensions(texture, orig_size);
                statesprite_set_texture(statesprite, texture, true);
                statesprite_set_draw_size(statesprite, orig_size[0] * scale, orig_size[1] * scale);
            } else {
                statesprite_destroy(statesprite);
                statesprite = null;
            }
        }

        modelholder_destroy(modelholder);
    } else {
        let texture = await texture_init(src);
        if (!texture) break L_load;

        statesprite = statesprite_init_from_texture(texture);
        statesprite_set_draw_location(statesprite, 0.0, 0.0);
    }

    if (!statesprite) {
        console.error("dialogue_internal_parse_speechimage() can not initialize: " + node.outerHTML);
        return null;
    }

    if (has_mirror) statesprite_flip_texture(statesprite, mirror, null);
    if (scale > 0.0) statesprite_change_draw_size_in_atlas_apply(statesprite, 1, scale);

    let speechimage = {
        name: name,
        statesprite: statesprite
    };

    arraylist_add(speechimages, speechimage);
    return speechimage;
}

async function dialogue_internal_load_psych_character_json(src, portraits) {
    let json = await json_load_from(src);
    if (!json) {
        console.error("dialogue_internal_load_psych_character_json() can not load: " + src);
        return;
    }

    let animations = json_read_array(json, "animations");
    let animations_length = json_read_array_length(animations);

    fs_folder_stack_push();
    fs_set_working_folder(src, 1);

    const position = [0, 0];
    dialogue_internal_read_offset(json, "position", position);
    let scale = json_read_number(json, "scale", 1.0);
    let dialogue_pos;

    let unparsed_dialogue_pos = json_read_string(json, "dialogue_pos", null);
    switch (unparsed_dialogue_pos) {
        case null:
        case "left":
            dialogue_pos = 0.0;
            break;
        case "center":
            dialogue_pos = 0.5;
            break;
        case "right":
            dialogue_pos = 1.0;
            break;
        default:
            dialogue_pos = 0.0;
            console.error("dialogue_internal_load_psych_character_json() unrecognized dialogue_pos: " + src);
            break;
    }

    L_process: {
        let image = json_read_string(json, "image", null);
        if (!image) {
            console.error("dialogue_internal_load_psych_character_json() missing 'image' of json: " + src);
            break L_process;
        } else if (image.indexOf('.', 0) >= 0) {
            // append atlas extension
            let tmp = string_concat(2, image, ".xml");
            image = undefined;
            image = tmp;
        }

        //
        // Note:
        //      There no such path like "/assets/shared/images/dialogue/required_atlas_file.xml"
        //      Attempt load the atlas from the current folder "./required_atlas_file.xml"
        //      or fallback to "/assets/common/image/dialogue/required_atlas_file.xml"
        //
        let modelholder = null;

        if (await fs_file_exists(image)) {
            modelholder = await modelholder_init(src);
        } else {
            // try load from common folder
            let tmp = string_concat(2, "/assets/common/image/dialogue/", image);
            if (await fs_file_exists(tmp)) {
                modelholder = await modelholder_init(tmp);
            }
            tmp = undefined;
        }

        if (modelholder == null) {
            console.error("dialogue_internal_load_psych_character_json() unreconized image path: " + image);
            break L_process;
        }

        // parse animations
        for (let i = 0; i < animations_length; i++) {
            let obj = json_read_array_item_object(animations, i);

            const idle_offset = [0, 0];
            const loop_offset = [0, 0];
            let anim, idle_name, loop_name;

            dialogue_internal_read_offset(obj, "idle_offsets", idle_offset);
            dialogue_internal_read_offset(obj, "loop_offsets", loop_offset);
            anim = json_read_string(obj, "anim", null);
            idle_name = json_read_string(obj, "idle_name", null);
            loop_name = json_read_string(obj, "loop_name", null);

            // prepare portrait
            let anim_idle = modelholder_create_animsprite(modelholder, idle_name, 1, 0);
            let anim_speak = modelholder_create_animsprite(modelholder, loop_name, 1, 0);
            let statesprite = statesprite_init_from_texture(null);
            statesprite_set_draw_location(statesprite, 0.0, 0.0);

            // accumulate offsets, assume 'position' as offset of portrait line
            idle_offset[0] += position[0];
            idle_offset[1] += position[1];
            loop_offset[0] += position[0];
            loop_offset[1] += position[1];

            if (scale >= 0.0) statesprite_change_draw_size_in_atlas_apply(statesprite, 1, scale);
            dialogue_internal_compute_state(modelholder, statesprite, anim_idle, idle_offset, 0);
            dialogue_internal_compute_state(modelholder, statesprite, anim_speak, loop_offset, 1);

            let portrait = {
                name: anim,
                is_added: false,
                is_removed: false,
                is_speaking: false,
                position: dialogue_pos,
                statesprite: statesprite
            };
            arraylist_add(portraits, portrait);
        }

        modelholder_destroy(modelholder);
    }

    fs_folder_stack_pop();
    json_destroy(json);
}

async function dialogue_internal_parse_dialog(src, dialogs) {
    let full_path = await fs_get_full_path_and_override(src);

    // check if is already loaded
    let id = 0;
    for (let existing_dialog of arraylist_iterate4(dialogs)) {
        if (existing_dialog.full_path == full_path) return id;
        id++;
    }

    // load and parse txt file
    let source = await fs_readtext(full_path);

    if (!source) {
        console.error("dialogue_internal_parse_dialog() can not read: " + src);
        full_path = undefined;
        return -1;
    }

    let tokenizer = tokenizer_init("\n", 0, 0, source);
    console.assert(tokenizer != null);

    let lines = arraylist_init2(tokenizer_count_occurrences(tokenizer));

    let line;
    let buffered_line = null;
    while ((line = tokenizer_read_next(tokenizer)) != null) {
        let end_index = -1;

        if (line.length > 0 && line[0] == ':') {
            end_index = line.indexOf(':', 1);
        }

        if (end_index < 0) {
            // buffer the current line
            let tmp = string_concat(3, buffered_line, "\n", line);
            buffered_line = undefined;
            buffered_line = tmp;
        } else {
            let state = line.substring(1, end_index);
            let tmp_line = line.substring(end_index + 1, line.length);
            let final_line = string_concat(2, buffered_line, tmp_line);

            tmp_line = undefined;
            buffered_line = undefined;
            buffered_line = null;

            let dialog_line = {
                target_state_name: state,
                text: final_line
            };

            arraylist_add(lines, dialog_line);
        }

        line = undefined;
    }

    if (buffered_line != null) {
        let dialog_line;
        if (arraylist_size(lines) < 1) {
            dialog_line = {
                target_state_name: null,
                text: buffered_line
            };
            arraylist_add(lines, dialog_line);
        } else {
            dialog_line = arraylist_get(lines, arraylist_size(lines) - 1);
            let tmp = string_concat(2, dialog_line.text, buffered_line);
            dialog_line.text = undefined;
            buffered_line = undefined;
            dialog_line.text = tmp;
        }
    }

    tokenizer_destroy(tokenizer);
    source = undefined;

    let dialog = {
        full_path: full_path,
        lines: null,
        lines_size: 0
    };

    arraylist_destroy2(lines, dialog, "lines_size", "lines");
    id = arraylist_size(dialogs);
    arraylist_add(dialogs, dialog);

    return id;
}

function dialogue_internal_read_color(node, rgba) {
    let rgb = [0];
    if (vertexprops_parse_hex(node.getAttribute("rgb"), rgb, 0)) {
        math2d_color_bytes_to_floats(rgb[0], 0, rgba);
        rgba[3] = vertexprops_parse_float(node, "alpha", 1.0);
    } else {
        rgba[0] = vertexprops_parse_float(node, "r", rgba[0]);
        rgba[1] = vertexprops_parse_float(node, "g", rgba[1]);
        rgba[2] = vertexprops_parse_float(node, "b", rgba[2]);
        rgba[3] = vertexprops_parse_float(node, "a", rgba[3]);
    }
}

function dialogue_internal_read_align(node, attribute) {
    let unparsed_align = node.getAttribute(attribute);
    if (!unparsed_align || unparsed_align == "none") {
        return ALIGN_NONE;
    }

    let align = vertexprops_parse_align2(unparsed_align);
    switch (align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            console.error("dialogue_internal_read_align() invalid align value: " + unparsed_align);
            align = ALIGN_NONE;
            break;
    }

    return align;
}


function dialogue_internal_read_offset(json_obj, property, offset) {
    let json_array = json_read_array(json_obj, property);
    offset[0] = json_read_array_item_number(json_array, 0, 0.0);
    offset[1] = json_read_array_item_number(json_array, 1, 0.0);
}

function dialogue_internal_compute_state(mdlhldr, sttsprt, anim, offst, is_speak) {
    if (!anim) return;

    let name = is_speak ? DIALOGUE_SPEAK : DIALOGUE_IDLE;
    let state = statesprite_state_add2(sttsprt, modelholder_get_texture(mdlhldr, 1), anim, null, 0x00, name);

    console.assert(state != null);

    state.offset_x = offst[0];
    state.offset_y = offst[1];
}

function dialogue_internal_add_state(statesprite, modelholder, anim_name, state_name, scale, looped) {
    if (!anim_name) return;

    let state = statesprite_state_add(statesprite, modelholder, anim_name, state_name);
    if (!state) return;

    if (state.animation != null && !modelholder_has_animlist(modelholder)) {
        let animlist = modelholder_get_animlist(modelholder);
        if (animlist_get_animation(animlist, anim_name) == null) {
            // the animation was builded from an atlas, explicit set the loop count
            animsprite_set_loop(state.animation, looped ? -1 : 1);
        }
    }

    const orig_size = [-1, -1];

    imgutils_get_statesprite_original_size(state, orig_size);
    state.draw_width = orig_size[0] * scale;
    state.draw_height = orig_size[1] * scale;
}

function dialogue_internal_stop_portrait_animation(portrait) {
    let state = statesprite_state_get(portrait.statesprite);
    if (state && state.animation) {
        animsprite_force_end3(state.animation, portrait.statesprite);
        animsprite_stop(state.animation);
    }
}


function dialogue_internal_get_audio(dialogue, name) {
    if (!name && dialogue.audios_size > 0) {
        // random choose
        let index = math2d_random_int(0, dialogue.audios_size - 1);
        return dialogue.audios[index];
    }

    for (let i = 0; i < dialogue.audios_size; i++) {
        if (dialogue.audios[i].name == name) {
            return dialogue.audios[i];
        }
    }
    return null;
}

function dialogue_internal_get_font(dialogue, name) {
    if (!name && dialogue.fonts_size > 0) {
        // random choose
        let index = math2d_random_int(0, dialogue.fonts_size - 1);
        return dialogue.fonts[index].fontholder;
    }

    for (let i = 0; i < dialogue.fonts_size; i++) {
        if (dialogue.fonts[i].name == name) {
            return dialogue.fonts[i].fontholder;
        }
    }
    return null;
}

function dialogue_internal_get_background_index(dialogue, name) {
    if (!name && dialogue.backgrounds_size > 0) {
        // random choose
        return math2d_random_int(0, dialogue.backgrounds_size - 1);
    }

    for (let i = 0; i < dialogue.backgrounds_size; i++) {
        if (dialogue.backgrounds[i].name == name) {
            return i;
        }
    }
    return -1;
}

function dialogue_internal_get_state(dialogue, name) {
    for (let i = 0; i < dialogue.states_size; i++) {
        if (dialogue.states[i].name == name) {
            return dialogue.states[i];
        }
    }
    return null;
}

function dialogue_internal_get_multiplechoice(dialogue, name) {
    if (!name && dialogue.multiplechoices_size > 0) {
        // random choose
        let index = math2d_random_int(0, dialogue.multiplechoices_size - 1);
        return dialogue.multiplechoices[index];
    }

    for (let i = 0; i < dialogue.multiplechoices_size; i++) {
        if (dialogue.multiplechoices[i].name == name) {
            return dialogue.multiplechoices[i];
        }
    }
    return null;
}

function _dialogue_lua_call(luascript_ptr, fn_name, ...args) {
    // JS only
    let allocated_strings = [];
    for (let i = 0; i < args.length; i++) {
        if (typeof (args[i]) === "string" || args[i] === null) {
            let ptr = ModuleLuaScript.kdmyEngine_stringToPtr(args[i]);
            allocated_strings.push(ptr);
            args[i] = ptr;
        }
    }

    args.unshift(luascript_ptr);

    ModuleLuaScript[`_${fn_name}`].apply(null, args);

    if (ModuleLuaScript.kdmyEngine_hasAsyncPending()) {
        console.warn(
            "_dialogue_lua_call() the call to " +
            fn_name +
            " or a previous lua call was was asynchronous, this will result in out-of-order execution of lua code"
        );
    }

    // this can lead to read-after-free if the call was async
    for (let ptr of allocated_strings) ModuleLuaScript.kdmyEngine_deallocate(ptr);
}

