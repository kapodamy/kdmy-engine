#include "game/gameplay/helpers/gameplaymanifest.h"

#include "fs.h"
#include "game/common/funkin.h"
#include "game/funkin/healthbar.h"
#include "jsonparser.h"
#include "malloc_utils.h"
#include "stringbuilder.h"
#include "stringutils.h"


static const char* GAMEPLAYMANIFEST_DEFAULT_GIRLFRIEND = "#girlfriend";
static const char* GAMEPLAYMANIFEST_DEFAULT_BOYFRIEND = "#boyfriend";


static const GameplayManifestDefault GAMEPLAYMANIFEST_DEFAULT = {
    .distributions = (Distribution*)&STRUMS_DEFAULT_DISTRIBUTION,
    .distributions_size = 1,
    .girlfriend = &(GameplayManifestGirlfriend){
        .refer = GameplayManifestRefer_GIRLFRIEND,
        .manifest = NULL, // "#girlfriend"
        .states = NULL,
        .states_size = 0,
    },
    .healthbar = &(GameplayManifestHealthBar){
        .has_disable = false,
        .disable = false,

        .has_use_alt_icons = false,
        .use_alt_icons = false,

        .has_opponent_color = false,
        .opponent_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_DAD,

        .has_player_color = false,
        .player_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_DAD,

        .states = NULL,
        .states_size = 0,

        .has_warnings_model = true,
        .warnings_model = HEALTHBAR_WARNING_MODEL,
    },
    .stage = NULL,
    .script = NULL,
    .players = NULL,
    .players_size = 0,
    .ui_layout = NULL,
    .pause_menu = NULL,
    .dialogue_params = NULL
};


static void gameplaymanifest_destroy_distributions(Distribution* distributions, int32_t distributions_size);
static void gameplaymanifest_destroy_healthbar(GameplayManifestHealthBar* healthbar);
static void gameplaymanifest_destroy_girlfriend(GameplayManifestGirlfriend* girlfriend);
static void gameplaymanifest_destroy_players(GameplayManifestPlayer* players, int32_t players_size);
static void gameplaymanifest_parse_song(GameplayManifestSong* song, JSONToken json_song, int32_t players_count);
static void gameplaymanifest_parse_notes(JSONToken json, Distribution* distribution);
static void gameplaymanifest_parse_distributions(JSONToken json, Distribution** distributions, int32_t* distributions_size);
static void gameplaymanifest_parse_distribution(JSONToken json_distribution, Distribution* distribution);
static void gameplaymanifest_parse_distributions_minimal(JSONToken json, Distribution** distributions, int32_t* distributions_size);
static void gameplaymanifest_parse_distributions_models(JSONToken json, Distribution** distributions, int32_t* distributions_size);
static GameplayManifestHealthBar* gameplaymanifest_parse_healthbar(JSONToken json);
static void gameplaymanifest_parse_healthbar_states(GameplayManifestHealthBarStateEntry* healthbar_state, JSONToken json_state, const char* property);
static void gameplaymanifest_parse_players(JSONToken json, GameplayManifestPlayer** players_out, int32_t* players_size_ptr);
static GameplayManifestGirlfriend* gameplaymanifest_parse_girlfriend(JSONToken json);
static GameplayManifestRefer gameplaymanifest_has_default_model(const char* charactermanifest_src);


