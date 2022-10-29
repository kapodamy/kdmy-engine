"use strict";

const GAMEPLAYMANIFEST_REFER_NONE = 0;
const GAMEPLAYMANIFEST_REFER_GIRLFRIEND = 1;
const GAMEPLAYMANIFEST_REFER_BOYFRIEND = 2;

const GAMEPLAYMANIFEST_DEFAULT_GIRLFRIEND = "#girlfriend";
const GAMEPLAYMANIFEST_DEFAULT_BOYFRIEND = "#boyfriend";

const GAMEPLAYMANIFEST_DEFAULT = {
    distributions: [
        STRUMS_DEFAULT_DISTRIBUTION
    ],
    distributions_size: 0,
    girlfriend: {
        refer: GAMEPLAYMANIFEST_REFER_GIRLFRIEND,
        manifest: null,// "#girlfriend"
        states: null,
        states_size: 0
    },
    healthbar: {
        has_disable: false,
        disable: false,

        has_use_alt_icons: false,
        use_alt_icons: false,

        has_opponent_color: false,
        opponent_color_rgb8: HEALTHBAR_DEFAULT_COLOR_DAD,

        has_player_color: false,
        player_color_rgb8: HEALTHBAR_DEFAULT_COLOR_DAD,

        states: null,
        states_size: 0,

        has_warnings_model: true,
        warnings_model: HEALTHBAR_WARNING_MODEL
    },
    stage: null,
    script: null,
    players: null,
    players_size: 0,
    ui_layout: null,
    pause_menu: null
};


async function gameplaymanifest_init(src) {
    let json = await json_load_from(src);
    if (!json) return null;

    let manifest = {
        default: GAMEPLAYMANIFEST_DEFAULT,
        tracks: null,
        tracks_size: 0
    };

    if (json_has_property_object(json, "default")) {
        if (json_is_property_null(json, "default")) {
            throw new Error("'default' property can not be null. File: " + src);
        }

        manifest.default = {
            distributions: GAMEPLAYMANIFEST_DEFAULT.distributions,
            distributions_size: GAMEPLAYMANIFEST_DEFAULT.distributions_size,

            girlfriend: GAMEPLAYMANIFEST_DEFAULT.girlfriend,
            healthbar: GAMEPLAYMANIFEST_DEFAULT.healthbar,

            stage: null,
            script: null,

            players: null,
            players_size: 0,
            ui_layout: null,
            pause_menu: null
        };

        let json_default = json_read_object(json, "default");

        if (json_has_property_array(json_default, "distributions"))
            gameplaymanifest_parse_distributions(json_default, manifest.default, "distributions", "distributions_size");
        else
            gameplaymanifest_parse_distributions_minimal(json_default, manifest.default);

        gameplaymanifest_parse_players(json_default, manifest.default, "players", "players_size");

        if (json_has_property_object(json_default, "healthbar")) {
            manifest.default.healthbar = gameplaymanifest_parse_healthbar(json_default);
        }
        if (json_has_property_object(json_default, "girlfriend")) {
            manifest.default.girlfriend = gameplaymanifest_parse_girlfriend(json_default);
        }

        manifest.default.stage = json_read_string(json_default, "stage", null);
        manifest.default.script = json_read_string(json_default, "script", null);
        manifest.default.ui_layout = json_read_string(json_default, "UILayout", null);
        manifest.default.pause_menu = json_read_string(json_default, "pauseMenu", null);
    }

    let tracks_array = json_read_array(json, "tracks");
    let tracks_array_size = json_read_array_length(tracks_array);

    if (tracks_array_size < 0) {
        json_destroy(json);
        return manifest;
    }

    manifest.tracks_size = tracks_array_size;
    manifest.tracks = new Array(tracks_array_size);

    let players_count = manifest.default ? manifest.default.players_size : -1;

    for (let i = 0; i < tracks_array_size; i++) {
        let json_tracks = json_read_array_item_object(tracks_array, i);

        manifest.tracks[i] = {};
        gameplaymanifest_parse_track(manifest.tracks[i], json_tracks, players_count);

        // check if the girlfriend manifest in the current track is null
        let track_girlfriend = manifest.tracks[i].girlfriend;
        if (manifest.default && manifest.default.girlfriend) {
            if (track_girlfriend && track_girlfriend.manifest == null) {
                track_girlfriend.manifest = strdup(manifest.default.girlfriend.manifest);
            }
        }
    }

    json_destroy(json);
    return manifest;
}

