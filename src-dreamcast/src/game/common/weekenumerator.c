#include "game/common/weekenumerator.h"

#include "expansions.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "jsonparser.h"
#include "linkedlist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "songplayer.h"
#include "stringutils.h"


/**
 * Contains all weeks definitions found
 */
WeekArray weeks_array = {.array = NULL, .size = -1};

/**
 * Menus style (customized by a week) used across the game
 */
const WeekInfo* custom_style_from_week = NULL;


static WeekInfo* weekenumerator_parse_week(const char* week_name);
static WeekInfoUnlockableCharacter* weekenumerator_parse_characters(JSONToken array_json);


void weekenumerator_enumerate() {
    fs_folder_stack_push();
    fs_set_working_folder(FUNKIN_WEEKS_FOLDER, false);

    FSFolderEnumerator folder_enumerator;
    const char* weeks_path = expansions_overrided_weeks_folder ? expansions_overrided_weeks_folder : FUNKIN_WEEKS_FOLDER;

    if (!(folder_enumerator = fs_folder_enumerate(weeks_path)).__run) {
        logger_error("weekenumerator_enumerate() can not read %s", weeks_path);
        fs_folder_stack_pop();
        return;
    }

    LinkedList parsed_weeks = linkedlist_init();

    FSFolderEnumeratorEntry* entry;
    while (fs_folder_enumerate_next(&folder_enumerator, &entry)) {
        if (entry->length >= 0) continue;
        const char* folder_name = entry->name;

        fs_set_working_subfolder(folder_name);
        WeekInfo* parsed_week = weekenumerator_parse_week(folder_name);
        if (parsed_week) linkedlist_add_item(parsed_weeks, parsed_week);
        fs_set_working_folder(FUNKIN_WEEKS_FOLDER, false);
    }

    fs_folder_enumerate_close(&folder_enumerator);

    weeks_array.size = linkedlist_count(parsed_weeks);
    weeks_array.array = linkedlist_to_solid_array(parsed_weeks, sizeof(WeekInfo));

    linkedlist_destroy2(&parsed_weeks, free_chk);
    fs_folder_stack_pop();
}

char* weekenumerator_get_host_model(const WeekInfo* weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, FUNKIN_WEEK_HOST_FILENAME
    );
}

char* weekenumerator_get_title_texture(const WeekInfo* weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, FUNKIN_WEEK_TITLE_FILENAME
    );
}

char* weekenumerator_get_week_folder(const WeekInfo* weekinfo) {
    return string_concat(2, FUNKIN_WEEKS_FOLDER, weekinfo->name);
}

/*char* weekenumerator_get_custom_folder(const WeekInfo* weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, FUNKIN_WEEK_CUSTOM_FOLDER
    );
}*/

char* weekenumerator_get_gameplay_manifest(const WeekInfo* weekinfo) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, FUNKIN_WEEK_GAMEPLAY_MANIFEST
    );
}

char* weekenumerator_get_greetings(const WeekInfo* weekinfo) {
    if (weekinfo->has_greetings) {
        return string_concat(
            4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, FUNKIN_WEEK_GREETINGS_FILE
        );
    }
    return NULL;
}

char* weekenumerator_get_asset(const WeekInfo* weekinfo, const char* relative_path) {
    return string_concat(
        4, FUNKIN_WEEKS_FOLDER, weekinfo->name, FS_STRING_SEPARATOR, relative_path
    );
}