GameplayManifest gameplaymanifest_init(const char* src) {
    JSONToken json = json_load_from(src);
    if (!json) return NULL;

    GameplayManifest manifest = malloc_chk(sizeof(struct GameplayManifest_s));
    malloc_assert(manifest, GameplayManifest);

    *manifest = (struct GameplayManifest_s){
        .default_ = (GameplayManifestDefault*)&GAMEPLAYMANIFEST_DEFAULT,
        .songs = NULL,
        .songs_size = 0
    };

    if (json_has_property_object(json, "default")) {
        /*if (json_is_property_null(json, "default")) {
            logger_error("'default' property can not be NULL. File: %s", src);
        }*/

        manifest->default_ = malloc_chk(sizeof(GameplayManifestDefault));
        malloc_assert(manifest->default_, GameplayManifestDefault);

        *manifest->default_ = (GameplayManifestDefault){
            .distributions = GAMEPLAYMANIFEST_DEFAULT.distributions,
            .distributions_size = GAMEPLAYMANIFEST_DEFAULT.distributions_size,

            .girlfriend = GAMEPLAYMANIFEST_DEFAULT.girlfriend,
            .healthbar = GAMEPLAYMANIFEST_DEFAULT.healthbar,

            .stage = NULL,
            .script = NULL,

            .players = NULL,
            .players_size = 0,
            .ui_layout = NULL,
            .pause_menu = NULL,
            .dialogue_params = NULL
        };

        JSONToken json_default = json_read_object(json, "default");

        if (json_has_property_array(json_default, "distributions"))
            gameplaymanifest_parse_distributions(json_default, &manifest->default_->distributions, &manifest->default_->distributions_size);
        else if (json_has_property_array(json_default, "distributionsMinimal"))
            gameplaymanifest_parse_distributions_minimal(json_default, &manifest->default_->distributions, &manifest->default_->distributions_size);
        else if (json_has_property_object(json_default, "distributionsModels"))
            gameplaymanifest_parse_distributions_models(json_default, &manifest->default_->distributions, &manifest->default_->distributions_size);

        gameplaymanifest_parse_players(json_default, &manifest->default_->players, &manifest->default_->players_size);

        if (json_has_property_object(json_default, "healthbar")) {
            manifest->default_->healthbar = gameplaymanifest_parse_healthbar(json_default);
        }
        if (json_has_property_object(json_default, "girlfriend")) {
            manifest->default_->girlfriend = gameplaymanifest_parse_girlfriend(json_default);
        }

        manifest->default_->stage = json_read_string2(json_default, "stage", NULL);
        manifest->default_->script = json_read_string2(json_default, "script", NULL);
        manifest->default_->ui_layout = json_read_string2(json_default, "UILayout", NULL);
        manifest->default_->pause_menu = json_read_string2(json_default, "pauseMenu", NULL);
        manifest->default_->dialogue_params = json_read_string2(json_default, "dialogueParams", NULL);
    }

    JSONToken songs_array = json_read_array(json, "songs");
    int32_t songs_array_size = json_read_array_length(songs_array);

    if (songs_array_size < 0) {
        json_destroy(&json);
        return manifest;
    }

    manifest->songs_size = songs_array_size;
    manifest->songs = malloc_for_array(GameplayManifestSong, songs_array_size);

    int32_t players_count = manifest->default_ ? manifest->default_->players_size : -1;

    for (int32_t i = 0; i < songs_array_size; i++) {
        JSONToken json_songs = json_read_array_item_object(songs_array, i);

        gameplaymanifest_parse_song(&manifest->songs[i], json_songs, players_count);

        // check if the girlfriend manifest in the current song is NULL
        GameplayManifestGirlfriend* song_girlfriend = manifest->songs[i].girlfriend;
        if (manifest->default_ && manifest->default_->girlfriend) {
            if (song_girlfriend && song_girlfriend->manifest == NULL) {
                song_girlfriend->manifest = string_duplicate(manifest->default_->girlfriend->manifest);
            }
        }
    }

    json_destroy(&json);
    return manifest;
}

void gameplaymanifest_destroy(GameplayManifest* gameplaymanifest_ptr) {
    if (!gameplaymanifest_ptr || !*gameplaymanifest_ptr) return;

    GameplayManifest gameplaymanifest = *gameplaymanifest_ptr;

    if (gameplaymanifest->default_ && gameplaymanifest->default_ != &GAMEPLAYMANIFEST_DEFAULT) {
        GameplayManifestDefault* manifest = gameplaymanifest->default_;

        gameplaymanifest_destroy_distributions(manifest->distributions, manifest->distributions_size);
        gameplaymanifest_destroy_girlfriend(manifest->girlfriend);
        gameplaymanifest_destroy_healthbar(manifest->healthbar);
        gameplaymanifest_destroy_players(manifest->players, manifest->players_size);

        free_chk(manifest->stage);
        free_chk(manifest->script);
        free_chk(manifest->ui_layout);
        free_chk(manifest->pause_menu);
        free_chk(manifest->dialogue_params);

        free_chk(gameplaymanifest->default_);
    }

    for (int32_t i = 0; i < gameplaymanifest->songs_size; i++) {
        GameplayManifestSong* song = &gameplaymanifest->songs[i];

        free_chk(song->name);
        free_chk(song->file);
        free_chk(song->chart);
        free_chk(song->stage);
        free_chk(song->script);
        free_chk(song->ui_layout);
        free_chk(song->pause_menu);
        free_chk(song->dialogue_params);
        free_chk(song->dialog_text);
        free_chk(song->selected_state_name);

        for (int32_t j = 0; j < song->selected_state_name_per_player_size; j++) {
            free_chk(song->selected_state_name_per_player[i]);
        }
        free_chk(song->selected_state_name_per_player);

        gameplaymanifest_destroy_healthbar(song->healthbar);
        gameplaymanifest_destroy_girlfriend(song->girlfriend);
        gameplaymanifest_destroy_distributions(song->distributions, song->distributions_size);
        gameplaymanifest_destroy_players(song->players, song->players_size);
    }
    free_chk(gameplaymanifest->songs);

    free_chk(gameplaymanifest);
    *gameplaymanifest_ptr = NULL;
}


