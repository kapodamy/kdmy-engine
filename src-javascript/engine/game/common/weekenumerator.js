"use strict";

const FUNKIN_WEEKS_FOLDER = "/assets/weeks/";
const FUNKIN_WEEK_ABOUT_FILE = "about.json";
const FUNKIN_WEEK_GREETINGS_FILE = "weekGreetings.txt";
const FUNKIN_WEEK_FOLDER_SEPARATOR = "/";
const FUNKIN_WEEK_HOST_FILENAME = "host";
const FUNKIN_WEEK_TITLE_FILENAME = "title.png";

//const FUNKIN_WEEK_ABOUT_FOLDER = "/about/";
//const FUNKIN_WEEK_CUSTOM_FOLDER = "custom";
const FUNKIN_WEEK_GAMEPLAY_MANIFEST = "gameplay.json";
const FUNKIN_WEEK_SONGS_FOLDER = "songs/";
const FUNKIN_WEEK_CHARTS_FOLDER = "charts/";


async function weekenumerator_enumerate() {
    fs_folder_stack_push();
    fs_set_working_folder(FUNKIN_WEEKS_FOLDER, 0);

    let folder_enumerator = { name: null, is_file: 0, is_folder: 0 };
    let weeks_path = expansions_overrided_weeks_folder ?? FUNKIN_WEEKS_FOLDER;

    if (!await fs_folder_enumerate(weeks_path, folder_enumerator)) {
        console.error(`weekenumerator_enumerate() can not read ${weeks_path}`);
        fs_folder_stack_pop();
        return;
    }

    let parsed_weeks = linkedlist_init();

    while (fs_folder_enumerate_next(folder_enumerator)) {
        if (folder_enumerator.is_file) continue;
        let folder_name = folder_enumerator.name;

        fs_set_working_subfolder(folder_name);
        let parsed_week = await weekenumerator_parse_week(folder_name);
        if (parsed_week) linkedlist_add_item(parsed_weeks, parsed_week);
        fs_set_working_folder(FUNKIN_WEEKS_FOLDER, 0);
    }

    fs_folder_enumerate_close(folder_enumerator);

    weeks_array.size = linkedlist_count(parsed_weeks);
    weeks_array.array = linkedlist_to_array(parsed_weeks);

    // In C all string on the list must be disposed
    linkedlist_destroy2(parsed_weeks, free);
    fs_folder_stack_pop();
}

function weekenumerator_get_host_model(weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, FUNKIN_WEEK_HOST_FILENAME
    );
}

function weekenumerator_get_title_texture(weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, FUNKIN_WEEK_TITLE_FILENAME
    );
}

function weekenumerator_get_week_folder(weekinfo) {
    return string_concat(2, FUNKIN_WEEKS_FOLDER, weekinfo.name);
}

/*function weekenumerator_get_custom_folder(weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, FUNKIN_WEEK_CUSTOM_FOLDER
    );
}*/

function weekenumerator_get_gameplay_manifest(weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, FUNKIN_WEEK_GAMEPLAY_MANIFEST
    );
}

function weekenumerator_get_greetings(weekinfo) {
    if (weekinfo.has_greetings) {
        return string_concat(
            4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, FUNKIN_WEEK_GREETINGS_FILE
        );
    }
    return null;
}

function weekenumerator_get_asset(weekinfo, relative_path) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo.name, FS_CHAR_SEPARATOR, relative_path
    );
}