static WeekInfo* weekenumerator_parse_week(const char* week_name) {
    WeekInfo* week_parsed = NULL;

    JSONToken json = json_load_from(FUNKIN_WEEK_ABOUT_FILE);

    if (!json_has_property_array(json, "songs")) {
        logger_error("missing or invalid songs in week: %s", week_name);
        goto L_failed;
    }


    bool has_difficulty_easy = json_read_boolean(json, "hasDifficultyEasy", true);
    bool has_difficulty_normal = json_read_boolean(json, "hasDifficultyNormal", true);
    bool has_difficulty_hard = json_read_boolean(json, "hasDifficultyHard", true);

    JSONToken customdifficulties_array = json_read_array(json, "customDifficulties");
    int32_t customdifficulties_array_size = json_read_array_length(customdifficulties_array);

    if (!has_difficulty_easy &&
        !has_difficulty_normal &&
        !has_difficulty_hard &&
        customdifficulties_array_size < 1) {
        logger_error("There no difficults in week: %s", week_name);
        goto L_failed;
    }

    char* customdifficults_model = weekenumerator_parse_path(json, "customDifficultsModel");
    if (!customdifficults_model && customdifficulties_array_size > 0) {
        free_chk(customdifficults_model);
        logger_error("Missing or invalid custom difficult model");
        goto L_failed;
    }

    // parse all custom difficults
    WeekInfoCustomDifficult* customdifficults = NULL;
    if (customdifficulties_array_size > 0) {
        customdifficults = malloc_for_array(WeekInfoCustomDifficult, customdifficulties_array_size);
        for (int32_t i = 0; i < customdifficulties_array_size; i++) {
            JSONToken customdifficult_obj = json_read_array_item_object(customdifficulties_array, i);

            customdifficults[i] = (WeekInfoCustomDifficult){
                .name = json_read_string2(customdifficult_obj, "name", NULL),
                .unlock_directive = json_read_string2(customdifficult_obj, "unlockDirective", NULL)
            };

            if (string_is_empty(customdifficults[i].name)) {
                logger_error("weekenumerator_parse_week() difficult name in week: %s", week_name);
            } else if (fs_is_invalid_filename(customdifficults[i].name)) {
                logger_error(
                    "weekenumerator_parse_week() forbidden difficult name: %s", customdifficults[i].name
                );
                free_chk((char*)customdifficults[i].name);
                customdifficults[i].name = NULL;
            }
        }
    } else {
        customdifficulties_array_size = 0;
    }

    JSONToken json_songs = json_read_array(json, "songs");
    int32_t json_songs_size = json_read_array_length(json_songs);
    WeekInfoSong* songs = malloc_for_array(WeekInfoSong, json_songs_size);
    for (int32_t i = 0; i < json_songs_size; i++) {
        JSONToken json_song = json_read_array_item_object(json_songs, i);
        songs[i] = (WeekInfoSong){
            .name = json_read_string2(json_song, "name", NULL),
            .freeplay_host_icon_model = weekenumerator_parse_path(json_song, "freeplayHostIconModel"),
            .freeplay_host_icon_name = json_read_string2(json_song, "freeplayHostIconName", NULL),
            .freeplay_locked_host_icon_model = weekenumerator_parse_path(json_song, "freeplayLockedHostIconModel"),
            .freeplay_locked_host_icon_name = json_read_string2(json_song, "freeplayLockedHostIconName", NULL),
            .freeplay_background = json_read_string2(json_song, "freeplayBackground", NULL),
            .freeplay_only = json_read_boolean(json_song, "freeplayOnly", false),
            .freeplay_unlock_directive = json_read_string2(json_song, "freeplayUnlockDirective", NULL),
            .freeplay_hide_if_week_locked = json_read_boolean(json_song, "freeplayHideIfWeekLocked", false),
            .freeplay_hide_if_locked = json_read_boolean(json_song, "freeplayHideIfLocked", false),
            .freeplay_gameplaymanifest = json_read_string2(json_song, "freeplayGameplayManifest", NULL),
            .freeplay_song_index_in_gameplaymanifest = (int32_t)json_read_number_long(json_song, "freeplaySongIndexInGameplayManifest", -1),
            .freeplay_song_filename = weekenumerator_parse_path(json_song, "freeplaySongFilename"),
            .freeplay_description = json_read_string2(json_song, "freeplayDescription", NULL),
            .freeplay_seek_time = (float)json_read_number_double(json_song, "freeplaySeekTime", FLOAT_NaN)
        };
    }

    JSONToken unlockables_json = json_read_object(json, "unlockables");
    JSONToken boyfriend_array = json_read_array(unlockables_json, "boyfriend");
    JSONToken girlfriend_array = json_read_array(unlockables_json, "girlfriend");

    week_parsed = malloc_chk(sizeof(WeekInfo));
    malloc_assert(week_parsed, WeekInfo);

    *week_parsed = (WeekInfo){
        .name = string_duplicate(week_name),
        .display_name = json_read_string2(json, "displayName", NULL),
        .description = json_read_string2(json, "description", NULL),
        .week_title_model = weekenumerator_parse_path(json, "weekTitleModel"),
        .week_title_model_animation_name = json_read_string2(json, "weekTitleModelAnimationName", NULL),
        .host_flip_sprite = json_read_boolean(json, "hostFlipSprite", false),
        .host_enable_beat = json_read_boolean(json, "hostEnableBeat", true),
        .host_hide_if_week_locked = json_read_boolean(json, "hostHideIfWeekLocked", false),
        .week_host_character_manifest = weekenumerator_parse_path(json, "hostCharacterManifest"),
        .week_host_model = weekenumerator_parse_path(json, "hostModel"),
        .week_host_model_idle_animation_name = json_read_string2(json, "hostModelIdleAnimationName", NULL),
        .week_host_model_choosen_animation_name = json_read_string2(json, "hostModelChoosenAnimationName", NULL),
        .songs = songs,
        .songs_count = json_songs_size,
        .selector_background_color = json_read_hex(json, "selectorBackgroundColor", 0x000000),
        .selector_background_color_override = json_has_property_hex(json, "selectorBackgroundColor"),
        .has_difficulty_easy = has_difficulty_easy,
        .has_difficulty_normal = has_difficulty_normal,
        .has_difficulty_hard = has_difficulty_hard,
        .custom_difficults_model = customdifficults_model,
        .custom_difficults = customdifficults,
        .custom_difficults_size = customdifficulties_array_size,
        .default_difficulty = json_read_string2(json, "customDifficultyDefault", NULL),
        .unlock_directive = json_read_string2(json, "unlockDirectiveName", NULL),
        .emit_directive = json_read_string2(json, "emitUnlockDirectiveName", NULL),
        .warning_message = json_read_string2(json, "warningMessage", NULL),
        .sensible_content_message = json_read_string2(json, "sensibleContentMessage", NULL),
        .disallow_custom_boyfriend = json_read_boolean(json, "disallowCustomBoyfriend", false),
        .disallow_custom_girlfriend = json_read_boolean(json, "disallowCustomGirlfriend", false),
        .unlockables = (WeekInfoUnlockables){
            .boyfriend_models = weekenumerator_parse_characters(boyfriend_array),
            .boyfriend_models_size = math2d_max_int(json_read_array_length(boyfriend_array), 0),
            .girlfriend_models = weekenumerator_parse_characters(girlfriend_array),
            .girlfriend_models_size = math2d_max_int(json_read_array_length(girlfriend_array), 0)
        },
        .custom_selector_layout = weekenumerator_parse_path(json, "customSelectorLayout"),
        .custom_folder = weekenumerator_parse_path(json, "customFolder"),
        .custom_folder_gameplay = weekenumerator_parse_path(json, "inGameplayCustomFolder"),
        .has_greetings = fs_file_exists(FUNKIN_WEEK_GREETINGS_FILE),
        .songs_default_freeplay_host_icon_model = weekenumerator_parse_path(json, "songsDefaultFreeplayHostIconModel")
    };

    if (week_parsed->unlockables.boyfriend_models_size > 0 && !week_parsed->unlockables.boyfriend_models) {
        week_parsed->unlockables.boyfriend_models_size = 0;
    }
    if (week_parsed->unlockables.girlfriend_models_size > 0 && !week_parsed->unlockables.girlfriend_models) {
        week_parsed->unlockables.girlfriend_models_size = 0;
    }

    json_destroy(&json);
    return week_parsed;

L_failed:
    json_destroy(&json);
    return NULL;
}