function gameplaymanifest_destroy(gameplaymanifest) {
    if (gameplaymanifest.default && gameplaymanifest.default != GAMEPLAYMANIFEST_DEFAULT) {
        const manifest = gameplaymanifest.default;

        gameplaymanifest_destroy_distributions(manifest.distributions, manifest.distributions_size);
        gameplaymanifest_destroy_girlfriend(manifest.girlfriend);
        gameplaymanifest_destroy_healthbar(manifest.healthbar);
        gameplaymanifest_destroy_players(manifest.players, manifest.players_size);

        manifest.stage = undefined;
        manifest.script = undefined;
        manifest.ui_layout = undefined;
        manifest.pause_menu = undefined;

        gameplaymanifest.default = undefined;
    }

    for (let i = 0; i < gameplaymanifest.tracks_size; i++) {
        const track = gameplaymanifest.tracks[i];

        track.name = undefined;
        track.song = undefined;
        track.chart = undefined;
        track.stage = undefined;
        track.script = undefined;
        track.ui_layout = undefined;
        track.pause_menu = undefined;
        track.selected_state_name = undefined;

        for (let j = 0; j < track.selected_state_name_per_player_size; j++) {
            track.selected_state_name_per_player[i] = undefined;
        }
        track.selected_state_name_per_player = undefined;

        gameplaymanifest_destroy_healthbar(track.healthbar);
        gameplaymanifest_destroy_girlfriend(track.girlfriend);
        gameplaymanifest_destroy_distributions(track.distributions, track.distributions_size);
        gameplaymanifest_destroy_players(track.players, track.players_size);
    }
    gameplaymanifest.tracks = undefined;

    gameplaymanifest = undefined;
}


function gameplaymanifest_destroy_distributions(distributions, distributions_size) {
    if (!distributions) return;
    if (distributions == GAMEPLAYMANIFEST_DEFAULT.distributions) return;

    for (let i = 0; i < distributions_size; i++) {
        if (distributions[i] == STRUMS_DEFAULT_DISTRIBUTION) continue;

        if (distributions[i].notes != STRUMS_DEFAULT_DISTRIBUTION.notes) {
            for (let j = 0; j < distributions[i].notes_size; j++) {
                distributions[i].notes[j].name = undefined;
                distributions[i].notes[j].custom_sick_effect_model_src = undefined;
            }
            distributions[i].notes = undefined;
        }

        if (distributions[i].strums != STRUMS_DEFAULT_DISTRIBUTION.strums) {
            for (let j = 0; j < distributions[i].strums_size; j++) {
                distributions[i].strums[j].name = undefined;
                distributions[i].strums[j].notes_ids = undefined;
            }
            distributions[i].strums = undefined;
        }

        if (distributions[i].strum_binds_is_custom) {
            distributions[i].strum_binds = undefined;
        }

        if (distributions[i].states != STRUMS_DEFAULT_DISTRIBUTION.states) {
            for (let j = 0; j < distributions[i].states_size; j++) {
                distributions[i].states[j].name = undefined;
                distributions[i].states[j].model_marker = undefined;
                distributions[i].states[j].model_sick_effect = undefined;
                distributions[i].states[j].model_background = undefined;
                distributions[i].states[j].model_notes = undefined;
            }
            distributions[i].states = undefined;
        }
    }

    distributions = undefined;
}

function gameplaymanifest_destroy_healthbar(healthbar) {
    if (!healthbar) return;
    if (healthbar == GAMEPLAYMANIFEST_DEFAULT.healthbar) return;

    for (let i = 0; i < healthbar.states_size; i++) {
        healthbar.states[i].name = undefined;

        healthbar.states[i].opponent.icon_model = undefined;
        healthbar.states[i].opponent.bar_model = undefined;

        healthbar.states[i].player.icon_model = undefined;
        healthbar.states[i].player.bar_model = undefined;

        healthbar.states[i].background.icon_model = undefined;
        healthbar.states[i].background.bar_model = undefined;
    }

    healthbar.states = undefined;
    healthbar.warnings_model = undefined;
    healthbar = undefined;
}