static void gameplaymanifest_destroy_distributions(Distribution* distributions, int32_t distributions_size) {
    if (!distributions) return;
    if (distributions == GAMEPLAYMANIFEST_DEFAULT.distributions) return;

    for (int32_t i = 0; i < distributions_size; i++) {
        if (&distributions[i] == &STRUMS_DEFAULT_DISTRIBUTION) continue;

        if (distributions[i].notes != STRUMS_DEFAULT_DISTRIBUTION.notes) {
            for (int32_t j = 0; j < distributions[i].notes_size; j++) {
                free_chk(distributions[i].notes[j].name);
                free_chk(distributions[i].notes[j].custom_sick_effect_model_src);
            }
            free_chk(distributions[i].notes);
        }

        if (distributions[i].strums != STRUMS_DEFAULT_DISTRIBUTION.strums) {
            for (int32_t j = 0; j < distributions[i].strums_size; j++) {
                free_chk(distributions[i].strums[j].name);
                free_chk(distributions[i].strums[j].notes_ids);
            }
            free_chk(distributions[i].strums);
        }

        if (distributions[i].strum_binds_is_custom) {
            free_chk(distributions[i].strum_binds);
        }

        if (distributions[i].states != STRUMS_DEFAULT_DISTRIBUTION.states) {
            for (int32_t j = 0; j < distributions[i].states_size; j++) {
                free_chk(distributions[i].states[j].name);
                free_chk(distributions[i].states[j].model_marker);
                free_chk(distributions[i].states[j].model_sick_effect);
                free_chk(distributions[i].states[j].model_background);
                free_chk(distributions[i].states[j].model_notes);
            }
            free_chk(distributions[i].states);
        }
    }

    free_chk(distributions);
}

static void gameplaymanifest_destroy_healthbar(GameplayManifestHealthBar* healthbar) {
    if (!healthbar) return;
    if (healthbar == GAMEPLAYMANIFEST_DEFAULT.healthbar) return;

    for (int32_t i = 0; i < healthbar->states_size; i++) {
        free_chk(healthbar->states[i].name);

        free_chk(healthbar->states[i].opponent.icon_model);
        free_chk(healthbar->states[i].opponent.bar_model);

        free_chk(healthbar->states[i].player.icon_model);
        free_chk(healthbar->states[i].player.bar_model);

        free_chk(healthbar->states[i].background.icon_model);
        free_chk(healthbar->states[i].background.bar_model);
    }

    free_chk(healthbar->states);
    free_chk(healthbar->warnings_model);
    free_chk(healthbar);
}

static void gameplaymanifest_destroy_girlfriend(GameplayManifestGirlfriend* girlfriend) {
    if (!girlfriend) return;
    if (girlfriend == GAMEPLAYMANIFEST_DEFAULT.girlfriend) return;

    for (int32_t i = 0; i < girlfriend->states_size; i++) {
        free_chk(girlfriend->states[i].name);
        free_chk(girlfriend->states[i].model);
    }

    free_chk(girlfriend->states);
    free_chk(girlfriend->manifest);
    free_chk(girlfriend);
}

static void gameplaymanifest_destroy_players(GameplayManifestPlayer* players, int32_t players_size) {
    if (!players) return;

    for (int32_t i = 0; i < players_size; i++) {
        free_chk(players[i].manifest);
        for (int32_t j = 0; j < players[i].states_size; j++) {
            free_chk(players[i].states[j].name);
            free_chk(players[i].states[j].model);
        }
        free_chk(players[i].states);
    }

    free_chk(players);
}


