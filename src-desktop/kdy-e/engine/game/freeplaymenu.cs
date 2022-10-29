using System;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Game.Gameplay.Helpers;
using Engine.Game.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS.MUTEX;
using KallistiOS.THD;

namespace Engine.Game {

    public class FreeplayMenu {

        private const string BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME = "freeplay-background";
        private const string PERSONAL_BEST = "PERSONAL BEST SCORE: $l";
        private const string INFO = "$s  -  $s  -  $s";


        private static readonly MenuManifest MENU_ICONS = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = null,// unused
                suffix_idle = null,// unused
                suffix_choosen = null,// unused
                suffix_discarded = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = null,
                animlist = null,

                anim_selected = null,
                anim_idle = null,
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0,// unused
                anim_transition_out_delay = 0,// unused

                font = null,
                font_glyph_suffix = null,// unused
                font_color_by_difference = false,// unused
                font_size = Single.NaN,// unused
                font_color = 0xFFFFFF,// unused
                font_border_color = 0x000000FF,// unused
                font_border_size = 4,// unused

                is_sparse = false,// unused
                is_vertical = true,
                is_per_page = false,
                static_index = 1,

                items_align = Align.START,
                items_gap = 58f,
                items_dimmen = 0f,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true// unused
            },
            items = null,
            items_size = 0
        };
        private static readonly MenuManifest MENU_SONGS = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = null,// unused
                suffix_idle = null,// unused
                suffix_choosen = null,// unused
                suffix_discarded = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = null,
                animlist = "/assets/common/anims/freeplay-menu.xml",

                anim_selected = "menu_item_selected",
                anim_idle = "menu_item_idle",
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0,// unused
                anim_transition_out_delay = 0,// unused

                font = null,
                font_glyph_suffix = null,// unused
                font_color_by_difference = false,// unused
                font_size = 28f,
                font_color = 0xFFFFFF,
                font_border_color = 0x000000FF,// unused
                font_border_size = 4,// unused

                is_sparse = false,// unused
                is_vertical = true,
                is_per_page = false,
                static_index = 1,

                items_align = Align.START,
                items_gap = 58f,
                items_dimmen = 0f,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true// unused
            },
            items = null,
            items_size = 0
        };

        public static void Main() {
            Layout layout = Layout.Init("/assets/common/image/freeplay-menu/layout.xml");
            LayoutPlaceholder placeholder_icons = layout.GetPlaceholder("menu_icons");
            LayoutPlaceholder placeholder_tracks = layout.GetPlaceholder("menu_songs");

            MenuManifest.Parameters params_icons = FreeplayMenu.MENU_ICONS.parameters;
            MenuManifest.Parameters params_tracks = FreeplayMenu.MENU_SONGS.parameters;


            params_tracks.font = (string)layout.GetAttachedValue(
                "menu_font", AttachedValueType.STRING, "/assets/common/font/Alphabet.xml"
            );
            params_tracks.font_glyph_suffix = (string)layout.GetAttachedValue(
                "menu_fontGlyphSuffix", AttachedValueType.STRING, "bold"
            );
            params_tracks.font_size = params_tracks.items_dimmen = layout.GetAttachedValueAsFloat(
                "menu_fontSize", 46f
            );
            params_tracks.font_color = (uint)layout.GetAttachedValue(
                "menu_fontColor", AttachedValueType.HEX, 0xFFFFFFU
            );
            params_tracks.font_border_color = (uint)layout.GetAttachedValue(
                "menu_fontBorderColor", AttachedValueType.HEX, 0x000000U
            );
            params_tracks.font_border_size = layout.GetAttachedValueAsFloat(
                "menu_fontBorderSize", 0f
            );
            params_tracks.items_gap = layout.GetAttachedValueAsFloat(
                "menu_itemsGap", 58f
            );

            params_icons.items_dimmen = params_tracks.font_size;
            params_icons.items_gap = params_tracks.items_gap;
            params_icons.items_align = params_tracks.items_align;


            // step 3: count required tracks
            ArrayList<MappedSong> songs = new ArrayList<MappedSong>(Funkin.weeks_array.size * 3);
            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                for (int j = 0 ; j < Funkin.weeks_array.array[i].songs_count ; j++) {
                    bool should_hide = Funkin.weeks_array.array[i].songs[j].freeplay_hide_if_week_locked;
                    bool is_locked = !FunkinSave.ContainsUnlockDirective(Funkin.weeks_array.array[i].unlock_directive);

                    if (should_hide && is_locked) continue;

                    songs.Add(new MappedSong() {
                        song_index = j,
                        week_index = i,
                        is_locked = is_locked
                    });
                }
            }

            // step 4: prepare menus
            int songs_size = songs.Size();
            FreeplayMenu.MENU_ICONS.items_size = songs_size;
            FreeplayMenu.MENU_SONGS.items_size = songs_size;
            FreeplayMenu.MENU_ICONS.items = new MenuManifest.Item[songs_size];
            FreeplayMenu.MENU_SONGS.items = new MenuManifest.Item[songs_size];

            string last_model = null;
            bool last_model_exists = false;
            for (int i = 0 ; i < songs_size ; i++) {
                MappedSong track = songs.Get(i);
                WeekInfo weekinfo = Funkin.weeks_array.array[track.week_index];

                string track_name = weekinfo.songs[track.song_index].name;
                string icon_name = weekinfo.songs[track.song_index].freeplay_host_icon_name;
                string model_path = weekinfo.songs[track.song_index].freeplay_host_icon_model;

                if (icon_name == null) icon_name = HealthBar.ICON_PREFIX_NEUTRAL;
                if (model_path == null) model_path = weekinfo.songs_default_freeplay_host_icon_model;

                if (model_path != last_model) {
                    last_model = model_path;
                    last_model_exists = FS.FileExists(model_path);
                }
                if (!last_model_exists) model_path = null;

                FreeplayMenu.MENU_ICONS.items[i] = new MenuManifest.Item() {
                    modelholder = model_path,
                    name = icon_name,
                };
                FreeplayMenu.MENU_SONGS.items[i] = new MenuManifest.Item() {
                    text = track_name
                };
            }

            // step 5: build menus
            Menu menu_icons = new Menu(
                FreeplayMenu.MENU_ICONS,
                placeholder_icons.x,
                placeholder_icons.y,
                placeholder_icons.z,
                placeholder_icons.width,
                placeholder_icons.height
            );
            Menu menu_songs = new Menu(
                FreeplayMenu.MENU_SONGS,
                placeholder_tracks.x,
                placeholder_tracks.y,
                placeholder_tracks.z,
                placeholder_tracks.width,
                placeholder_tracks.height
            );
            menu_songs.SetTextForceCase(VertexProps.TEXTSPRITE_FORCE_UPPERCASE);
            placeholder_icons.vertex = menu_icons.GetDrawable();
            placeholder_tracks.vertex = menu_songs.GetDrawable();
            menu_icons.SelectIndex(-1);
            menu_songs.SelectIndex(-1);

            // step 6: drop menu manifests
            //free(FreeplayMenu.MENU_ICONS.items);
            //free(FreeplayMenu.MENU_SONGS.items);
            FreeplayMenu.MENU_ICONS.items = FreeplayMenu.MENU_SONGS.items = null;

            State state = new State() {
                difficult_index = -1,
                difficult_locked = false,
                use_alternative = false,
                map = songs.Get(0),
                async_id_song = 0,
                async_id_background = 0,
                soundplayer_path = null,
                soundplayer = null,
                background = layout.GetSprite("custom_background"),
                layout = layout,
                personal_best = layout.GetTextsprite("personal_best"),
                info = layout.GetTextsprite("info"),
                description = layout.GetTextsprite("description"),
                difficulties = null,
                difficulties_size = 0
            };
            mutex.Init(out state.mutex, mutex.TYPE_NORMAL);
            if (state.background != null) state.background.SetTexture(null, false);

            string default_bf = FreeplayMenu.InternalGetDefaultCharacterManifest(true);
            string default_gf = FreeplayMenu.InternalGetDefaultCharacterManifest(false);


            while (true) {
                int map_index = Show(menu_icons, menu_songs, state, songs);
                if (map_index < 0) break;// back to main menu

                string difficult = state.difficulties[state.difficult_index].name;
                WeekInfo weekinfo = Funkin.weeks_array.array[songs.Get(map_index).week_index];
                string gameplaymanifest = weekinfo.songs[state.map.song_index].freeplay_gameplaymanifest;
                InternalDropSoundplayer(state);

                layout.Suspend();
                int ret = Week.Main(
                    weekinfo, state.use_alternative, difficult, default_bf, default_gf,
                    gameplaymanifest, state.map.song_index
                );
                layout.Resume();
                if (ret == 3) break;// back to main menu
            }

            // Wait for running threads
            while (true) {
                PVRContext.global_context.WaitReady();
                mutex.Lock(state.mutex);
                if (state.running_threads < 1) break;
                mutex.Unlock(state.mutex);
            }
            thd.pass();

            FreeplayMenu.InternalDropCustomBackground(state);

            //free(state.difficulties);
            mutex.Destroy(state.mutex);
            InternalDropSoundplayer(state);
            menu_icons.Destroy();
            menu_songs.Destroy();
            songs.Destroy(false);
            layout.Destroy();
        }

        private static int Show(Menu menu_icons, Menu menu_songs, State state, ArrayList<MappedSong> songs) {
            int map_index = -1;
            Gamepad gamepad = new Gamepad(-1);
            gamepad.SetButtonsDelay(250);

            SoundPlayer sound_asterik = SoundPlayer.Init("/assets/common/sound/asterikMenu.ogg");

            if (menu_icons.GetSelectedIndex() < 0) {
                menu_icons.SelectIndex(0);
                menu_songs.SelectIndex(0);

                state.map = songs.Get(menu_songs.GetSelectedIndex());
                state.use_alternative = false;
                FreeplayMenu.InternalBuildDifficulties(state);
                FreeplayMenu.InternalShowInfo(state);
                FreeplayMenu.InternalLoadBackgroundAsync(state);
                FreeplayMenu.InternalLoadSongAsync(state);
            }


            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();
                state.layout.Animate(elapsed);
                state.layout.Draw(PVRContext.global_context);

                GamepadButtons btns = gamepad.HasPressedDelayed(
                    GamepadButtons.DPAD_UP | GamepadButtons.DPAD_DOWN |
                    GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD_RIGHT |
                    GamepadButtons.X | GamepadButtons.A | GamepadButtons.B |
                    GamepadButtons.BACK | GamepadButtons.START
                );

                int offset;
                bool switch_difficult;

                if ((btns & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    break;
                } else if ((btns & (GamepadButtons.X)) != GamepadButtons.NOTHING) {
                    WeekInfo weeinfo = Funkin.weeks_array.array[state.map.week_index];
                    if (!String.IsNullOrEmpty(weeinfo.warning_message))
                        state.use_alternative = !state.use_alternative;
                    else if (sound_asterik != null)
                        sound_asterik.Replay();
                    continue;
                } else if ((btns & GamepadButtons.DPAD_UP) != GamepadButtons.NOTHING) {
                    offset = -1;
                    switch_difficult = false;
                } else if ((btns & GamepadButtons.DPAD_DOWN) != GamepadButtons.NOTHING) {
                    offset = 1;
                    switch_difficult = false;
                } else if ((btns & GamepadButtons.DPAD_LEFT) != GamepadButtons.NOTHING) {
                    offset = -1;
                    switch_difficult = true;
                } else if ((btns & GamepadButtons.DPAD_RIGHT) != GamepadButtons.NOTHING) {
                    offset = 1;
                    switch_difficult = true;
                } else if ((btns & (GamepadButtons.A | GamepadButtons.START)) != GamepadButtons.NOTHING) {
                    int index = menu_songs.GetSelectedIndex();
                    if (index < 0 || index >= menu_songs.GetItemsCount() || state.difficult_locked) {
                        if (sound_asterik != null) sound_asterik.Replay();
                        continue;
                    }

                    map_index = index;
                    break;
                } else {
                    continue;
                }

                if (switch_difficult) {
                    int new_index = state.difficult_index + offset;
                    if (new_index < 0 || new_index >= state.difficulties_size) {
                        if (sound_asterik != null) sound_asterik.Replay();
                    } else {
                        state.difficult_index = new_index;
                        state.difficult_locked = state.difficulties[new_index].is_locked;
                        FreeplayMenu.InternalShowInfo(state);
                    }
                    continue;
                }

                menu_icons.SelectVertical(offset);
                if (!menu_songs.SelectVertical(offset)) {
                    int index = menu_songs.GetSelectedIndex();
                    if (index < 1)
                        index = menu_songs.GetItemsCount() - 1;
                    else
                        index = 0;

                    menu_icons.SelectIndex(index);
                    menu_songs.SelectIndex(index);
                }

                state.map = songs.Get(menu_songs.GetSelectedIndex());
                state.use_alternative = false;
                FreeplayMenu.InternalBuildDifficulties(state);
                FreeplayMenu.InternalShowInfo(state);
                FreeplayMenu.InternalLoadBackgroundAsync(state);
                FreeplayMenu.InternalLoadSongAsync(state);
            }

            if (sound_asterik != null) sound_asterik.Destroy();
            gamepad.Destroy();

            return map_index;
        }

        private static void InternalLoadSongAsync(State state) {
            // adquire mutex and declare this thread
            mutex.Lock(state.mutex);
            state.running_threads++;

            WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
            WeekInfo.Song songinfo = weekinfo.songs[state.map.song_index];
            int async_id_song = state.async_id_song;
            string path_base = songinfo.freeplay_song_filename;
            bool src_is_null = songinfo.freeplay_song_filename == null;

            if (state.map.is_locked || songinfo.name == null) {
                InternalDropSoundplayer(state);

                state.running_threads--;
                mutex.Unlock(state.mutex);
                return;
            }

            // shared variables adquired, release mutex
            mutex.Unlock(state.mutex);

            // guess the filename
            if (src_is_null) {
                string temp_nospaces = songinfo.name.Replace('\x20', '-');
                string temp_lowercase = StringUtils.ToLowerCase(temp_nospaces);
                //free(temp_nospaces);
                string temp_relativepath = StringUtils.Concat(Funkin.WEEK_SONGS_FOLDER, temp_lowercase, ".ogg");
                //free(temp_lowercase);
                path_base = WeekEnumerator.GetAsset(weekinfo, temp_relativepath);
                //free(temp_relativepath);
            }

            // get the path of instrumetal track of the song
            string path_voices, path_instrumental;
            bool is_not_splitted = SongPlayer.HelperGetTracks(
                path_base, state.use_alternative, out path_voices, out path_instrumental
            );
            string final_path = is_not_splitted ? path_base : (path_instrumental ?? path_voices);

            if (async_id_song != state.async_id_song) {
                goto L_return;
            }

            if (final_path == state.soundplayer_path) {
                goto L_return;
            }
            if (final_path == null) {
                InternalDropSoundplayer(state);
                goto L_return;
            }

            // instance a soundplayer
            SoundPlayer soundplayer = SoundPlayer.Init(final_path);
            if (soundplayer == null) final_path = null;

            // check if the user selected another song
            if (async_id_song != state.async_id_song) {
                if (soundplayer != null) soundplayer.Destroy();
                goto L_return;
            }

            // adquire mutex and swap the soundplayer
            mutex.Lock(state.mutex);

            InternalDropSoundplayer(state);
            state.soundplayer_path = final_path;// strdup(final_path)
            state.soundplayer = soundplayer;

            soundplayer.SetVolume(0.5f);
            soundplayer.Play();
            soundplayer.Fade(true, 500);

            mutex.Unlock(state.mutex);

L_return:
            mutex.Lock(state.mutex);
            //if (src_is_null) free(path_base);
            //free(path_voices);
            //free(path_instrumental);
            state.running_threads--;
            mutex.Unlock(state.mutex);
        }

        private static void InternalLoadBackgroundAsync(State state) {
            if (state.background == null) return;

            // adquire mutex and declare this thread
            mutex.Lock(state.mutex);
            state.running_threads++;

            WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
            string src = weekinfo.songs[state.map.song_index].freeplay_background;
            int async_id_background = state.async_id_background;
            ModelHolder modelholder = null;
            Texture texture = null;

            // check if the selected song has a custom background
            if (String.IsNullOrEmpty(src)) {
                FreeplayMenu.InternalDropCustomBackground(state);
                state.running_threads--;
                mutex.Unlock(state.mutex);
                return;
            }

            if (src.LowercaseEndsWithKDY(".json") || src.LowercaseEndsWithKDY(".xml")) {
                modelholder = ModelHolder.Init(src);
            } else {
                // assume is a image file
                texture = Texture.Init(src);
            }

            mutex.Lock(state.mutex);

            // if the user has no changed the song, set the background
            if (async_id_background == state.async_id_background) {
                AnimSprite sprite_anim = null;
                Texture sprite_tex = null;

                if (modelholder != null) {
                    sprite_tex = modelholder.GetTexture(true);
                    sprite_anim = modelholder.CreateAnimsprite(
                        FreeplayMenu.BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME, true, false
                    );
                } else if (texture != null) {
                    sprite_tex = texture.ShareReference();
                }

                FreeplayMenu.InternalDropCustomBackground(state);
                if (sprite_tex != null) state.background.SetTexture(sprite_tex, true);
                if (sprite_anim != null) state.background.ExternalAnimationSet(sprite_anim);

                if (sprite_tex == null)
                    state.layout.TriggerAny("track-background-hide");
                else
                    state.layout.TriggerAny("track-background-set");
            }

            if (modelholder != null) modelholder.Destroy();
            if (texture != null) texture.Destroy();

            state.running_threads--;
            mutex.Unlock(state.mutex);
        }

        private static void InternalShowInfo(State state) {
            Layout layout = state.layout;
            WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
            string desc = weekinfo.songs[state.map.song_index].freeplay_description;
            string week_name = weekinfo.display_name;
            string song_name = weekinfo.songs[state.map.song_index].name;

            if (String.IsNullOrEmpty(weekinfo.display_name)) week_name = weekinfo.name;

            string difficult;
            bool is_locked;

            long score;
            if (state.difficult_index >= 0 && state.difficult_index < state.difficulties_size) {
                difficult = state.difficulties[state.difficult_index].name;
                score = FunkinSave.GetWeekScore(weekinfo.name, difficult);
                is_locked = state.difficulties[state.difficult_index].is_locked;
            } else {
                score = 0;
                is_locked = true;
                difficult = null;
            }

            if (state.personal_best != null)
                state.personal_best.SetTextFormated(FreeplayMenu.PERSONAL_BEST, score);

            if (state.info != null)
                state.info.SetTextFormated(FreeplayMenu.INFO, week_name, song_name, difficult);

            if (desc != null) {
                if (state.description != null)
                    state.description.SetTextIntern(true, desc);
                state.layout.TriggerAny("description-show");
            } else {
                state.layout.TriggerAny("description-hide");
            }

            layout.TriggerAny(is_locked ? "locked" : "not-locked");

            if (!String.IsNullOrEmpty(weekinfo.warning_message))
                layout.TriggerAny(state.use_alternative ? "use-alternative" : "not-use-alternative");
            else
                layout.TriggerAny("hide-alternative");
        }

        private static void InternalDropSoundplayer(State state) {
            if (state.soundplayer != null) {
                state.soundplayer.Stop();
                state.soundplayer.Destroy();
                state.soundplayer = null;
            }

            if (state.soundplayer_path != null) {
                //free(state.soundplayer_path);
                state.soundplayer_path = null;
            }
        }

        private static void InternalDropCustomBackground(State state) {
            if (state.background == null) return;
            Texture tex_old = state.background.SetTexture(null, false);
            AnimSprite anim_old = state.background.ExternalAnimationSet(null);
            if (tex_old != null) tex_old.Destroy();
            if (anim_old != null) anim_old.Destroy();
        }

        private static void InternalBuildDifficulties(State state) {
            //if (state.difficulties != null) free(state.difficulties);

            WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
            int count = 3 + weekinfo.custom_difficults_size;

            state.difficulties = new Difficult[count];
            state.difficulties_size = 0;

            if (weekinfo.has_difficulty_easy) {
                state.difficulties[state.difficulties_size++] = new Difficult() {
                    name = Funkin.DIFFICULT_EASY,
                    is_locked = false
                };
            }
            if (weekinfo.has_difficulty_normal) {
                state.difficulties[state.difficulties_size++] = new Difficult() {
                    name = Funkin.DIFFICULT_NORMAL,
                    is_locked = false
                };
            }
            if (weekinfo.has_difficulty_hard) {
                state.difficulties[state.difficulties_size++] = new Difficult() {
                    name = Funkin.DIFFICULT_HARD,
                    is_locked = false
                };
            }

            for (int i = 0 ; i < weekinfo.custom_difficults_size ; i++) {
                bool is_locked = FunkinSave.ContainsUnlockDirective(weekinfo.custom_difficults[i].unlock_directive);
                state.difficulties[state.difficulties_size++] = new Difficult() {
                    name = weekinfo.custom_difficults[i].name,
                    is_locked = is_locked
                };
            }

            // choose default difficult
            state.difficult_index = 0;
            string default_difficult = weekinfo.default_difficulty ?? Funkin.DIFFICULT_NORMAL;
            for (int i = 0 ; i < state.difficulties_size ; i++) {
                if (state.difficulties[i].name == default_difficult) {
                    state.difficult_index = i;
                    break;
                }
            }

        }

        private static string InternalGetDefaultCharacterManifest(bool is_boyfriend) {
            string src = is_boyfriend ? WeekSelectorMdlSelect.MODELS_BF : WeekSelectorMdlSelect.MODELS_GF;
            JSONParser json = JSONParser.LoadFrom(src);

            if (json == null) return null;

            int array_length = JSONParser.ReadArrayLength(json);
            string manifest_src = null;

            for (int i = 0 ; i < array_length ; i++) {
                JSONToken item = JSONParser.ReadArrayItemObject(json, i);
                if (item == null) continue;

                string unlock_directive = JSONParser.ReadString(item, "unlockDirectiveName", null);
                if (!FunkinSave.ContainsUnlockDirective(unlock_directive)) continue;

                string model = JSONParser.ReadString(item, "manifest", null);
                if (String.IsNullOrEmpty(model)) continue;

                manifest_src = FS.BuildPath2(src, model);
                break;
            }

            JSONParser.Destroy(json);
            return manifest_src;
        }


        private struct MappedSong {
            public int song_index;
            public int week_index;
            public bool is_locked;
        }

        private struct Difficult {
            public string name;
            public bool is_locked;
        }

        private class State {
            public int difficult_index;
            public bool difficult_locked;
            public bool use_alternative;
            public MappedSong map;

            public volatile int async_id_song;
            public volatile int async_id_background;

            public SoundPlayer soundplayer;
            public string soundplayer_path;

            public mutex_t mutex;
            public volatile int running_threads;

            public Sprite background;
            public Layout layout;

            public TextSprite personal_best;
            public TextSprite info;
            public TextSprite description;

            public Difficult[] difficulties;
            public int difficulties_size;
        }

    }
}