function gameplaymanifest_destroy_girlfriend(girlfriend) {
    if (!girlfriend) return;
    if (girlfriend == GAMEPLAYMANIFEST_DEFAULT.girlfriend) return;

    for (let i = 0; i < girlfriend.states_size; i++) {
        girlfriend.states[i].name = undefined;
        girlfriend.states[i].model = undefined;
    }

    girlfriend.states = undefined;
    girlfriend.manifest = undefined;
    girlfriend = undefined;
}

function gameplaymanifest_destroy_players(players, players_size) {
    if (!players) return;

    for (let i = 0; i < players_size; i++) {
        players[i].manifest = undefined;
        for (let j = 0; j < players[i].states_size; j++) {
            players[i].states.name = undefined;
            players[i].states.model = undefined;
        }
        players[i].states = undefined;
    }

    players = undefined;
}


function gameplaymanifest_parse_track(track, json_track, players_count) {
    track.name = json_read_string(json_track, "name", null);
    track.song = json_read_string(json_track, "song", null);
    track.chart = json_read_string(json_track, "chart", null);

    if (track.name == null && !track.song) {
        throw new Error("gameplaymanifest_parse_track() track without 'name' and 'song'");
    } else if (track.name != null && track.song == null && track.chart == null) {
        if (fs_is_invalid_filename(track.name)) {
            throw new Error("The track '" + track.name + "' is invalid for filename (song=null chart=null)");
        }

        // build the song name and track name using lowercase and without spaces
        // example: "Dad Battle" --> "dadbattle"
        let track_name_length = track.name.length;
        let stringbuilder = stringbuilder_init(track_name_length);
        stringbuilder_add_with_replace(stringbuilder, track.name, "\x20", null);
        stringbuilder_lowercase(stringbuilder);
        let lowercase_name = stringbuilder_get_copy(stringbuilder);
        stringbuilder_destroy(stringbuilder);

        track.song = string_concat(3, FUNKIN_WEEK_SONGS_FOLDER, lowercase_name, ".ogg");
        track.chart = string_concat(3, FUNKIN_WEEK_CHARTS_FOLDER, lowercase_name, ".json");
        lowercase_name = undefined;
    }

    track.stage = json_read_string(json_track, "stage", null);
    track.has_stage = json_has_property(json_track, "stage");

    track.script = json_read_string(json_track, "script", null);
    track.has_script = json_has_property(json_track, "script");

    track.duration = json_read_number(json_track, "duration", -1);

    track.selected_state_name = json_read_string(json_track, "selectedStateName", null);
    track.has_selected_state_name = json_has_property(json_track, "selectedStateName");

    track.ui_layout = json_read_string(json_track, "UILayout", null);
    track.has_ui_layout = json_has_property(json_track, "UILayout");

    track.pause_menu = json_read_string(json_track, "pauseMenu", null);
    track.has_pause_menu = json_has_property(json_track, "pauseMenu");

    track.selected_state_name_per_player = null;
    track.selected_state_name_per_player_size = 0;

    track.players = null;
    track.players_size = 0;

    track.distributions = null;
    track.distributions_size = 0;

    track.has_players = json_has_property_array(json_track, "players");
    gameplaymanifest_parse_players(json_track, track, "players", "players_size");

    track.has_distributions = json_has_property_array(json_track, "distributions");
    gameplaymanifest_parse_distributions(json_track, track, "distributions", "distributions_size");

    track.healthbar = gameplaymanifest_parse_healthbar(json_track);
    track.girlfriend = gameplaymanifest_parse_girlfriend(json_track);
    track.has_girlfriend = json_has_property_object(json_track, "girlfriend");

    let selected_states_array = json_read_array(json_track, "selectedStateNamePerPlayer");
    let selected_states_array_size = json_read_array_length(selected_states_array);

    if (track.has_players) players_count = track.players_size;

    if (players_count < 0 && selected_states_array_size > 0) {
        console.error(
            "gameplaymanifest_parse_track() 'selectedStateNamePerPlayer' found without 'players'"
        );
    } else if (selected_states_array_size > 0) {

        if (players_count != selected_states_array_size) {
            console.warn(
                "gameplaymanifest_parse_track() 'selectedStateNamePerPlayer.length' != 'players.length'"
            );
        } else {
            track.selected_state_name_per_player = new Array(selected_states_array_size);
            track.selected_state_name_per_player_size = selected_states_array_size;
        }
    }

    for (let i = 0; i < selected_states_array_size; i++) {
        track.selected_state_name_per_player[i] = json_read_array_item_string(
            selected_states_array, i, null
        );
    }
}

