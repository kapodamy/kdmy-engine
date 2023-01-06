using System;
using System.Text;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Gameplay.Helpers {

    public enum GameplayManifestRefer {
        NONE,
        GIRLFRIEND,
        BOYFRIEND
    }
    public struct GameplayManifestCharacterState {
        public string name;
        public string model;
    }
    public class GameplayManifestPlayer {
        public string manifest;
        public GameplayManifestCharacterState[] states;
        public int states_size;
        public GameplayManifestRefer refer;
        public int controller;
        public int layout_strums_id;
        public int distribution_index;
        public bool can_die;
        public bool can_recover;
        public bool is_opponent;
    }
    public class GameplayManifestGirlfriend {
        public GameplayManifestRefer refer;
        public string manifest;
        public GameplayManifestCharacterState[] states;
        public int states_size;
    }
    public struct GameplayManifestHealthBarStateEntry {
        public string icon_model;
        public string bar_model;
        public uint bar_color;
    }
    public struct GameplayManifestHealthBarState {
        public string name;
        public GameplayManifestHealthBarStateEntry opponent;
        public GameplayManifestHealthBarStateEntry player;
        public GameplayManifestHealthBarStateEntry background;
    }
    public class GameplayManifestHealthBar {
        public bool has_disable;
        public bool disable;
        public bool has_use_alt_icons;
        public bool use_alt_icons;
        public bool has_opponent_color;
        public uint opponent_color_rgb8;
        public bool has_player_color;
        public uint player_color_rgb8;
        public GameplayManifestHealthBarState[] states;
        public int states_size;
        public bool has_warnings_model;
        public string warnings_model;
    }
    public class GameplayManifestTrack {
        public string name;
        public string song;
        public string chart;
        public string stage;
        public bool has_stage;
        public string script;
        public bool has_script;
        public double duration;
        public string selected_state_name;
        public bool has_selected_state_name;
        public string ui_layout;
        public bool has_ui_layout;
        public string pause_menu;
        public bool has_pause_menu;
        public string[] selected_state_name_per_player;
        public int selected_state_name_per_player_size;
        public GameplayManifestPlayer[] players;
        public int players_size;
        public Distribution[] distributions;
        public int distributions_size;
        public bool has_players;
        public bool has_distributions;
        public GameplayManifestHealthBar healthbar;
        public GameplayManifestGirlfriend girlfriend;
        public bool has_girlfriend;
        public string dialogue_params;
        public string dialog_text;
        public bool dialog_ignore_on_freeplay;
    }
    public class GameplayManifestDefault {
        public Distribution[] distributions;
        public int distributions_size;
        public GameplayManifestGirlfriend girlfriend;
        public GameplayManifestHealthBar healthbar;
        public string stage;
        public string script;
        public GameplayManifestPlayer[] players;
        public int players_size;
        public string ui_layout;
        public string pause_menu;
        public string dialogue_params;
    }


    public class GameplayManifest {

        public const string DEFAULT_GIRLFRIEND = "#girlfriend";
        public const string DEFAULT_BOYFRIEND = "#boyfriend";

        public static readonly GameplayManifestDefault DEFAULT = new GameplayManifestDefault() {
            distributions = new Distribution[]{
                Strums.DEFAULT_DISTRIBUTION
            },
            distributions_size = 0,
            girlfriend = new GameplayManifestGirlfriend() {
                refer = GameplayManifestRefer.GIRLFRIEND,
                manifest = null,// "#girlfriend"
                states = null,
                states_size = 0
            },
            healthbar = new GameplayManifestHealthBar() {
                has_disable = false,
                disable = false,

                has_use_alt_icons = false,
                use_alt_icons = false,

                has_opponent_color = false,
                opponent_color_rgb8 = HealthBar.DEFAULT_COLOR_DAD,

                has_player_color = false,
                player_color_rgb8 = HealthBar.DEFAULT_COLOR_BOYFRIEND,

                states = null,
                states_size = 0,

                has_warnings_model = true,
                warnings_model = HealthBar.WARNING_MODEL
            },
            stage = null,
            script = null,
            players = null,
            players_size = 0,
            ui_layout = null,
            pause_menu = null,
            dialogue_params = null
        };

        public GameplayManifestDefault @default;
        public GameplayManifestTrack[] tracks;
        public int tracks_size;


        private GameplayManifest() { }


        public static GameplayManifest Init(string src) {
            JSONParser json = JSONParser.LoadFrom(src);
            if (json == null) return null;

            GameplayManifest manifest = new GameplayManifest() {
                @default = GameplayManifest.DEFAULT,
                tracks = null,
                tracks_size = 0
            };

            if (JSONParser.HasPropertyObject(json, "default")) {
                if (JSONParser.IsPropertyNull(json, "default")) {
                    throw new Exception("'default' property can not be null. File: " + src);
                }

                manifest.@default = new GameplayManifestDefault() {
                    distributions = GameplayManifest.DEFAULT.distributions,
                    distributions_size = GameplayManifest.DEFAULT.distributions_size,

                    girlfriend = GameplayManifest.DEFAULT.girlfriend,
                    healthbar = GameplayManifest.DEFAULT.healthbar,

                    stage = null,
                    script = null,

                    players = null,
                    players_size = 0,
                    ui_layout = null,
                    pause_menu = null,
                    dialogue_params = null
                };

                JSONToken json_default = JSONParser.ReadObject(json, "default");

                if (JSONParser.HasPropertyArray(json_default, "distributions"))
                    GameplayManifest.ParseDistributions(json_default, out manifest.@default.distributions, out manifest.@default.distributions_size);
                else
                    GameplayManifest.ParseDistributionsMinimal(json_default, manifest.@default);

                GameplayManifest.ParsePlayers(json_default, out manifest.@default.players, out manifest.@default.players_size);

                if (JSONParser.HasPropertyObject(json_default, "healthbar")) {
                    manifest.@default.healthbar = GameplayManifest.ParseHealthbar(json_default);
                }
                if (JSONParser.HasPropertyObject(json_default, "girlfriend")) {
                    manifest.@default.girlfriend = GameplayManifest.ParseGirlfriend(json_default);
                }

                manifest.@default.stage = JSONParser.ReadString(json_default, "stage", null);
                manifest.@default.script = JSONParser.ReadString(json_default, "script", null);
                manifest.@default.ui_layout = JSONParser.ReadString(json_default, "UILayout", null);
                manifest.@default.pause_menu = JSONParser.ReadString(json_default, "pauseMenu", null);
                manifest.@default.dialogue_params = JSONParser.ReadString(json_default, "dialogueParams", null);
            }

            JSONToken tracks_array = JSONParser.ReadArray(json, "tracks");
            int tracks_array_size = JSONParser.ReadArrayLength(tracks_array);

            if (tracks_array_size < 0) {
                JSONParser.Destroy(json);
                return manifest;
            }

            manifest.tracks_size = tracks_array_size;
            manifest.tracks = new GameplayManifestTrack[tracks_array_size];

            int players_count = manifest.@default != null ? manifest.@default.players_size : -1;

            for (int i = 0 ; i < tracks_array_size ; i++) {
                JSONToken json_tracks = JSONParser.ReadArrayItemObject(tracks_array, i);

                manifest.tracks[i] = new GameplayManifestTrack() { };
                GameplayManifest.ParseTrack(manifest.tracks[i], json_tracks, players_count);

                // check if the girlfriend manifest in the current track is null
                GameplayManifestGirlfriend track_girlfriend = manifest.tracks[i].girlfriend;
                if (manifest.@default != null && manifest.@default.girlfriend != null) {
                    if (track_girlfriend != null && track_girlfriend.manifest == null) {
                        track_girlfriend.manifest = manifest.@default.girlfriend.manifest;
                    }
                }
            }

            JSONParser.Destroy(json);
            return manifest;
        }

        public void Destroy() {
            if (this.@default != null && this.@default != GameplayManifest.DEFAULT) {
                GameplayManifestDefault manifest = this.@default;

                GameplayManifest.DestroyDistributions(manifest.distributions, manifest.distributions_size);
                GameplayManifest.DestroyGirlfriend(manifest.girlfriend);
                GameplayManifest.DestroyHealthbar(manifest.healthbar);
                GameplayManifest.DestroyPlayers(manifest.players, manifest.players_size);

                //free(manifest.stage);
                //free(manifest.script);
                //free(manifest.ui_layout);
                //free(manifest.pause_menu);

                //free(gameplaymanifest.@default);
                //free(manifest.dialogue_params);
            }

            for (int i = 0 ; i < this.tracks_size ; i++) {
                GameplayManifestTrack track = this.tracks[i];

                //free(track.name);
                //free(track.song);
                //free(track.chart);
                //free(track.stage);
                //free(track.script);
                //free(track.ui_layout);
                //free(track.pause_menu);
                //free(track.selected_state_name);

                //for (int j = 0 ; j < track.selected_state_name_per_player_size ; j++) {
                //    free(track.selected_state_name_per_player[i]);
                //}
                //free(track.selected_state_name_per_player);
                //free(track.dialogue_params);
                //free(track.dialog_text);

                GameplayManifest.DestroyHealthbar(track.healthbar);
                GameplayManifest.DestroyGirlfriend(track.girlfriend);
                GameplayManifest.DestroyDistributions(track.distributions, track.distributions_size);
                GameplayManifest.DestroyPlayers(track.players, track.players_size);
            }
            //free(this.tracks);

            //free(this);
        }


        public static void DestroyDistributions(Distribution[] distributions, int distributions_size) {
            if (distributions == null) return;
            if (distributions == GameplayManifest.DEFAULT.distributions) return;

            for (int i = 0 ; i < distributions_size ; i++) {
                if (distributions[i] == Strums.DEFAULT_DISTRIBUTION) continue;

                //if (distributions[i].notes != Strums.DEFAULT_DISTRIBUTION.notes) {
                //    for (int j = 0 ; i < distributions[i].notes_size ; i++) {
                //        free(distributions[i].notes[j].name);
                //        free(distributions[i].notes[j].custom_sick_effect_model_src);
                //    }
                //    free(distributions[i].notes);
                //}

                //if (distributions[i].strums != STRUMS_DEFAULT_DISTRIBUTION.strums) {
                //    for (int j = 0 ; i < distributions[i].strums_size ; j++) {
                //        free(distributions[i].strums[j].name);
                //        free(distributions[i].strums[j].notes_ids);
                //    }
                //    free(distributions[i].strums);
                //}

                //if (distributions[i].strum_binds != Strums.DEFAULT_DISTRIBUTION.strum_binds) {
                //    free(distributions[i].strum_binds);
                //}

                //if (distributions[i].states != Strums.DEFAULT_DISTRIBUTION.states) {
                //    for (int j = 0 ; j < distributions[i].states_size ; j++) {
                //        free(distributions[i].states[j].name);
                //        free(distributions[i].states[j].model_marker);
                //        free(distributions[i].states[j].model_sick_effect);
                //        free(distributions[i].states[j].model_background);
                //        free(distributions[i].states[j].model_notes);
                //    }
                //    free(distributions[i].states);
                //}
            }

            //free(distributions);
        }

        public static void DestroyHealthbar(GameplayManifestHealthBar healthbar) {
            if (healthbar == null) return;
            if (healthbar == GameplayManifest.DEFAULT.healthbar) return;

            //for (int i = 0 ; i < healthbar.states_size ; i++) {
            //    free(healthbar.states[i].name);
            //
            //    free(healthbar.states[i].opponent.icon_model);
            //    free(healthbar.states[i].opponent.bar_model);
            //
            //    free(healthbar.states[i].player.icon_model);
            //    free(healthbar.states[i].player.bar_model);
            //
            //    free(healthbar.states[i].background.icon_model);
            //    free(healthbar.states[i].background.bar_model);
            //}

            //free(healthbar.states);
            //free(healthbar.warnings_model);
            //free(healthbar);
        }

        public static void DestroyGirlfriend(GameplayManifestGirlfriend girlfriend) {
            if (girlfriend == null) return;
            if (girlfriend == GameplayManifest.DEFAULT.girlfriend) return;

            //for (int i = 0 ; i < girlfriend.states_size ; i++) {
            //    free(girlfriend.states[i].name);
            //    free(girlfriend.states[i].model);
            //}

            //free(girlfriend.states);
            //free(girlfriend.manifest);
            //free(girlfriend);
        }

        public static void DestroyPlayers(GameplayManifestPlayer[] players, int players_size) {
            if (players == null) return;

            //for (int i = 0 ; i < players_size ; i++) {
            //    free(players[i].manifest);
            //    for (int j = 0 ; j < players[i].states_size ; j++) {
            //        free(players[i].states.name);
            //        free(players[i].states.model);
            //    }
            //    free(players[i].states);
            //}

            //free(players);
        }


        public static void ParseTrack(GameplayManifestTrack track, JSONToken json_track, int players_count) {
            track.name = JSONParser.ReadString(json_track, "name", null);
            track.song = JSONParser.ReadString(json_track, "song", null);
            track.chart = JSONParser.ReadString(json_track, "chart", null);

            if (track.name == null && String.IsNullOrEmpty(track.song)) {
                throw new Exception("gameplaymanifest_parse_track() track without 'name' and 'song'");
            } else if (track.name != null && track.song == null && track.chart == null) {
                if (FS.IsInvalidFilename(track.name)) {
                    throw new Exception($"The track '{track.name}' is invalid for filename (song=null chart=null)");
                }

                // build the song name and track name using lowercase and without spaces
                // example: "Dad Battle" --> "dadbattle"
                int track_name_length = track.name.Length;
                StringBuilder stringbuilder = new StringBuilder(track_name_length);
                stringbuilder.AddWithReplaceKDY(track.name, "\x20", null);
                stringbuilder.LowerCaseKDY();
                string lowercase_name = stringbuilder.GetCopyKDY();
                //stringbuilder.Destroy();

                track.song = StringUtils.Concat(Funkin.WEEK_SONGS_FOLDER, lowercase_name, ".ogg");
                track.chart = StringUtils.Concat(Funkin.WEEK_CHARTS_FOLDER, lowercase_name, ".json");
                //free(lowercase_name);
            }

            track.stage = JSONParser.ReadString(json_track, "stage", null);
            track.has_stage = JSONParser.HasProperty(json_track, "stage");

            track.script = JSONParser.ReadString(json_track, "script", null);
            track.has_script = JSONParser.HasProperty(json_track, "script");

            track.duration = JSONParser.ReadNumberDouble(json_track, "duration", -1.0);
            if (track.duration >= 0) track.duration *= 1000.0;// convert to milliseconds

            track.selected_state_name = JSONParser.ReadString(json_track, "selectedStateName", null);
            track.has_selected_state_name = JSONParser.HasProperty(json_track, "selectedStateName");

            track.ui_layout = JSONParser.ReadString(json_track, "UILayout", null);
            track.has_ui_layout = JSONParser.HasProperty(json_track, "UILayout");

            track.pause_menu = JSONParser.ReadString(json_track, "pauseMenu", null);
            track.has_pause_menu = JSONParser.HasProperty(json_track, "pauseMenu");

            track.dialogue_params = JSONParser.ReadString(json_track, "dialogueParams", null);
            track.dialog_text = JSONParser.ReadString(json_track, "dialogText", null);
            track.dialog_ignore_on_freeplay = JSONParser.ReadBoolean(json_track, "dialogIgnoreOnFreeplay", true);

            track.selected_state_name_per_player = null;
            track.selected_state_name_per_player_size = 0;

            track.players = null;
            track.players_size = 0;

            track.distributions = null;
            track.distributions_size = 0;

            track.has_players = JSONParser.HasPropertyArray(json_track, "players");
            GameplayManifest.ParsePlayers(json_track, out track.players, out track.players_size);

            track.has_distributions = JSONParser.HasPropertyArray(json_track, "distributions");
            GameplayManifest.ParseDistributions(json_track, out track.distributions, out track.distributions_size);

            track.healthbar = GameplayManifest.ParseHealthbar(json_track);
            track.girlfriend = GameplayManifest.ParseGirlfriend(json_track);
            track.has_girlfriend = JSONParser.HasPropertyObject(json_track, "girlfriend");

            JSONToken selected_states_array = JSONParser.ReadArray(json_track, "selectedStateNamePerPlayer");
            int selected_states_array_size = JSONParser.ReadArrayLength(selected_states_array);

            if (track.has_players) players_count = track.players_size;

            if (players_count < 0 && selected_states_array_size > 0) {
                Console.Error.WriteLine("[ERROR]" +
                    "gameplaymanifest_parse_track() 'selectedStateNamePerPlayer' found without 'players'"
                );
            } else if (selected_states_array_size > 0) {

                if (players_count != selected_states_array_size) {
                    Console.Error.WriteLine("[WARN]" +
                        "gameplaymanifest_parse_track() 'selectedStateNamePerPlayer.length' != 'players.length'"
                    );
                } else {
                    track.selected_state_name_per_player = new string[selected_states_array_size];
                    track.selected_state_name_per_player_size = selected_states_array_size;
                }
            }

            for (int i = 0 ; i < selected_states_array_size ; i++) {
                track.selected_state_name_per_player[i] = JSONParser.ReadArrayItemString(
                    selected_states_array, i, null
                );
            }
        }

        public static void ParseNotes(JSONToken json, Distribution distribution) {
            JSONToken notes_array = JSONParser.ReadArray(json, "notes");
            int notes_array_size = JSONParser.ReadArrayLength(notes_array);

            if (notes_array_size < 0) {
                distribution.notes = null;
                distribution.notes_size = 0;
                return;
            }

            DistributionNote[] notes = new DistributionNote[notes_array_size];
            int notes_size = notes_array_size;

            for (int i = 0 ; i < notes_array_size ; i++) {
                notes[i] = new DistributionNote() {
                    name = null,
                    custom_sick_effect_model_src = null,
                    damage_ratio = 1.0f,
                    heal_ratio = 1.0f,
                    ignore_hit = false,
                    ignore_miss = false,
                    can_kill_on_hit = false,
                    is_special = false,
                    model_src = null,
                    sick_effect_state_name = null
                };

                JSONToken note_json = JSONParser.ReadArrayItemObject(notes_array, i);
                if (note_json == null) {
                    Console.Error.WriteLine("[ERROR] gameplaymanifest_parse_notes() invalid note definition");
                    continue;
                }

                notes[i].name = JSONParser.ReadString(note_json, "name", null);
                notes[i].custom_sick_effect_model_src = JSONParser.ReadString(note_json, "customSickEffectModel", null);
                notes[i].damage_ratio = (float)JSONParser.ReadNumberDouble(note_json, "damageRatio", 1.0);
                notes[i].heal_ratio = (float)JSONParser.ReadNumberDouble(note_json, "healRatio", 1.0);
                notes[i].ignore_hit = JSONParser.ReadBoolean(note_json, "ignoreHit", false);
                notes[i].ignore_miss = JSONParser.ReadBoolean(note_json, "ignoreMiss", false);
                notes[i].can_kill_on_hit = JSONParser.ReadBoolean(note_json, "canKillOnHit", false);
                notes[i].is_special = JSONParser.ReadBoolean(note_json, "isSpecial", false);
                notes[i].model_src = JSONParser.ReadString(note_json, "model", null);
                notes[i].sick_effect_state_name = JSONParser.ReadString(note_json, "sickEffectStateName", null);
            }

            distribution.notes = notes;
            distribution.notes_size = notes_size;
        }

        public static void ParseDistributions(JSONToken json, out Distribution[] out_distributions, out int out_distributions_size) {
            JSONToken json_distributions = JSONParser.ReadArray(json, "distributions");
            int json_distributions_size = JSONParser.ReadArrayLength(json_distributions);

            if (json_distributions_size < 1) {
                out_distributions = null;
                out_distributions_size = 0;
                return;
            }

            Distribution[] distributions = new Distribution[json_distributions_size];
            int distributions_size = json_distributions_size;

            for (int i = 0 ; i < json_distributions_size ; i++) {
                distributions[i] = new Distribution() { };
                GameplayManifest.ParseDistribution(
                     JSONParser.ReadArrayItemObject(json_distributions, i), distributions[i]
                 );
            }

            out_distributions = distributions;
            out_distributions_size = distributions_size;
        }

        public static void ParseDistribution(JSONToken json_distribution, Distribution distribution) {
            if (json_distribution == null) throw new Exception("Invalid distribution found");

            JSONToken strums_array = JSONParser.ReadArray(json_distribution, "strums");
            int strums_array_size = JSONParser.ReadArrayLength(strums_array);
            JSONToken strums_binds_array = JSONParser.ReadArray(json_distribution, "buttonBinds");
            int strums_binds_array_size = JSONParser.ReadArrayLength(strums_array);


            distribution.notes = null;
            distribution.notes_size = 0;

            distribution.strums = null;
            distribution.strum_binds = null;
            distribution.strum_binds_is_custom = false;
            distribution.strums_size = 0;

            distribution.states = null;
            distribution.states_size = 0;


            if (strums_binds_array_size < 0) {
                switch (strums_array_size) {
                    case 4:
                        distribution.strum_binds = Strums.BUTTONS_BIND_4;
                        break;
                    case 5:
                        distribution.strum_binds = Strums.BUTTONS_BIND_5;
                        break;
                    case 6:
                        distribution.strum_binds = Strums.BUTTONS_BIND_6;
                        break;
                    case 7:
                        distribution.strum_binds = Strums.BUTTONS_BIND_7;
                        break;
                    case 8:
                        distribution.strum_binds = Strums.BUTTONS_BIND_8;
                        break;
                    case 9:
                        distribution.strum_binds = Strums.BUTTONS_BIND_9;
                        break;
                    default:
                        throw new Exception("Can not find the button binds for strums size=" + strums_array_size);
                }
            } else {
                if (strums_binds_array_size != strums_array_size) {
                    throw new Exception("'buttonBinds.length' != 'strums.length'");
                }
                if (strums_binds_array_size > 0) {
                    distribution.strum_binds = new GamepadButtons[strums_array_size];
                    distribution.strum_binds_is_custom = true;
                    for (int i = 0 ; i < strums_array_size ; i++) {
                        distribution.strum_binds[i] = (GamepadButtons)JSONParser.ReadArrayItemHex(strums_binds_array, i, (uint)GamepadButtons.NOTHING);
                    }
                }
            }

            GameplayManifest.ParseNotes(json_distribution, distribution);

            if (strums_array_size > 0) {
                distribution.strums = new DistributionStrum[strums_array_size];
                distribution.strums_size = strums_array_size;
            }

            for (int i = 0 ; i < strums_array_size ; i++) {
                JSONToken strum_json = JSONParser.ReadArrayItemObject(strums_array, i);

                distribution.strums[i] = new DistributionStrum() {
                    name = JSONParser.ReadString(strum_json, "name", null),
                    notes_ids = null,
                    notes_ids_size = 0
                };

                JSONToken note_ids_array = JSONParser.ReadArray(strum_json, "noteIds");
                int note_ids_array_size = JSONParser.ReadArrayLength(note_ids_array);

                if (note_ids_array_size > 0) {
                    distribution.strums[i].notes_ids_size = note_ids_array_size;
                    distribution.strums[i].notes_ids = new int[note_ids_array_size];
                }

                for (int j = 0 ; j < note_ids_array_size ; j++) {
                    int index = (int)JSONParser.ReadArrayItemNumberLong(note_ids_array, j, -1L);

                    if (index < 0) {
                        throw new Exception("Invalid note index found in strum index=" + i);
                    } else if (index >= distribution.notes_size) {
                        Console.Error.WriteLine("[ERROR] gameplaymanifest_parse_strums() invalid 'noteId' index = " + index);
                        index = 0;
                    }

                    distribution.strums[i].notes_ids[j] = index;
                }
            }

            JSONToken states_array = JSONParser.ReadArray(json_distribution, "states");
            int states_array_size = JSONParser.ReadArrayLength(states_array);

            if (states_array_size > 0) {
                distribution.states = new DistributionStrumState[states_array_size];
                distribution.states_size = states_array_size;
            }

            for (int i = 0 ; i < states_array_size ; i++) {
                JSONToken state_json = JSONParser.ReadArrayItemObject(states_array, i);
                distribution.states[i] = new DistributionStrumState() {
                    name = JSONParser.ReadString(state_json, "name", null),
                    model_marker = JSONParser.ReadString(state_json, "modelMarker", null),
                    model_sick_effect = JSONParser.ReadString(state_json, "modelSickEffect", null),
                    model_background = JSONParser.ReadString(state_json, "modelBackground", null),
                    model_notes = JSONParser.ReadString(state_json, "modelNotes", null)
                };
            }

        }

        public static void ParseDistributionsMinimal(JSONToken json, GameplayManifestDefault manifest) {
            JSONToken json_dists_minimal_array = JSONParser.ReadArray(json, "distributionsMinimal");
            int json_dists_minimal_array_length = JSONParser.ReadArrayLength(json_dists_minimal_array);

            if (json_dists_minimal_array_length < 0) return;

            if (json_dists_minimal_array_length < 1) {
                manifest.distributions = null;
                manifest.distributions_size = 0;
                return;
            }

            Distribution[] dists_minimal = new Distribution[json_dists_minimal_array_length];

            for (int i = 0 ; i < json_dists_minimal_array_length ; i++) {
                JSONToken json_dist = JSONParser.ReadArrayItemObject(json_dists_minimal_array, i);

                dists_minimal[i] = new Distribution() {
                    notes_size = Strums.DEFAULT_DISTRIBUTION.notes_size,
                    notes = Strums.DEFAULT_DISTRIBUTION.notes,

                    strums_size = Strums.DEFAULT_DISTRIBUTION.strums_size,
                    strums = Strums.DEFAULT_DISTRIBUTION.strums,
                    strum_binds = Strums.DEFAULT_DISTRIBUTION.strum_binds,
                    states = null,
                    states_size = 1
                };

                JSONToken dist_states_array = JSONParser.ReadArray(json_dist, "states");
                int dist_states_array_size = JSONParser.ReadArrayLength(dist_states_array);

                if (dist_states_array_size > 0) dists_minimal[i].states_size += dist_states_array_size;
                dists_minimal[i].states = new DistributionStrumState[dists_minimal[i].states_size];

                // build default state
                DistributionStrumState default_state = dists_minimal[i].states[0] = new DistributionStrumState() {
                    name = Strums.DEFAULT_DISTRIBUTION.states[0].name,
                    model_marker = Strums.DEFAULT_DISTRIBUTION.states[0].model_marker,
                    model_sick_effect = Strums.DEFAULT_DISTRIBUTION.states[0].model_sick_effect,
                    model_background = Strums.DEFAULT_DISTRIBUTION.states[0].model_background
                };

                // override values in the default state
                if (JSONParser.HasPropertyString(json_dist, "modelMarker")) {
                    default_state.model_marker = JSONParser.ReadString(json_dist, "modelMarker", null);
                }
                if (JSONParser.HasPropertyString(json_dist, "modelSickEffect")) {
                    default_state.model_sick_effect = JSONParser.ReadString(json_dist, "modelSickEffect", null);
                }
                if (JSONParser.HasPropertyString(json_dist, "modelBackground")) {
                    default_state.model_background = JSONParser.ReadString(json_dist, "modelBackground", null);
                }

                // read all remaining states
                for (int j = 0 ; j < dist_states_array_size ; j++) {
                    JSONToken state_json = JSONParser.ReadArrayItemObject(dist_states_array, i);
                    dists_minimal[i].states[j + 1] = new DistributionStrumState() {
                        name = JSONParser.ReadString(state_json, "name", null),
                        model_marker = JSONParser.ReadString(state_json, "modelMarker", null),
                        model_sick_effect = JSONParser.ReadString(state_json, "modelSickEffect", null),
                        model_background = JSONParser.ReadString(state_json, "modelBackground", null),
                        model_notes = JSONParser.ReadString(state_json, "modelNotes", null)
                    };
                }
            }

            manifest.distributions = dists_minimal;
            manifest.distributions_size = json_dists_minimal_array_length;
        }


        public static GameplayManifestHealthBar ParseHealthbar(JSONToken json) {
            JSONToken json_healthbar = JSONParser.ReadObject(json, "healthbar");
            if (json_healthbar == null) return null;

            GameplayManifestHealthBar healthbar = new GameplayManifestHealthBar() {
                disable = false,
                has_disable = false,

                opponent_color_rgb8 = 0x00,
                has_opponent_color = false,

                player_color_rgb8 = 0x00,
                has_player_color = false,

                warnings_model = null,
                has_warnings_model = false,

                use_alt_icons = false,
                has_use_alt_icons = false,

                states = null,
                states_size = 0
            };

            if (JSONParser.HasPropertyBoolean(json_healthbar, "disable")) {
                healthbar.disable = JSONParser.ReadBoolean(json_healthbar, "disable", healthbar.disable);
                healthbar.has_disable = true;
            }

            if (JSONParser.HasPropertyHex(json_healthbar, "opponentColor")) {
                healthbar.opponent_color_rgb8 = JSONParser.ReadHex(json_healthbar, "opponentColor", HealthBar.DEFAULT_COLOR_DAD);
                healthbar.has_opponent_color = true;
            }

            if (JSONParser.HasPropertyHex(json_healthbar, "playerColor")) {
                healthbar.player_color_rgb8 = JSONParser.ReadHex(json_healthbar, "playerColor", HealthBar.DEFAULT_COLOR_BOYFRIEND);
                healthbar.has_player_color = true;
            }

            if (JSONParser.HasProperty(json_healthbar, "warningsModel")) {
                healthbar.warnings_model = JSONParser.ReadString(json_healthbar, "warningsModel", null);
                healthbar.has_warnings_model = true;
            }

            if (JSONParser.HasProperty(json_healthbar, "useAltWarnIcons")) {
                healthbar.use_alt_icons = JSONParser.ReadBoolean(json_healthbar, "useAltWarnIcons", false);
                healthbar.has_use_alt_icons = true;
            }


            JSONToken states_array = JSONParser.ReadArray(json_healthbar, "states");
            int states_array_size = JSONParser.ReadArrayLength(states_array);

            if (states_array_size < 1) return healthbar;

            healthbar.states = new GameplayManifestHealthBarState[states_array_size];
            healthbar.states_size = states_array_size;

            for (int i = 0 ; i < states_array_size ; i++) {
                GameplayManifestHealthBarState state = healthbar.states[i] = new GameplayManifestHealthBarState() {
                    name = null,
                    opponent = new GameplayManifestHealthBarStateEntry() { },
                    player = new GameplayManifestHealthBarStateEntry() { },
                    background = new GameplayManifestHealthBarStateEntry() { }
                };

                JSONToken state_json = JSONParser.ReadArrayItemObject(states_array, i);
                if (state_json == null) {
                    Console.Error.WriteLine("[WARN] gameplaymanifest_parse_healthbar_states() invalid healthbar state at " + i);
                    continue;
                }

                state.name = JSONParser.ReadString(state_json, "name", null);
                GameplayManifest.ParseHealthbarStates(state.opponent, state_json, "opponent");
                GameplayManifest.ParseHealthbarStates(state.player, state_json, "player");
                GameplayManifest.ParseHealthbarStates(state.background, state_json, "background");
            }

            return healthbar;
        }

        public static void ParseHealthbarStates(GameplayManifestHealthBarStateEntry healthbar_state, JSONToken json_state, string property) {
            JSONToken json = JSONParser.ReadObject(json_state, property);
            healthbar_state.icon_model = JSONParser.ReadString(json, "iconModel", null);
            healthbar_state.bar_model = JSONParser.ReadString(json, "barModel", null);
            healthbar_state.bar_color = JSONParser.ReadHex(json, "barColor", 0x000000);
        }


        public static void ParsePlayers(JSONToken json, out GameplayManifestPlayer[] out_players, out int out_players_size) {
            JSONToken players_array = JSONParser.ReadArray(json, "players");
            int players_array_size = JSONParser.ReadArrayLength(players_array);

            if (players_array_size < 1) {
                out_players = null;
                out_players_size = 0;
                return;
            }

            GameplayManifestPlayer[] players = new GameplayManifestPlayer[players_array_size];
            int players_size = players_array_size;

            for (int i = 0 ; i < players_array_size ; i++) {
                JSONToken player_json = JSONParser.ReadArrayItemObject(players_array, i);

                players[i] = new GameplayManifestPlayer() {
                    manifest = JSONParser.ReadString(player_json, "manifest", null),
                    refer = GameplayManifestRefer.NONE,
                    controller = (int)JSONParser.ReadNumberLong(player_json, "controller", -1L),
                    layout_strums_id = (int)JSONParser.ReadNumberLong(player_json, "strumsInLayoutId", -1L),
                    distribution_index = (int)JSONParser.ReadNumberLong(player_json, "distributionIndex", 0L),
                    states = null,
                    states_size = 0,
                    can_die = false,
                    can_recover = false,
                    is_opponent = false,
                };

                bool has_is_opponent = JSONParser.HasPropertyBoolean(player_json, "isOpponent");

                if (has_is_opponent) {
                    players[i].is_opponent = JSONParser.ReadBoolean(player_json, "isOpponent", false);
                } else {
                    //
                    // bot players:   can_die=false  can_recover=false
                    // real players:  can_die=true   can_recover=true
                    //
                    players[i].can_recover = players[i].can_die = players[i].controller >= 0;
                    players[i].is_opponent = players[i].controller < 0;
                }

                if (JSONParser.HasPropertyBoolean(player_json, "canDie")) {
                    players[i].can_die = JSONParser.ReadBoolean(player_json, "canDie", false);
                } else if (has_is_opponent) {
                    players[i].can_die = !players[i].is_opponent;
                }

                if (JSONParser.HasPropertyBoolean(player_json, "canRecover")) {
                    players[i].can_recover = JSONParser.ReadBoolean(player_json, "canRecover", false);
                } else if (has_is_opponent) {
                    players[i].can_recover = !players[i].is_opponent;
                }

                players[i].refer = GameplayManifest.HasDefaultModel(players[i].manifest);
                if (players[i].refer != GameplayManifestRefer.NONE) {
                    // a default character manifest was specified, ignore
                    //free(players[i].manifest);
                    players[i].manifest = null;
                }

                JSONToken states_array = JSONParser.ReadArray(player_json, "states");
                int states_array_size = JSONParser.ReadArrayLength(states_array);

                if (states_array_size < 1) continue;

                players[i].states = new GameplayManifestCharacterState[states_array_size];
                players[i].states_size = states_array_size;

                for (int j = 0 ; j < states_array_size ; j++) {
                    JSONToken state_json = JSONParser.ReadArrayItemObject(states_array, j);
                    players[i].states[j] = new GameplayManifestCharacterState() {
                        name = JSONParser.ReadString(state_json, "name", null),
                        model = JSONParser.ReadString(state_json, "model", null)
                    };
                }
            }

            out_players = players;
            out_players_size = players_size;
        }

        public static GameplayManifestGirlfriend ParseGirlfriend(JSONToken json) {
            JSONToken json_girlfriend = JSONParser.ReadObject(json, "girlfriend");
            if (json_girlfriend == null) return null;

            GameplayManifestGirlfriend girlfriend = new GameplayManifestGirlfriend() {
                manifest = null,
                refer = GameplayManifestRefer.NONE,
                states = null,
                states_size = 0
            };

            girlfriend.manifest = JSONParser.ReadString(json_girlfriend, "manifest", null);
            girlfriend.refer = GameplayManifest.HasDefaultModel(girlfriend.manifest);

            if (girlfriend.refer != GameplayManifestRefer.NONE) {
                // a default character manifest was specified, ignore
                //free(girlfriend.manifest);
                girlfriend.manifest = null;
            }

            JSONToken states_array = JSONParser.ReadArray(json_girlfriend, "states");
            int states_array_size = JSONParser.ReadArrayLength(states_array);

            if (states_array_size < 1) return girlfriend;

            girlfriend.states = new GameplayManifestCharacterState[states_array_size];
            girlfriend.states_size = states_array_size;

            for (int i = 0 ; i < states_array_size ; i++) {
                JSONToken state_json = JSONParser.ReadArrayItemObject(states_array, i);
                girlfriend.states[i] = new GameplayManifestCharacterState() {
                    name = JSONParser.ReadString(state_json, "name", null),
                    model = JSONParser.ReadString(state_json, "model", null)
                };
            }

            return girlfriend;
        }


        public static GameplayManifestRefer HasDefaultModel(string charactermanifest_src) {
            if (charactermanifest_src == GameplayManifest.DEFAULT_GIRLFRIEND)
                return GameplayManifestRefer.GIRLFRIEND;
            else if (charactermanifest_src == GameplayManifest.DEFAULT_BOYFRIEND)
                return GameplayManifestRefer.BOYFRIEND;
            else
                return GameplayManifestRefer.NONE;
        }

    }

}