static void gameplaymanifest_parse_song(GameplayManifestSong* song, JSONToken json_song, int32_t players_count) {
    song->name = json_read_string2(json_song, "name", NULL);
    song->file = json_read_string2(json_song, "file", NULL);
    song->chart = json_read_string2(json_song, "chart", NULL);

    if (song->name == NULL && !song->file) {
        logger_error("gameplaymanifest_parse_song() song without 'name' and 'file'");
        assert(song->name != NULL && song->file);
    } else if (song->name != NULL && song->file == NULL && song->chart == NULL) {
        if (fs_is_invalid_filename(song->name)) {
            logger_error("The song '%s' is invalid for filename (file=null chart=null)", song->name);
            assert(!fs_is_invalid_filename(song->name));
        }

        // build the file name and song name using lowercase and without spaces
        // example: "Dad Battle" --> "dadbattle"
        size_t song_name_length = strlen(song->name);
        StringBuilder stringbuilder = stringbuilder_init(song_name_length);
        stringbuilder_add_with_replace(stringbuilder, song->name, "\x20", "");
        stringbuilder_lowercase(stringbuilder);
        char* lowercase_name = stringbuilder_finalize(&stringbuilder);

        song->file = string_concat(3, FUNKIN_WEEK_SONGS_FOLDER, lowercase_name, ".ogg");
        song->chart = string_concat(3, FUNKIN_WEEK_CHARTS_FOLDER, lowercase_name, ".json");
        free_chk(lowercase_name);
    }

    song->stage = json_read_string2(json_song, "stage", NULL);
    song->has_stage = json_has_property(json_song, "stage");

    song->script = json_read_string2(json_song, "script", NULL);
    song->has_script = json_has_property(json_song, "script");

    song->duration = json_read_number_double(json_song, "duration", -1.0);
    if (song->duration >= 0.0) song->duration *= 1000.0; // convert to milliseconds

    song->selected_state_name = json_read_string2(json_song, "selectedStateName", NULL);
    song->has_selected_state_name = json_has_property(json_song, "selectedStateName");

    song->ui_layout = json_read_string2(json_song, "UILayout", NULL);
    song->has_ui_layout = json_has_property(json_song, "UILayout");

    song->dialogue_params = json_read_string2(json_song, "dialogueParams", NULL);
    song->dialog_text = json_read_string2(json_song, "dialogText", NULL);
    song->dialog_ignore_on_freeplay = json_read_boolean(json_song, "dialogIgnoreOnFreeplay", true);

    //
    // Note: disableResourceCacheBetweenSongs default value depends on the platform
    //          C:  true by default on dreamcast (with preloadcache is enough)
    //          C#: false by default on desktop (due slow PNG decoding)
    //          JS: false by default on web (due slow PNG decoding and network load)
    //
    song->disable_resource_cache_between_songs = json_read_boolean(json_song, "disableResourceCacheBetweenSongs", true);

    song->pause_menu = json_read_string2(json_song, "pauseMenu", NULL);
    song->has_pause_menu = json_has_property(json_song, "pauseMenu");

    song->selected_state_name_per_player = NULL;
    song->selected_state_name_per_player_size = 0;

    song->players = NULL;
    song->players_size = 0;

    song->distributions = NULL;
    song->distributions_size = 0;

    song->has_players = json_has_property_array(json_song, "players");
    gameplaymanifest_parse_players(json_song, &song->players, &song->players_size);

    if (json_has_property_array(json_song, "distributions")) {
        song->has_distributions = true;
        gameplaymanifest_parse_distributions(json_song, &song->distributions, &song->distributions_size);
    } else if (json_has_property_array(json_song, "distributionsMinimal")) {
        song->has_distributions = true;
        gameplaymanifest_parse_distributions_minimal(json_song, &song->distributions, &song->distributions_size);
    } else if (json_has_property_array(json_song, "distributionsModels")) {
        song->has_distributions = true;
        gameplaymanifest_parse_distributions_models(json_song, &song->distributions, &song->distributions_size);
    } else {
        song->has_distributions = false;
    }

    song->healthbar = gameplaymanifest_parse_healthbar(json_song);
    song->girlfriend = gameplaymanifest_parse_girlfriend(json_song);
    song->has_girlfriend = json_has_property_object(json_song, "girlfriend");

    JSONToken selected_states_array = json_read_array(json_song, "selectedStateNamePerPlayer");
    int32_t selected_states_array_size = json_read_array_length(selected_states_array);

    if (song->has_players) players_count = song->players_size;

    if (players_count < 0 && selected_states_array_size > 0) {
        logger_error(
            "gameplaymanifest_parse_song() 'selectedStateNamePerPlayer' found without 'players'"
        );
    } else if (selected_states_array_size > 0) {

        if (players_count != selected_states_array_size) {
            logger_warn(
                "gameplaymanifest_parse_song() 'selectedStateNamePerPlayer.length' != 'players.length'"
            );
        } else {
            song->selected_state_name_per_player = malloc_for_array(char*, selected_states_array_size);
            song->selected_state_name_per_player_size = selected_states_array_size;
        }
    }

    for (int32_t i = 0; i < selected_states_array_size; i++) {
        song->selected_state_name_per_player[i] = json_read_array_item_string2(
            selected_states_array, i, NULL
        );
    }
}

static void gameplaymanifest_parse_notes(JSONToken json, Distribution* distribution) {
    JSONToken notes_array = json_read_array(json, "notes");
    int32_t notes_array_size = json_read_array_length(notes_array);

    if (notes_array_size < 0) {
        distribution->notes = NULL;
        distribution->notes_size = 0;
        return;
    }

    DistributionNote* notes = malloc_for_array(DistributionNote, notes_array_size);
    int32_t notes_size = notes_array_size;

    for (int32_t i = 0; i < notes_array_size; i++) {
        notes[i] = (DistributionNote){
            .name = NULL,
            .custom_sick_effect_model_src = NULL,
            .damage_ratio = 1.0,
            .heal_ratio = 1.0,
            .ignore_hit = false,
            .ignore_miss = false,
            .can_kill_on_hit = false,
            .is_special = false,
            .model_src = NULL,
            .sick_effect_state_name = NULL
        };

        JSONToken note_json = json_read_array_item_object(notes_array, i);
        if (!note_json) {
            logger_error("gameplaymanifest_parse_notes() invalid note definition");
            continue;
        }

        notes[i].name = json_read_string2(note_json, "name", NULL);
        notes[i].custom_sick_effect_model_src = json_read_string2(note_json, "customSickEffectModel", NULL);
        notes[i].damage_ratio = (float)json_read_number_double(note_json, "damageRatio", 1.0);
        notes[i].heal_ratio = (float)json_read_number_double(note_json, "healRatio", 1.0);
        notes[i].ignore_hit = json_read_boolean(note_json, "ignoreHit", false);
        notes[i].ignore_miss = json_read_boolean(note_json, "ignoreMiss", false);
        notes[i].can_kill_on_hit = json_read_boolean(note_json, "canKillOnHit", false);
        notes[i].is_special = json_read_boolean(note_json, "isSpecial", false);
        notes[i].model_src = json_read_string2(note_json, "model", NULL);
        notes[i].sick_effect_state_name = json_read_string2(note_json, "sickEffectStateName", NULL);
    }

    distribution->notes = notes;
    distribution->notes_size = notes_size;
}