function gameplaymanifest_parse_notes(json, distribution) {
    let notes_array = json_read_array(json, "notes");
    let notes_array_size = json_read_array_length(notes_array);

    if (notes_array_size < 0) {
        distribution.notes = null;
        distribution.notes_size = 0;
        return;
    }

    let notes = new Array(notes_array_size);
    let notes_size = notes_array_size;

    for (let i = 0; i < notes_array_size; i++) {
        notes[i] = {
            name: null,
            custom_sick_effect_model_src: null,
            damage_ratio: 1.0,
            heal_ratio: 1.0,
            ignore_hit: false,
            ignore_miss: false,
            can_kill_on_hit: false,
            is_special: false,
            model_src: null,
            sick_effect_state_name: null
        };

        let note_json = json_read_array_item_object(notes_array, i);
        if (!note_json) {
            console.error("gameplaymanifest_parse_notes() invalid note definition");
            continue;
        }

        notes[i].name = json_read_string(note_json, "name", null);
        notes[i].custom_sick_effect_model_src = json_read_string(note_json, "customSickEffectModel", null);
        notes[i].damage_ratio = json_read_number(note_json, "damageRatio", 1.0);
        notes[i].heal_ratio = json_read_number(note_json, "healRatio", 1.0);
        notes[i].ignore_hit = json_read_boolean(note_json, "ignoreHit", false);
        notes[i].ignore_miss = json_read_boolean(note_json, "ignoreMiss", false);
        notes[i].can_kill_on_hit = json_read_boolean(note_json, "canKillOnHit", false);
        notes[i].is_special = json_read_boolean(note_json, "isSpecial", false);
        notes[i].model_src = json_read_string(note_json, "model", null);
        notes[i].sick_effect_state_name = json_read_string(note_json, "sickEffectStateName", null);
    }

    distribution.notes = notes;
    distribution.notes_size = notes_size;
}

function gameplaymanifest_parse_distributions(json, obj, ptr_distributions, ptr_distributions_size) {
    let json_distributions = json_read_array(json, "distributions");
    let json_distributions_size = json_read_array_length(json_distributions);

    if (json_distributions_size < 1) {
        obj[ptr_distributions] = null;
        obj[ptr_distributions_size] = 0;
        return;
    }

    let distributions = new Array(json_distributions_size);
    let distributions_size = json_distributions_size;

    for (let i = 0; i < json_distributions_size; i++) {
        distributions[i] = {};
        gameplaymanifest_parse_distribution(
            json_read_array_item_object(json_distributions, i), distributions[i]
        );
    }
    
    obj[ptr_distributions] = distributions;
    obj[ptr_distributions_size] = distributions_size;
}