async function weekenumerator_parse_week(week_name) {
    let week_parsed = null;

    try {
        let json = await json_load_from(FUNKIN_WEEK_ABOUT_FILE);

        if (!json_has_property_array(json, "songs")) {
            throw new Error("missing or invalid songs in week: " + week_name);
        }


        let has_difficulty_easy = json_read_boolean(json, "hasDifficultyEasy", 1);
        let has_difficulty_normal = json_read_boolean(json, "hasDifficultyNormal", 1);
        let has_difficulty_hard = json_read_boolean(json, "hasDifficultyHard", 1);

        let customdifficulties_array = json_read_array(json, "customDifficulties");
        let customdifficulties_array_size = json_read_array_length(customdifficulties_array);

        if (!has_difficulty_easy &&
            !has_difficulty_normal &&
            !has_difficulty_hard &&
            customdifficulties_array_size < 1) {
            throw new Error("There no difficults in week: " + week_name);
        }

        let customdifficults_model = await weekenumerator_parse_path(json, "customDifficultsModel");
        if (!customdifficults_model && customdifficulties_array_size > 0) {
            throw new Error("Missing or invalid custom difficult model");
        }

        // parse all custom difficults
        let customdifficults = null;
        if (customdifficulties_array_size > 0) {
            customdifficults = new Array(customdifficulties_array_size);
            for (let i = 0; i < customdifficulties_array_size; i++) {
                let customdifficult_obj = json_read_array_item_object(customdifficulties_array, i);

                customdifficults[i] = {
                    name: json_read_string(customdifficult_obj, "name", null),
                    unlock_directive: json_read_string(customdifficult_obj, "unlockDirective", null)
                };

                if (!customdifficults[i].name) {
                    console.error("weekenumerator_parse_week() difficult name in week: " + week_name);
                } else if (fs_is_invalid_filename(customdifficults[i].name)) {
                    console.error(
                        "weekenumerator_parse_week() forbidden difficult name: " + customdifficults[i].name
                    );
                    customdifficults[i].name = undefined;
                    customdifficults[i].name = null;
                }
            }
        } else {
            customdifficulties_array_size = 0;
        }

        let json_songs = json_read_array(json, "songs");
        let json_songs_size = json_read_array_length(json_songs);
        let songs = new Array(json_songs_size);
        for (let i = 0; i < json_songs_size; i++) {
            let json_song = json_read_array_item_object(json_songs, i);
            songs[i] = {
                name: json_read_string(json_song, "name", null),
                freeplay_host_icon_model: await weekenumerator_parse_path(json_song, "freeplayHostIconModel"),
                freeplay_host_icon_name: json_read_string(json_song, "freeplayHostIconName", null),
                freeplay_locked_host_icon_model: await weekenumerator_parse_path(json_song, "freeplayLockedHostIconModel"),
                freeplay_locked_host_icon_name: json_read_string(json_song, "freeplayLockedHostIconName", null),
                freeplay_background: json_read_string(json_song, "freeplayBackground", null),
                freeplay_only: json_read_boolean(json_song, "freeplayOnly", false),
                freeplay_unlock_directive: json_read_string(json_song, "freeplayUnlockDirective", null),
                freeplay_hide_if_week_locked: json_read_boolean(json_song, "freeplayHideIfWeekLocked", 0),
                freeplay_hide_if_locked: json_read_boolean(json_song, "freeplayHideIfLocked", 0),
                freeplay_gameplaymanifest: json_read_string(json_song, "freeplayGameplayManifest", null),
                freeplay_song_index_in_gameplaymanifest: json_read_number(json_song, "freeplaySongIndexInGameplayManifest", -1),
                freeplay_song_filename: await weekenumerator_parse_path(json_song, "freeplaySongFilename"),
                freeplay_description: json_read_string(json_song, "freeplayDescription", null),
                freeplay_seek_time: json_read_number(json_song, "freeplaySeekTime", NaN)
            }
        }

        let unlockables_json = json_read_object(json, "unlockables");
        let boyfriend_array = json_read_array(unlockables_json, "boyfriend");
        let girlfriend_array = json_read_array(unlockables_json, "girlfriend");

        week_parsed = {
            name: strdup(week_name),
            display_name: json_read_string(json, "displayName", null),
            description: json_read_string(json, "description", null),
            week_title_model: await weekenumerator_parse_path(json, "weekTitleModel"),
            week_title_model_animation_name: json_read_string(json, "weekTitleModelAnimationName", null),
            host_flip_sprite: json_read_boolean(json, "hostFlipSprite", false),
            host_enable_beat: json_read_boolean(json, "hostEnableBeat", true),
            host_hide_if_week_locked: json_read_boolean(json, "hostHideIfWeekLocked", false),
            week_host_character_manifest: await weekenumerator_parse_path(json, "hostCharacterManifest"),
            week_host_model: await weekenumerator_parse_path(json, "hostModel"),
            week_host_model_idle_animation_name: json_read_string(json, "hostModelIdleAnimationName", null),
            week_host_model_choosen_animation_name: json_read_string(json, "hostModelChoosenAnimationName", null),
            songs: songs,
            songs_count: json_songs_size,
            selector_background_color: json_read_hex(json, "selectorBackgroundColor", 0x000000),
            selector_background_color_override: json_has_property_hex(json, "selectorBackgroundColor"),
            has_difficulty_easy: has_difficulty_easy,
            has_difficulty_normal: has_difficulty_normal,
            has_difficulty_hard: has_difficulty_hard,
            custom_difficults_model: customdifficults_model,
            custom_difficults: customdifficults,
            custom_difficults_size: customdifficulties_array_size,
            default_difficulty: json_read_string(json, "customDifficultyDefault", null),
            unlock_directive: json_read_string(json, "unlockDirectiveName", null),
            emit_directive: json_read_string(json, "emitUnlockDirectiveName", null),
            warning_message: json_read_string(json, "warningMessage", null),
            sensible_content_message: json_read_string(json, "sensibleContentMessage", null),
            disallow_custom_boyfriend: json_read_boolean(json, "disallowCustomBoyfriend", false),
            disallow_custom_girlfriend: json_read_boolean(json, "disallowCustomGirlfriend", false),
            unlockables: {
                boyfriend_models: await weekenumerator_parse_characters(boyfriend_array),
                boyfriend_models_size: Math.max(json_read_array_length(boyfriend_array), 0),
                girlfriend_models: await weekenumerator_parse_characters(girlfriend_array),
                girlfriend_models_size: Math.max(json_read_array_length(girlfriend_array), 0)
            },
            custom_selector_layout: await weekenumerator_parse_path(json, "backgroundLayout"),
            custom_folder: await weekenumerator_parse_path(json, "customFolder"),
            custom_folder_gameplay: await weekenumerator_parse_path(json, "inGameplayCustomFolder"),
            has_greetings: await fs_file_exists(FUNKIN_WEEK_GREETINGS_FILE),
            songs_default_freeplay_host_icon_model: await weekenumerator_parse_path(json, "songsDefaultFreeplayHostIconModel")
        };

        json_destroy(json);
    } catch (e) {
        console.error("weekenumerator_parse_week()", e);
    }

    return week_parsed;
}