static void gameplaymanifest_parse_distributions(JSONToken json, Distribution** distributions, int32_t* distributions_size) {
    JSONToken json_distributions = json_read_array(json, "distributions");
    int32_t json_distributions_size = json_read_array_length(json_distributions);

    if (json_distributions_size < 1) {
        *distributions = NULL;
        *distributions_size = 0;
        return;
    }

    Distribution* distributions_new = malloc_for_array(Distribution, json_distributions_size);
    int32_t distributions_size_new = json_distributions_size;

    for (int32_t i = 0; i < json_distributions_size; i++) {
        gameplaymanifest_parse_distribution(
            json_read_array_item_object(json_distributions, i), &distributions_new[i]
        );
    }

    *distributions = distributions_new;
    *distributions_size = distributions_size_new;
}

static void gameplaymanifest_parse_distribution(JSONToken json_distribution, Distribution* distribution) {
    if (!json_distribution) {
        logger_error("Invalid distribution found");
        assert(json_distribution);
    }

    JSONToken strums_array = json_read_array(json_distribution, "strums");
    int32_t strums_array_size = json_read_array_length(strums_array);
    JSONToken strum_binds_array = json_read_array(json_distribution, "buttonBinds");
    int32_t strum_binds_array_size = json_read_array_length(strums_array);


    distribution->notes = NULL;
    distribution->notes_size = 0;

    distribution->strums = NULL;
    distribution->strum_binds = NULL;
    distribution->strums_size = 0;
    distribution->strum_binds_is_custom = false;

    distribution->states = NULL;
    distribution->states_size = 0;


    if (strum_binds_array_size < 0) {
        switch (strums_array_size) {
            case 4:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_4;
                break;
            case 5:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_5;
                break;
            case 6:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_6;
                break;
            case 7:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_7;
                break;
            case 8:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_8;
                break;
            case 9:
                distribution->strum_binds = (GamepadButtons*)BUTTONS_BIND_9;
                break;
            default:
                logger_error("Can not find the button binds for strums size=" FMT_I4, strums_array_size);
                assert(strums_array_size >= 4 && strums_array_size <= 9);
        }
    } else {
        if (strum_binds_array_size != strums_array_size) {
            logger_error("'buttonBinds.length' != 'strums.length'");
        }
        if (strum_binds_array_size > 0) {
            distribution->strum_binds = malloc_for_array(GamepadButtons, strums_array_size);
            distribution->strum_binds_is_custom = true;
            for (int32_t i = 0; i < strums_array_size; i++) {
                distribution->strum_binds[i] = json_read_array_item_hex(strum_binds_array, i, 0x00);
            }
        }
    }

    gameplaymanifest_parse_notes(json_distribution, distribution);

    if (strums_array_size > 0) {
        distribution->strums = malloc_for_array(DistributionStrum, strums_array_size);
        distribution->strums_size = strums_array_size;
    }

    for (int32_t i = 0; i < strums_array_size; i++) {
        JSONToken strum_json = json_read_array_item_object(strums_array, i);

        distribution->strums[i] = (DistributionStrum){
            .name = json_read_string2(strum_json, "name", NULL),
            .notes_ids = NULL,
            .notes_ids_size = 0
        };

        JSONToken notes_ids_array = json_read_array(strum_json, "noteIds");
        int32_t notes_ids_array_size = json_read_array_length(notes_ids_array);

        if (notes_ids_array_size > 0) {
            distribution->strums[i].notes_ids_size = notes_ids_array_size;
            distribution->strums[i].notes_ids = malloc_for_array(int32_t, notes_ids_array_size);
        }

        for (int32_t j = 0; j < notes_ids_array_size; j++) {
            int32_t index = (int32_t)json_read_array_item_number_long(notes_ids_array, j, -1);

            if (index < 0) {
                logger_error("Invalid note index found in strum index=" FMT_I4, i);
                assert(index >= 0);
            } else if (index >= distribution->notes_size) {
                logger_error("gameplaymanifest_parse_strums() invalid 'noteId' index=" FMT_I4, index);
                index = 0;
            }

            distribution->strums[i].notes_ids[j] = index;
        }
    }

    JSONToken states_array = json_read_array(json_distribution, "states");
    int32_t states_array_size = json_read_array_length(states_array);

    if (states_array_size > 0) {
        distribution->states = malloc_for_array(DistributionStrumState, states_array_size);
        distribution->states_size = states_array_size;
    }

    for (int32_t i = 0; i < states_array_size; i++) {
        JSONToken state_json = json_read_array_item_object(states_array, i);
        distribution->states[i] = (DistributionStrumState){
            .name = json_read_string2(state_json, "name", NULL),
            .model_marker = json_read_string2(state_json, "modelMarker", NULL),
            .model_sick_effect = json_read_string2(state_json, "modelSickEffect", NULL),
            .model_background = json_read_string2(state_json, "modelBackground", NULL),
            .model_notes = json_read_string2(state_json, "modelNotes", NULL)
        };
    }
}