function gameplaymanifest_parse_distribution(json_distribution, distribution) {
    if (!json_distribution) throw new Error("Invalid distribution found");

    let strums_array = json_read_array(json_distribution, "strums");
    let strums_array_size = json_read_array_length(strums_array);
    let strum_binds_array = json_read_array(json_distribution, "buttonBinds");
    let strum_binds_array_size = json_read_array_length(strums_array);


    distribution.notes = null;
    distribution.notes_size = 0;

    distribution.strums = null;
    distribution.strum_binds = null;
    distribution.strums_size = 0;
    distribution.strum_binds_is_custom = 0;

    distribution.states = null;
    distribution.states_size = 0


    if (strum_binds_array_size < 0) {
        switch (strums_array_size) {
            case 4:
                distribution.strum_binds = BUTTONS_BIND_4;
                break;
            case 5:
                distribution.strum_binds = BUTTONS_BIND_5;
                break;
            case 6:
                distribution.strum_binds = BUTTONS_BIND_6;
                break;
            case 7:
                distribution.strum_binds = BUTTONS_BIND_7;
                break;
            case 8:
                distribution.strum_binds = BUTTONS_BIND_8;
                break;
            case 9:
                distribution.strum_binds = BUTTONS_BIND_9;
                break;
            default:
                throw new Error("Can not find the button binds for strums size=" + strums_array_size);
        }
    } else {
        if (strum_binds_array_size != strums_array_size) {
            throw new Error("'buttonBinds.length' != 'strums.length'");
        }
        if (strum_binds_array_size > 0) {
            distribution.strum_binds = new Array(strums_array_size);
            distribution.strum_binds_is_custom = 1;
            for (let i = 0; i < strums_array_size; i++) {
                distribution.strum_binds[i] = json_read_array_item_hex(strum_binds_array, i, 0x00);
            }
        }
    }

    gameplaymanifest_parse_notes(json_distribution, distribution);

    if (strums_array_size > 0) {
        distribution.strums = new Array(strums_array_size);
        distribution.strums_size = strums_array_size;
    }

    for (let i = 0; i < strums_array_size; i++) {
        let strum_json = json_read_array_item_object(strums_array, i);

        distribution.strums[i] = {
            name: json_read_string(strum_json, "name", null),
            notes_ids: null,
            notes_ids_size: 0
        };

        let notes_ids_array = json_read_array(strum_json, "noteIds");
        let notes_ids_array_size = json_read_array_length(notes_ids_array);

        if (notes_ids_array_size > 0) {
            distribution.strums[i].notes_ids_size = notes_ids_array_size;
            distribution.strums[i].notes_ids = new Array(notes_ids_array_size);
        }

        for (let j = 0; j < notes_ids_array_size; j++) {
            let index = json_read_array_item_number(notes_ids_array, j, -1);

            if (index < 0) {
                throw new Error("Invalid note index found in strum index=" + i);
            } else if (index >= distribution.notes_size) {
                console.error("gameplaymanifest_parse_strums() invalid 'noteId' index=" + index);
                index = 0;
            }

            distribution.strums[i].notes_ids[j] = index;
        }
    }

    let states_array = json_read_array(json_distribution, "states");
    let states_array_size = json_read_array_length(states_array);

    if (states_array_size > 0) {
        distribution.states = new Array(states_array_size);
        distribution.states_size = states_array_size;
    }

    for (let i = 0; i < states_array_size; i++) {
        let state_json = json_read_array_item_object(states_array, i);
        distribution.states[i] = {
            name: json_read_string(state_json, "name", null),
            model_marker: json_read_string(state_json, "modelMarker", null),
            model_sick_effect: json_read_string(state_json, "modelSickEffect", null),
            model_background: json_read_string(state_json, "modelBackground", null),
            model_notes: json_read_string(state_json, "modelNotes", null)
        };
    }

}

function gameplaymanifest_parse_distributions_minimal(json, manifest) {
    let json_dists_minimal_array = json_read_array(json, "distributionsMinimal");
    let json_dists_minimal_array_length = json_read_array_length(json_dists_minimal_array);

    if (json_dists_minimal_array_length < 0) return;

    if (json_dists_minimal_array_length < 1) {
        manifest.distributions = null;
        manifest.distributions_size = 0;
        return;
    }

    let dists_minimal = new Array(json_dists_minimal_array_length);

    for (let i = 0; i < json_dists_minimal_array_length; i++) {
        let json_dist = json_read_array_item_object(json_dists_minimal_array, i);

        dists_minimal[i] = {
            notes_size: STRUMS_DEFAULT_DISTRIBUTION.notes_size,
            notes: STRUMS_DEFAULT_DISTRIBUTION.notes,

            strums_size: STRUMS_DEFAULT_DISTRIBUTION.strums_size,
            strums: STRUMS_DEFAULT_DISTRIBUTION.strums,
            strum_binds: STRUMS_DEFAULT_DISTRIBUTION.strum_binds,
            states: null,
            states_size: 1
        };

        let dist_states_array = json_read_array(json_dist, "states");
        let dist_states_array_size = json_read_array_length(dist_states_array);

        if (dist_states_array_size > 0) dists_minimal[i].states_size += dist_states_array_size;
        dists_minimal[i].states = new Array(dists_minimal[i].states_size);

        // build default state
        let default_state = dists_minimal[i].states[0] = {
            name: strdup(STRUMS_DEFAULT_DISTRIBUTION.states[0].name),
            model_marker: strdup(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_marker),
            model_sick_effect: strdup(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_sick_effect),
            model_background: strdup(STRUMS_DEFAULT_DISTRIBUTION.states[0].model_background)
        };

        // override values in the default state
        if (json_has_property_string(json_dist, "modelMarker")) {
            default_state.model_marker = json_read_string(json_dist, "modelMarker", null);
        }
        if (json_has_property_string(json_dist, "modelSickEffect")) {
            default_state.model_sick_effect = json_read_string(json_dist, "modelSickEffect", null);
        }
        if (json_has_property_string(json_dist, "modelBackground")) {
            default_state.model_background = json_read_string(json_dist, "modelBackground", null);
        }

        // read all remaining states
        for (let j = 0; j < dist_states_array_size; j++) {
            let state_json = json_read_array_item_object(dist_states_array, i);
            dists_minimal[i].states[j + 1] = {
                name: json_read_string(state_json, "name", null),
                model_marker: json_read_string(state_json, "modelMarker", null),
                model_sick_effect: json_read_string(state_json, "modelSickEffect", null),
                model_background: json_read_string(state_json, "modelBackground", null),
                model_notes: json_read_string(state_json, "modelNotes", null)
            };
        }
    }

    manifest.distributions = dists_minimal;
    manifest.distributions_size = json_dists_minimal_array_length;
}