async function weekenumerator_parse_path(json, json_property_name) {
    let path = json_read_string(json, json_property_name, null);
    if (path == null) return null;
    if (path.length < 1) {
        console.error(`weekenumerator_parse_path() invalid '${json_property_name}' found`);
        path = undefined;
        return null;
    }

    if (json_property_name == "freeplaySongFilename") {
        return await songplayer_helper_get_tracks_full_path(path);
    }

    if (path.indexOf(FS_CHAR_SEPARATOR_REJECT, 0) >= 0) {
        console.error(`weekenumerator_parse_path() invalid char separator in: ${path}`);
        path = undefined;
        return null;
    }

    if (path.startsWith(FS_CHAR_SEPARATOR) || path.startsWith(FS_NO_OVERRIDE_COMMON)) {
        return path;// absolute path
    }

    return await fs_get_full_path(path);
}

async function weekenumerator_parse_characters(array_json) {
    let array_json_length = json_read_array_length(array_json);
    if (array_json_length < 1) return null;

    let array = new Array(array_json_length);

    for (let i = 0; i < array_json_length; i++) {
        let array_item = json_read_array_item_object(array_json, i);
        array[i] = {
            hide_if_locked: json_read_boolean(array_item, "hideIfLocked", false),
            unlock_directive: json_read_string(array_item, "unlockDirectiveName", null),
            manifest: await weekenumerator_parse_path(array_item, "manifest"),
            name: json_read_string(array_item, "name", null),
        };
        if (!array[i].name && !array[i].manifest) {
            throw new Error("weekenumerator_parse_character() missing 'name' and/or 'manifest'");
        }
    }

    return array;
}