static void gameplaymanifest_parse_distributions_minimal(JSONToken json, Distribution** distributions, int32_t* distributions_size) {
    JSONToken json_dists_minimal_array = json_read_array(json, "distributionsMinimal");
    int32_t json_dists_minimal_array_length = json_read_array_length(json_dists_minimal_array);

    if (json_dists_minimal_array_length < 1) {
        *distributions = NULL;
        *distributions_size = 0;
        return;
    }

    Distribution* dists_minimal = malloc_for_array(Distribution, json_dists_minimal_array_length);

    for (int32_t i = 0; i < json_dists_minimal_array_length; i++) {
        JSONToken json_dist = json_read_array_item_object(json_dists_minimal_array, i);

        dists_minimal[i] = (Distribution){
            .notes_size = STRUMS_DEFAULT_DISTRIBUTION.notes_size,
            .notes = STRUMS_DEFAULT_DISTRIBUTION.notes,

            .strums_size = STRUMS_DEFAULT_DISTRIBUTION.strums_size,
            .strums = STRUMS_DEFAULT_DISTRIBUTION.strums,
            .strum_binds = STRUMS_DEFAULT_DISTRIBUTION.strum_binds,
            .states = NULL,
            .states_size = 1
        };

        JSONToken dist_states_array = json_read_array(json_dist, "states");
        int32_t dist_states_array_size = json_read_array_length(dist_states_array);

        if (dist_states_array_size > 0) dists_minimal[i].states_size += dist_states_array_size;
        dists_minimal[i].states = malloc_for_array(DistributionStrumState, dists_minimal[i].states_size);

        // build default_ state
        dists_minimal[i].states[0] = (DistributionStrumState){
            .name = string_duplicate(STRUMS_DEFAULT_DISTRIBUTION.states[0].name),
            .model_marker = string_duplicate(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_marker),
            .model_sick_effect = string_duplicate(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_sick_effect),
            .model_background = string_duplicate(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_background)
        };
        DistributionStrumState* default_state = &dists_minimal[i].states[0];

        // override values in the default_ state
        if (json_has_property_string(json_dist, "modelMarker")) {
            default_state->model_marker = json_read_string2(json_dist, "modelMarker", NULL);
        }
        if (json_has_property_string(json_dist, "modelSickEffect")) {
            default_state->model_sick_effect = json_read_string2(json_dist, "modelSickEffect", NULL);
        }
        if (json_has_property_string(json_dist, "modelBackground")) {
            default_state->model_background = json_read_string2(json_dist, "modelBackground", NULL);
        }

        // read all remaining states
        for (int32_t j = 0; j < dist_states_array_size; j++) {
            JSONToken state_json = json_read_array_item_object(dist_states_array, i);
            dists_minimal[i].states[j + 1] = (DistributionStrumState){
                .name = json_read_string2(state_json, "name", NULL),
                .model_marker = json_read_string2(state_json, "modelMarker", NULL),
                .model_sick_effect = json_read_string2(state_json, "modelSickEffect", NULL),
                .model_background = json_read_string2(state_json, "modelBackground", NULL),
                .model_notes = json_read_string2(state_json, "modelNotes", NULL)
            };
        }
    }

    *distributions = dists_minimal;
    *distributions_size = json_dists_minimal_array_length;
}

static void gameplaymanifest_parse_distributions_models(JSONToken json, Distribution** distributions, int32_t* distributions_size) {
    JSONToken json_obj = json_read_object(json, "distributionsModels");

    char* model_marker = json_read_string2(json_obj, "modelMarker", FUNKIN_COMMON_NOTES);
    char* model_sick_effect = json_read_string2(json_obj, "modelSickEffect", FUNKIN_COMMON_NOTES);
    char* model_background = json_read_string2(json_obj, "modelBackground", FUNKIN_COMMON_NOTES);
    char* model_notes = json_read_string2(json_obj, "modelNotes", FUNKIN_COMMON_NOTES);

    *distributions = malloc_chk(sizeof(Distribution));
    malloc_assert(distributions, Distribution);

    DistributionStrumState* state = malloc_chk(sizeof(DistributionStrumState));
    malloc_assert(state, DistributionStrumState);

    *state = (DistributionStrumState){
        .name = NULL,
        .model_marker = model_marker,
        .model_sick_effect = model_sick_effect,
        .model_background = model_background,
        .model_notes = model_notes
    };

    **distributions = (Distribution){
        .notes = STRUMS_DEFAULT_DISTRIBUTION.notes,
        .notes_size = STRUMS_DEFAULT_DISTRIBUTION.notes_size,
        .states = state,
        .states_size = 1,
        .strums = STRUMS_DEFAULT_DISTRIBUTION.strums,
        .strums_size = STRUMS_DEFAULT_DISTRIBUTION.strums_size,
        .strum_binds = STRUMS_DEFAULT_DISTRIBUTION.strum_binds,
        .strum_binds_is_custom = STRUMS_DEFAULT_DISTRIBUTION.strum_binds_is_custom
    };
    *distributions_size = 1;
}