char* weekenumerator_parse_path(JSONToken json, const char* json_property_name) {
    const char* path = json_read_string(json, json_property_name, NULL);
    if (path == NULL) return NULL;
    if (strlen(path) < 1) {
        logger_error("weekenumerator_parse_path() invalid %s found", json_property_name);
        return NULL;
    }

    if (string_equals(json_property_name, "freeplaySongFilename")) {
        return songplayer_helper_get_tracks_full_path(path);
    }

    if (string_index_of_char(path, 0, FS_CHAR_SEPARATOR_REJECT) >= 0) {
        logger_error("weekenumerator_parse_path() invalid char separator in: %s", path);
        return NULL;
    }

    if (string_starts_with(path, FS_STRING_SEPARATOR) || string_starts_with(path, FS_NO_OVERRIDE_COMMON)) {
        return string_duplicate(path); // absolute path
    }

    return fs_get_full_path(path);
}

static WeekInfoUnlockableCharacter* weekenumerator_parse_characters(JSONToken array_json) {
    int32_t array_json_length = json_read_array_length(array_json);
    if (array_json_length < 1) return NULL;

    WeekInfoUnlockableCharacter* array = malloc_for_array(WeekInfoUnlockableCharacter, array_json_length);
    for (int32_t i = 0; i < array_json_length; i++) {
        JSONToken array_item = json_read_array_item_object(array_json, i);
        array[i] = (WeekInfoUnlockableCharacter){
            .hide_if_locked = json_read_boolean(array_item, "hideIfLocked", false),
            .unlock_directive = json_read_string2(array_item, "unlockDirectiveName", NULL),
            .manifest = weekenumerator_parse_path(array_item, "manifest"),
            .name = json_read_string2(array_item, "name", NULL),
        };
        if (!array[i].name && !array[i].manifest) {
            logger_warn("weekenumerator_parse_character() missing 'name' and/or 'manifest'");
            for (; i >= 0; i--) {
                free_chk((char*)array[i].unlock_directive);
                free_chk((char*)array[i].name);
                free_chk((char*)array[i].manifest);
            }
            free_chk(array);
            return NULL;
        }
    }

    return array;
}