function gameplaymanifest_parse_healthbar(json) {
    let json_healthbar = json_read_object(json, "healthbar");
    if (!json_healthbar) return null;

    let healthbar = {
        disable: false,
        has_disable: false,

        opponent_color_rgb8: 0x00,
        has_opponent_color: false,

        player_color_rgb8: 0x00,
        has_player_color: false,

        warnings_model: null,
        has_warnings_model: false,

        use_alt_icons: false,
        has_use_alt_icons: false,

        states: null,
        states_size: 0
    };

    if (json_has_property_boolean(json_healthbar, "disable")) {
        healthbar.disable = json_read_boolean(json_healthbar, "disable", healthbar.disable);
        healthbar.has_disable = true;
    }

    if (json_has_property_hex(json_healthbar, "opponentColor")) {
        healthbar.opponent_color_rgb8 = json_read_hex(json_healthbar, "opponentColor", HEALTHBAR_DEFAULT_COLOR_DAD);
        healthbar.has_opponent_color = true;
    }

    if (json_has_property_hex(json_healthbar, "playerColor")) {
        healthbar.player_color_rgb8 = json_read_hex(json_healthbar, "playerColor", HEALTHBAR_DEFAULT_COLOR_BOYFRIEND);
        healthbar.has_player_color = true;
    }

    if (json_has_property(json_healthbar, "warningsModel")) {
        healthbar.warnings_model = json_read_string(json_healthbar, "warningsModel", null);
        healthbar.has_warnings_model = true;
    }

    if (json_has_property(json_healthbar, "useAltWarnIcons")) {
        healthbar.use_alt_icons = json_read_boolean(json_healthbar, "useAltWarnIcons", false);
        healthbar.has_use_alt_icons = true;
    }


    let states_array = json_read_array(json_healthbar, "states");
    let states_array_size = json_read_array_length(states_array);

    if (states_array_size < 1) return healthbar;

    healthbar.states = new Array(states_array_size);
    healthbar.states_size = states_array_size;

    for (let i = 0; i < states_array_size; i++) {
        let state = healthbar.states[i] = {
            name: null,
            opponent: {},
            player: {},
            background: {}
        };

        let state_json = json_read_array_item_object(states_array, i);
        if (!state_json) {
            console.warn("gameplaymanifest_parse_healthbar_states() invalid healthbar state at " + i);
            continue;
        }

        state.name = json_read_string(state_json, "name", null);
        gameplaymanifest_parse_healthbar_states(state.opponent, state_json, "opponent");
        gameplaymanifest_parse_healthbar_states(state.player, state_json, "player");
        gameplaymanifest_parse_healthbar_states(state.background, state_json, "background");
    }

    return healthbar;
}