static GameplayManifestHealthBar* gameplaymanifest_parse_healthbar(JSONToken json) {
    JSONToken json_healthbar = json_read_object(json, "healthbar");
    if (!json_healthbar) return NULL;

    GameplayManifestHealthBar* healthbar = malloc_chk(sizeof(GameplayManifestHealthBar));
    malloc_assert(healthbar, GameplayManifestHealthBar);

    *healthbar = (GameplayManifestHealthBar){
        .disable = false,
        .has_disable = false,

        .opponent_color_rgb8 = 0x00,
        .has_opponent_color = false,

        .player_color_rgb8 = 0x00,
        .has_player_color = false,

        .warnings_model = NULL,
        .has_warnings_model = false,

        .use_alt_icons = false,
        .has_use_alt_icons = false,

        .states = NULL,
        .states_size = 0
    };

    if (json_has_property_boolean(json_healthbar, "disable")) {
        healthbar->disable = json_read_boolean(json_healthbar, "disable", healthbar->disable);
        healthbar->has_disable = true;
    }

    if (json_has_property_hex(json_healthbar, "opponentColor")) {
        healthbar->opponent_color_rgb8 = json_read_hex(json_healthbar, "opponentColor", HEALTHBAR_DEFAULT_COLOR_DAD);
        healthbar->has_opponent_color = true;
    }

    if (json_has_property_hex(json_healthbar, "playerColor")) {
        healthbar->player_color_rgb8 = json_read_hex(json_healthbar, "playerColor", HEALTHBAR_DEFAULT_COLOR_BOYFRIEND);
        healthbar->has_player_color = true;
    }

    if (json_has_property(json_healthbar, "warningsModel")) {
        healthbar->warnings_model = json_read_string2(json_healthbar, "warningsModel", NULL);
        healthbar->has_warnings_model = true;
    }

    if (json_has_property(json_healthbar, "useAltWarnIcons")) {
        healthbar->use_alt_icons = json_read_boolean(json_healthbar, "useAltWarnIcons", false);
        healthbar->has_use_alt_icons = true;
    }


    JSONToken states_array = json_read_array(json_healthbar, "states");
    int32_t states_array_size = json_read_array_length(states_array);

    if (states_array_size < 1) return healthbar;

    healthbar->states = malloc_for_array(GameplayManifestHealthBarState, states_array_size);
    healthbar->states_size = states_array_size;

    for (int32_t i = 0; i < states_array_size; i++) {
        healthbar->states[i] = (GameplayManifestHealthBarState){
            .name = NULL,
            .opponent = (GameplayManifestHealthBarStateEntry){},
            .player = (GameplayManifestHealthBarStateEntry){},
            .background = (GameplayManifestHealthBarStateEntry){}
        };
        GameplayManifestHealthBarState* state = &healthbar->states[i];

        JSONToken state_json = json_read_array_item_object(states_array, i);
        if (!state_json) {
            logger_warn("gameplaymanifest_parse_healthbar_states() invalid healthbar state at " FMT_I4, i);
            continue;
        }

        state->name = json_read_string2(state_json, "name", NULL);
        gameplaymanifest_parse_healthbar_states(&state->opponent, state_json, "opponent");
        gameplaymanifest_parse_healthbar_states(&state->player, state_json, "player");
        gameplaymanifest_parse_healthbar_states(&state->background, state_json, "background");
    }

    return healthbar;
}

static void gameplaymanifest_parse_healthbar_states(GameplayManifestHealthBarStateEntry* healthbar_state, JSONToken json_state, const char* property) {
    JSONToken json = json_read_object(json_state, property);
    healthbar_state->icon_model = json_read_string2(json, "iconModel", NULL);
    healthbar_state->bar_model = json_read_string2(json, "barModel", NULL);
    healthbar_state->bar_color = json_read_hex(json, "barColor", 0x000000);
}


static void gameplaymanifest_parse_players(JSONToken json, GameplayManifestPlayer** players_out, int32_t* players_size_ptr) {
    JSONToken players_array = json_read_array(json, "players");
    int32_t players_array_size = json_read_array_length(players_array);

    if (players_array_size < 1) {
        *players_out = NULL;
        *players_size_ptr = 0;
        return;
    }

    GameplayManifestPlayer* players = malloc_for_array(GameplayManifestPlayer, players_array_size);
    int32_t players_size = players_array_size;

    for (int32_t i = 0; i < players_array_size; i++) {
        JSONToken player_json = json_read_array_item_object(players_array, i);

        players[i] = (GameplayManifestPlayer){
            .manifest = json_read_string2(player_json, "manifest", NULL),
            .refer = GameplayManifestRefer_NONE,
            .controller = (int32_t)json_read_number_long(player_json, "controller", -1),
            .layout_strums_id = (int32_t)json_read_number_long(player_json, "strumsInLayoutId", -1),
            .distribution_index = (int32_t)json_read_number_long(player_json, "distributionIndex", 0),
            .states = NULL,
            .states_size = 0,
            .can_die = false,
            .can_recover = false,
            .is_opponent = false,
        };

        bool has_is_opponent = json_has_property_boolean(player_json, "isOpponent");
        if (has_is_opponent) {
            players[i].is_opponent = json_read_boolean(player_json, "isOpponent", false);
        } else {
            //
            // bot players:   can_die=false  can_recover=false
            // real players:  can_die=true   can_recover=true
            //
            players[i].can_recover = players[i].can_die = players[i].controller >= 0;
            players[i].is_opponent = players[i].controller < 0;
        }

        if (json_has_property_boolean(player_json, "canDie")) {
            players[i].can_die = json_read_boolean(player_json, "canDie", false);
        } else if (has_is_opponent) {
            players[i].can_die = !players[i].is_opponent;
        }

        if (json_has_property_boolean(player_json, "canRecover")) {
            players[i].can_recover = json_read_boolean(player_json, "canRecover", false);
        } else if (has_is_opponent) {
            players[i].can_recover = !players[i].is_opponent;
        }

        players[i].refer = gameplaymanifest_has_default_model(players[i].manifest);
        if (players[i].refer != GameplayManifestRefer_NONE) {
            // a default_ character manifest was specified, ignore
            free_chk(players[i].manifest);
            players[i].manifest = NULL;
        }

        JSONToken states_array = json_read_array(player_json, "states");
        int32_t states_array_size = json_read_array_length(states_array);

        if (states_array_size < 1) continue;

        players[i].states = malloc_for_array(GameplayManifestCharacterState, states_array_size);
        players[i].states_size = states_array_size;

        for (int32_t j = 0; j < states_array_size; j++) {
            JSONToken state_json = json_read_array_item_object(states_array, j);
            players[i].states[j] = (GameplayManifestCharacterState){
                .name = json_read_string2(state_json, "name", NULL),
                .model = json_read_string2(state_json, "model", NULL)
            };
        }
    }

    *players_out = players;
    *players_size_ptr = players_size;
}

static GameplayManifestGirlfriend* gameplaymanifest_parse_girlfriend(JSONToken json) {
    JSONToken json_girlfriend = json_read_object(json, "girlfriend");
    if (!json_girlfriend) return NULL;

    GameplayManifestGirlfriend* girlfriend = malloc_chk(sizeof(GameplayManifestGirlfriend));
    malloc_assert(girlfriend, GameplayManifestGirlfriend);

    *girlfriend = (GameplayManifestGirlfriend){
        .manifest = NULL,
        .refer = GameplayManifestRefer_NONE,
        .states = NULL,
        .states_size = 0
    };

    girlfriend->manifest = json_read_string2(json_girlfriend, "manifest", NULL);
    girlfriend->refer = gameplaymanifest_has_default_model(girlfriend->manifest);

    if (girlfriend->refer != GameplayManifestRefer_NONE) {
        // a default_ character manifest was specified, ignore
        free_chk(girlfriend->manifest);
        girlfriend->manifest = NULL;
    }

    JSONToken states_array = json_read_array(json_girlfriend, "states");
    int32_t states_array_size = json_read_array_length(states_array);

    if (states_array_size < 1) return girlfriend;

    girlfriend->states = malloc_for_array(GameplayManifestCharacterState, states_array_size);
    girlfriend->states_size = states_array_size;

    for (int32_t i = 0; i < states_array_size; i++) {
        JSONToken state_json = json_read_array_item_object(states_array, i);
        girlfriend->states[i] = (GameplayManifestCharacterState){
            .name = json_read_string2(state_json, "name", NULL),
            .model = json_read_string2(state_json, "model", NULL)
        };
    }

    return girlfriend;
}


static GameplayManifestRefer gameplaymanifest_has_default_model(const char* charactermanifest_src) {
    if (string_equals(charactermanifest_src, GAMEPLAYMANIFEST_DEFAULT_GIRLFRIEND))
        return GameplayManifestRefer_GIRLFRIEND;
    else if (string_equals(charactermanifest_src, GAMEPLAYMANIFEST_DEFAULT_BOYFRIEND))
        return GameplayManifestRefer_BOYFRIEND;
    else
        return GameplayManifestRefer_NONE;
}