function gameplaymanifest_parse_healthbar_states(healthbar_state, json_state, property) {
    let json = json_read_object(json_state, property);
    healthbar_state.icon_model = json_read_string(json, "iconModel", null);
    healthbar_state.bar_model = json_read_string(json, "barModel", null);
    healthbar_state.bar_color = json_read_hex(json, "barColor", 0x000000);
}


function gameplaymanifest_parse_players(json, obj, ptr_players, ptr_players_size) {
    let players_array = json_read_array(json, "players");
    let players_array_size = json_read_array_length(players_array);

    if (players_array_size < 1) {
        obj[ptr_players] = null;
        obj[ptr_players_size] = 0;
        return;
    }

    let players = new Array(players_array_size);
    let players_size = players_array_size;

    for (let i = 0; i < players_array_size; i++) {
        let player_json = json_read_array_item_object(players_array, i);

        players[i] = {
            manifest: json_read_string(player_json, "manifest", null),
            refer: GAMEPLAYMANIFEST_REFER_NONE,
            controller: json_read_number(player_json, "controller", -1),
            layout_strums_id: json_read_number(player_json, "strumsInLayoutId", -1),
            distribution_index: json_read_number(player_json, "distributionIndex", 0),
            states: null,
            states_size: 0,
            can_die: 0,
            can_recover: 0,
            is_opponent: 0,
        };

        if (json_has_property_boolean(json, "isOpponent")) {
            players[i].is_opponent = json_read_boolean(json, "isOpponent", false);
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
        }
        if (json_has_property_boolean(player_json, "canRecover")) {
            players[i].can_recover = json_read_boolean(player_json, "canRecover", false);
        }

        players[i].refer = gameplaymanifest_has_default_model(players[i].manifest);
        if (players[i].refer != GAMEPLAYMANIFEST_REFER_NONE) {
            // a default character manifest was specified, ignore
            players[i].manifest = undefined;
            players[i].manifest = null;
        }

        let states_array = json_read_array(player_json, "states");
        let states_array_size = json_read_array_length(states_array);

        if (states_array_size < 1) continue;

        players[i].states = new Array(states_array_size);
        players[i].states_size = states_array_size;

        for (let j = 0; j < states_array_size; j++) {
            let state_json = json_read_array_item_object(states_array, j);
            players[i].states[j] = {
                name: json_read_string(state_json, "name", null),
                model: json_read_string(state_json, "model", null)
            };
        }
    }

    obj[ptr_players] = players;
    obj[ptr_players_size] = players_size;
}

function gameplaymanifest_parse_girlfriend(json) {
    let json_girlfriend = json_read_object(json, "girlfriend");
    if (!json_girlfriend) return null;

    let girlfriend = {
        manifest: null,
        refer: GAMEPLAYMANIFEST_REFER_NONE,
        states: null,
        states_size: 0
    };

    girlfriend.manifest = json_read_string(json_girlfriend, "manifest", null);
    girlfriend.refer = gameplaymanifest_has_default_model(girlfriend.manifest);

    if (girlfriend.refer != GAMEPLAYMANIFEST_REFER_NONE) {
        // a default character manifest was specified, ignore
        girlfriend.manifest = undefined;
        girlfriend.manifest = null;
    }

    let states_array = json_read_array(json_girlfriend, "states");
    let states_array_size = json_read_array_length(states_array);

    if (states_array_size < 1) return girlfriend;

    girlfriend.states = new Array(states_array_size);
    girlfriend.states_size = states_array_size;

    for (let i = 0; i < states_array_size; i++) {
        let state_json = json_read_array_item_object(states_array, i);
        girlfriend.states[i] = {
            name: json_read_string(state_json, "name", null),
            model: json_read_string(state_json, "model", null)
        };
    }

    return girlfriend;
}


function gameplaymanifest_has_default_model(charactermanifest_src) {
    if (charactermanifest_src == GAMEPLAYMANIFEST_DEFAULT_GIRLFRIEND)
        return GAMEPLAYMANIFEST_REFER_GIRLFRIEND;
    else if (charactermanifest_src == GAMEPLAYMANIFEST_DEFAULT_BOYFRIEND)
        return GAMEPLAYMANIFEST_REFER_BOYFRIEND;
    else
        return GAMEPLAYMANIFEST_REFER_NONE;
}

