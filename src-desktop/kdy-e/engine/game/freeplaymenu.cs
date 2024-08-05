using System;
using System.Threading;
using CsharpWrapper;
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

namespace Engine.Game;

public class FreeplayMenu {

    private const string BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME = "freeplay-background";
    private const string PERSONAL_BEST = "PERSONAL BEST SCORE: $l";
    private const string INFO = "$s  -  $s  -  $s";
    private const string BG_INFO_NAME = "background_song_info";
    private const string LAYOUT = "/assets/common/image/freeplay-menu/layout.xml";
    private const string LAYOUT_DREAMCAST = "/assets/common/image/freeplay-menu/layout~dreamcast.xml";
    private const string MODDING_SCRIPT = "/assets/common/data/scripts/freeplaymenu.lua";


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
            font_color_by_addition = false,// unused
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
        string src_layout = PVRContext.global_context.IsWidescreen() ? FreeplayMenu.LAYOUT : FreeplayMenu.LAYOUT_DREAMCAST;
        Layout layout = Layout.Init(src_layout);
        if (layout == null) {
            Logger.Error("freeplay_main() missing layout");
            return;
        }

        PVRContext.MuteAudioOutputOnMinimized(false);

        LayoutPlaceholder placeholder = layout.GetPlaceholder("menu");
        MenuManifest.Parameters @params = FreeplayMenu.MENU_SONGS.parameters;


        @params.font = (string)layout.GetAttachedValue(
            "menu_font", AttachedValueType.STRING, "/assets/common/font/Alphabet.xml"
        );
        @params.font_glyph_suffix = (string)layout.GetAttachedValue(
            "menu_fontGlyphSuffix", AttachedValueType.STRING, "bold"
        );
        @params.font_size = @params.items_dimmen = layout.GetAttachedValueAsFloat(
            "menu_fontSize", 46f
        );
        @params.font_color = (uint)layout.GetAttachedValue(
            "menu_fontColor", AttachedValueType.HEX, 0xFFFFFFU
        );
        @params.font_border_color = (uint)layout.GetAttachedValue(
            "menu_fontBorderColor", AttachedValueType.HEX, 0x000000U
        );
        @params.font_border_size = layout.GetAttachedValueAsFloat(
            "menu_fontBorderSize", 0f
        );
        @params.items_gap = layout.GetAttachedValueAsFloat(
            "menu_itemsGap", 58f
        );
        float icons_dimmen = layout.GetAttachedValueAsFloat(
            "menu_iconsDimmen", 70f
        );

        float dt_playsong = layout.GetAttachedValueAsFloat("durationTransition_playSong", 0f);
        float dt_screenout = layout.GetAttachedValueAsFloat("durationTransition_screenOut", 0f);

        float bg_info_width = -1;
        Sprite bg_info = layout.GetSprite(FreeplayMenu.BG_INFO_NAME);
        if (bg_info != null) {
            bg_info.GetDrawSize(out bg_info_width, out _);
        }

        // step 3: count required songs
        ArrayList<MappedSong> songs = new ArrayList<MappedSong>(Funkin.weeks_array.size * 3);
        for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
            WeekInfo weekinfo = Funkin.weeks_array.array[i];
            bool is_week_locked = !FunkinSave.ContainsUnlockDirective(weekinfo.unlock_directive);

            for (int j = 0 ; j < weekinfo.songs_count ; j++) {
                WeekInfo.Song song = weekinfo.songs[j];

                bool should_hide = song.freeplay_hide_if_week_locked;
                if (should_hide && is_week_locked) continue;

                should_hide = song.freeplay_hide_if_locked;
                bool is_song_locked = !FunkinSave.ContainsUnlockDirective(song.freeplay_unlock_directive);
                if (should_hide && is_song_locked) continue;

                int gameplaymanifest_index = song.freeplay_song_index_in_gameplaymanifest;
                if (gameplaymanifest_index < 0) gameplaymanifest_index = j;

                songs.Add(new MappedSong() {
                    song_index = j,
                    gameplaymanifest_index = gameplaymanifest_index,
                    week_index = i,
                    is_locked = is_week_locked || is_song_locked
                });
            }
        }

        // step 4: prepare menu
        int songs_size = songs.Size();
        FreeplayMenu.MENU_SONGS.items_size = songs_size;
        FreeplayMenu.MENU_SONGS.items = EngineUtils.CreateArray<MenuManifest.Item>(songs_size);

        for (int i = 0 ; i < songs_size ; i++) {
            MappedSong song = songs.Get(i);
            WeekInfo weekinfo = Funkin.weeks_array.array[song.week_index];

            FreeplayMenu.MENU_SONGS.items[i] = new MenuManifest.Item() {
                text = weekinfo.songs[song.song_index].name
            };
        }

        // step 5: build menu
        Menu menu_songs = new Menu(
            FreeplayMenu.MENU_SONGS,
            placeholder.x,
            placeholder.y,
            placeholder.z,
            placeholder.width,
            placeholder.height
        );
        menu_songs.SetTextForceCase(VertexProps.TEXTSPRITE_FORCE_UPPERCASE);
        placeholder.vertex = menu_songs.GetDrawable();
        menu_songs.SelectIndex(-1);

        // step 6: drop menu manifests
        //free(FreeplayMenu.MENU_SONGS.items);
        FreeplayMenu.MENU_SONGS.items = null;

        // step 7: create menu icons
        FreeplaySongIcons songicons = new FreeplaySongIcons(songs, icons_dimmen, @params.font_size);
        menu_songs.SetDrawCallback(false, songicons.DrawItemIcon);

        // step 8: initialize modding
        Modding modding = new Modding(layout, FreeplayMenu.MODDING_SCRIPT);
        modding.native_menu = modding.active_menu = menu_songs;
        modding.callback_private_data = null;
        modding.callback_option = null;
        modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);


        State state = new State() {
            difficult_index = -1,
            difficult_locked = false,
            use_alternative = false,
            map = songs.Get(0),
            async_id_operation = 0,
            soundplayer_path = null,
            soundplayer = null,
            background = layout.GetSprite("custom_background"),
            layout = layout,
            personal_best = layout.GetTextsprite("personal_best"),
            info = layout.GetTextsprite("info"),
            description = layout.GetTextsprite("description"),
            difficulties = null,
            difficulties_size = 0,
            running_threads = 0,
            mutex = null,
            bg_info_width = bg_info_width,
            modding = modding,
            song_preview_volume = layout.GetAttachedValueAsFloat("song_preview_volume", 0.7f)
        };
        mutex.Init(out state.mutex, mutex.TYPE_NORMAL);
        if (state.background != null) state.background.SetTexture(null, false);

        string default_bf = FreeplayMenu.HelperGetDefaultCharacterManifest(true);
        string default_gf = FreeplayMenu.HelperGetDefaultCharacterManifest(false);

        if (GameMain.background_menu_music != null) {
            GameMain.background_menu_music.Pause();
        }

        layout.TriggerAny("transition-in");
        modding.HelperNotifyEvent("transition-in");

        while (true) {
            modding.has_exit = false;
            modding.has_halt = false;

            int map_index = FreeplayMenu.Show(menu_songs, state, songs);
            if (map_index < 0) break;// back to main menu

            string difficult = state.difficulties[state.difficult_index].name;
            WeekInfo weekinfo = Funkin.weeks_array.array[songs.Get(map_index).week_index];
            string gameplaymanifest = weekinfo.songs[state.map.song_index].freeplay_gameplaymanifest;
            InternalDropSoundplayer(state, true);

            FreeplayMenu.InternalWaitTransition(state, "before-play-song", dt_playsong);

            layout.Suspend();
            PVRContext.MuteAudioOutputOnMinimized(EngineSettings.mute_on_minimize);

            GameMain.HelperDrawLoadingScreen();

            int ret = Week.Main(
                weekinfo, state.use_alternative, difficult, default_bf, default_gf,
                gameplaymanifest, state.map.gameplaymanifest_index, "RETURN TO FREEPLAY MENU"
            );
            if (ret == 0) break;// back to main menu

            PVRContext.MuteAudioOutputOnMinimized(false);
            layout.Resume();
            FreeplayMenu.InternalWaitTransition(state, "after-play-song", dt_playsong);
            FreeplayMenu.InternalSongLoad(state, false);
            FreeplayMenu.InternalShowInfo(state);
        }

        Interlocked.Increment(ref state.async_id_operation);
        while (state.running_threads > 0) {
            // wait until all async operations are done
            thd.pass();
        }

        FreeplayMenu.InternalWaitTransition(state, "transition-out", dt_screenout);
        FreeplayMenu.InternalDropCustomBackground(state);

        modding.HelperNotifyExit2();

        //free(default_bf);
        //free(default_gf);
        //free(state.difficulties);
        mutex.Destroy(state.mutex);
        InternalDropSoundplayer(state, false);
        songicons.Destroy();
        menu_songs.Destroy();
        songs.Destroy(false);
        layout.Destroy();
        modding.Destroy();

        SaveManager.CheckAndSaveChanges();

        PVRContext.MuteAudioOutputOnMinimized(EngineSettings.mute_on_minimize);

        if (GameMain.background_menu_music != null) {
            GameMain.background_menu_music.Play();
        }

    }

    private static int Show(Menu menu, State state, ArrayList<MappedSong> songs) {
        int map_index = -1;
        Gamepad gamepad = new Gamepad(-1);
        gamepad.SetButtonsDelay(170);

        SoundPlayer sound_asterik = SoundPlayer.Init("/assets/common/sound/asterikMenu.ogg");

        if (menu.GetSelectedIndex() < 0 && menu.GetItemsCount() > 0) {
            menu.SelectIndex(0);

            state.map = songs.Get(0);
            state.use_alternative = false;
            FreeplayMenu.InternalBuildDifficulties(state);
            FreeplayMenu.InternalShowInfo(state);
            FreeplayMenu.InternalSongLoad(state, true);
            InternalTriggerActionMenu(state, true, false);
            FreeplayMenu.InternalModdingNotifyEvent(state, true, true);
        }


        while (!state.modding.has_exit) {
            float elapsed = PVRContext.global_context.WaitReady();
            PVRContext.global_context.Reset();

            if (state.running_threads < 1 && state.modding.script != null && state.soundplayer != null) {
                state.modding.script.NotifyTimerSong(state.soundplayer.GetPosition());
            }
            if (state.modding.HelperHandleCustomMenu(gamepad, elapsed) != ModdingHelperResult.CONTINUE) {
                break;
            }

            state.layout.Animate(elapsed);
            state.layout.Draw(PVRContext.global_context);

            if (state.modding.has_halt) continue;

            GamepadButtons btns = gamepad.HasPressedDelayed(
                GamepadButtons.DPAD_UP | GamepadButtons.DPAD_DOWN |
                GamepadButtons.DPAD_LEFT | GamepadButtons.DPAD_RIGHT |
                GamepadButtons.X | GamepadButtons.A | GamepadButtons.B |
                GamepadButtons.BACK | GamepadButtons.START
            );

            int offset;
            bool switch_difficult;

            if ((btns & (GamepadButtons.B | GamepadButtons.BACK)).Bool() && !state.modding.HelperNotifyBack()) {
                break;
            } else if ((btns & (GamepadButtons.X)).Bool()) {
                WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
                if (StringUtils.IsNotEmpty(weekinfo.warning_message)) {
                    state.use_alternative = !state.use_alternative;
                    FreeplayMenu.InternalModdingNotifyEvent(state, false, true);
                } else if (sound_asterik != null) {
                    sound_asterik.Replay();
                }
                continue;
            } else if ((btns & GamepadButtons.DPAD_UP).Bool()) {
                offset = -1;
                switch_difficult = false;
            } else if ((btns & GamepadButtons.DPAD_DOWN).Bool()) {
                offset = 1;
                switch_difficult = false;
            } else if ((btns & GamepadButtons.DPAD_LEFT).Bool()) {
                if (state.map.is_locked) continue;
                offset = -1;
                switch_difficult = true;
            } else if ((btns & GamepadButtons.DPAD_RIGHT).Bool()) {
                if (state.map.is_locked) continue;
                offset = 1;
                switch_difficult = true;
            } else if ((btns & (GamepadButtons.A | GamepadButtons.START)).Bool()) {
                int index = menu.GetSelectedIndex();
                if (index < 0 || index >= menu.GetItemsCount() || state.difficult_locked || state.map.is_locked) {
                    if (sound_asterik != null) sound_asterik.Replay();
                    continue;
                }
                if (FreeplayMenu.InternalModdingNotifyOption(state, false)) continue;

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
                    FreeplayMenu.InternalModdingNotifyEvent(state, true, false);
                }
                continue;
            }

            int old_index = menu.GetSelectedIndex();
            if (!menu.SelectVertical(offset)) {
                int index = menu.GetSelectedIndex();
                if (index < 1)
                    index = menu.GetItemsCount() - 1;
                else
                    index = 0;

                menu.SelectIndex(index);
            }

            int selected_index = menu.GetSelectedIndex();
            if (selected_index != old_index) InternalTriggerActionMenu(state, false, false);

            state.map = songs.Get(selected_index);
            state.use_alternative = false;
            FreeplayMenu.InternalBuildDifficulties(state);
            FreeplayMenu.InternalShowInfo(state);
            FreeplayMenu.InternalSongLoad(state, true);

            if (selected_index != old_index) {
                InternalTriggerActionMenu(state, true, false);
                FreeplayMenu.InternalModdingNotifyEvent(state, true, true);
            }
        }

        if (sound_asterik != null) sound_asterik.Destroy();
        gamepad.Destroy();

        if (map_index >= 0) InternalTriggerActionMenu(state, false, true);

        return map_index;
    }

    private static object InternalLoadSoundPlayerAsync(State state) {
        int async_id_operation = state.async_id_operation;

        // adquire mutex and adquire shared variables
        mutex.Lock(state.mutex);

        MappedSong map = state.map;
        WeekInfo weekinfo = Funkin.weeks_array.array[map.week_index];
        WeekInfo.Song songinfo = weekinfo.songs[map.song_index];
        float seek = songinfo.freeplay_seek_time * 1000f;

        if (map.is_locked || songinfo.name == null) {
            // nothing to load
            InternalDropSoundplayer(state, false);
            mutex.Unlock(state.mutex);
            Interlocked.Decrement(ref state.running_threads);
            return null;
        }

        // shared variables adquired, release mutex
        mutex.Unlock(state.mutex);

        string song_path = FreeplayMenu.InternalGetSongPath(
            weekinfo, state.use_alternative, map.song_index
        );

        if (async_id_operation != state.async_id_operation) {
            // another song was selected
            //free(song_path);
            Interlocked.Decrement(ref state.running_threads);
            return null;
        }

        if (song_path == null) {
            // song not available
            mutex.Lock(state.mutex);
            InternalDropSoundplayer(state, false);
            mutex.Unlock(state.mutex);
            Interlocked.Decrement(ref state.running_threads);
            return null;
        }

        if (song_path == state.soundplayer_path) {
            // the same song is used, seek and fade previous song
            if (state.soundplayer != null) {
                mutex.Lock(state.mutex);
                if (!Single.IsNaN(seek)) state.soundplayer.Seek(seek);
                state.soundplayer.Play();
                state.soundplayer.Fade(true, 500.0f);
                mutex.Unlock(state.mutex);
            }

            //free(song_path);
            Interlocked.Decrement(ref state.running_threads);
            return null;
        }

        // instance a new soundplayer
        SoundPlayer soundplayer = SoundPlayer.Init(song_path);

        if (async_id_operation == state.async_id_operation) {
            // swap soundplayer
            mutex.Lock(state.mutex);
            InternalDropSoundplayer(state, false);

            if (soundplayer != null) {
                state.soundplayer_path = song_path;
                state.soundplayer = soundplayer;

                soundplayer.SetVolume(state.song_preview_volume);
                if (!Single.IsNaN(seek)) soundplayer.Seek(seek);
                soundplayer.Play();
                soundplayer.Fade(true, 500.0f);
            } else {
                //free(song_path);
            }
            mutex.Unlock(state.mutex);
        } else {
            // another song was selected
            if (soundplayer != null) soundplayer.Destroy();
            //free(song_path);
        }

        Interlocked.Decrement(ref state.running_threads);
        return null;
    }

    private static object InternalLoadBackgroundAsync(State state) {
        int async_id_operation = state.async_id_operation;

        if (state.background == null) return null;

        // adquire mutex and get shared variables
        mutex.Lock(state.mutex);

        WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
        string src = weekinfo.songs[state.map.song_index].freeplay_background;
        ModelHolder modelholder = null;
        Texture texture = null;

        // check if the selected song has a custom background
        if (StringUtils.IsEmpty(src)) {
            FreeplayMenu.InternalDropCustomBackground(state);
            Interlocked.Decrement(ref state.running_threads);
            mutex.Unlock(state.mutex);
            return null;
        }

        if (ModelHolder.UtilsIsKnownExtension(src)) {
            modelholder = ModelHolder.Init(src);
        } else {
            // assume is a image file
            texture = Texture.Init(src);
        }

        mutex.Lock(state.mutex);

        // if the user has no changed the song, set the background
        if (async_id_operation == state.async_id_operation) {
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
                state.layout.TriggerAny("song-background-hide");
            else
                state.layout.TriggerAny("song-background-set");
        }

        if (modelholder != null) modelholder.Destroy();
        if (texture != null) texture.Destroy();

        Interlocked.Decrement(ref state.running_threads);
        mutex.Unlock(state.mutex);
        return null;
    }

    private static void InternalShowInfo(State state) {
        Layout layout = state.layout;
        WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
        string desc = weekinfo.songs[state.map.song_index].freeplay_description;
        string week_name = weekinfo.display_name;
        string song_name = weekinfo.songs[state.map.song_index].name;

        if (weekinfo.display_name == null) week_name = weekinfo.name;

        string difficult;
        bool is_locked;
        float bg_info_width = state.bg_info_width;
        float text_width;

        long score;
        if (state.difficult_index >= 0 && state.difficult_index < state.difficulties_size) {
            difficult = state.difficulties[state.difficult_index].name;
            score = FunkinSave.GetFreeplayScore(weekinfo.name, difficult, song_name);
            is_locked = state.difficulties[state.difficult_index].is_locked;
        } else {
            score = 0;
            is_locked = true;
            difficult = null;
        }

        if (state.map.is_locked) is_locked = true;

        if (state.personal_best != null) {
            state.personal_best.SetTextFormated(FreeplayMenu.PERSONAL_BEST, score);
            state.personal_best.GetDrawSize(out text_width, out _);
            if (text_width > bg_info_width) bg_info_width = text_width * 1.1f;
        }

        if (state.info != null) {
            state.info.SetTextFormated(FreeplayMenu.INFO, week_name, song_name, difficult);
            state.info.GetDrawSize(out text_width, out _);
            if (text_width > bg_info_width) bg_info_width = text_width * 1.1f;
        }

        Sprite bg_info = layout.GetSprite(FreeplayMenu.BG_INFO_NAME);
        if (bg_info != null) bg_info.SetDrawSize(bg_info_width, Single.NaN);

        if (StringUtils.IsNotEmpty(desc)) {
            if (state.description != null)
                state.description.SetTextIntern(true, desc);
            layout.TriggerAny("description-show");
        } else {
            layout.TriggerAny("description-hide");
        }

        layout.TriggerAny(is_locked ? "locked" : "not-locked");

        if (weekinfo.warning_message != null)
            layout.TriggerAny(state.use_alternative ? "use-alternative" : "not-use-alternative");
        else
            layout.TriggerAny("hide-alternative");
    }

    private static void InternalDropSoundplayer(State state, bool wait_for_background_threads) {
        // wait for running threads
        while (wait_for_background_threads) {
            thd.pass();
            mutex.Lock(state.mutex);
            bool exit = state.running_threads < 1;
            mutex.Unlock(state.mutex);
            if (exit) break;
        }

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

        state.difficulties = EngineUtils.CreateArray<Difficult>(count);
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

    public static string HelperGetDefaultCharacterManifest(bool is_boyfriend) {
        string src = is_boyfriend ? WeekSelectorMdlSelect.MODELS_BF : WeekSelectorMdlSelect.MODELS_GF;
        JSONToken json = JSONParser.LoadFrom(src);

        if (json == null) return null;

        int array_length = JSONParser.ReadArrayLength(json);
        string manifest_src = null;

        for (int i = 0 ; i < array_length ; i++) {
            JSONToken item = JSONParser.ReadArrayItemObject(json, i);
            if (item == null) continue;

            string unlock_directive = JSONParser.ReadString(item, "unlockDirectiveName", null);
            if (!FunkinSave.ContainsUnlockDirective(unlock_directive)) continue;

            string model = JSONParser.ReadString(item, "manifest", null);
            if (StringUtils.IsEmpty(model)) continue;

            manifest_src = FS.BuildPath2(src, model);
            break;
        }

        JSONParser.Destroy(json);
        return manifest_src;
    }

    private static void InternalSongLoad(State state, bool with_bg) {
        mutex.Lock(state.mutex);

        Interlocked.Increment(ref state.async_id_operation);

        if (with_bg) {
            Interlocked.Increment(ref state.running_threads);
            GameMain.THDHelperSpawn(true, FreeplayMenu.InternalLoadBackgroundAsync, state);
        }

        Interlocked.Increment(ref state.running_threads);
        GameMain.THDHelperSpawn(true, FreeplayMenu.InternalLoadSoundPlayerAsync, state);

        mutex.Unlock(state.mutex);
    }

    private static void InternalWaitTransition(State state, string what, float duration) {
        Layout layout = state.layout;
        Modding modding = state.modding;

        state.modding.HelperNotifyEvent(what);

        if (duration < 1f) return;
        if (layout.TriggerAny(what) < 1) return;

        while (duration > 0f) {
            float elapsed = PVRContext.global_context.WaitReady();
            duration -= elapsed;

            modding.HelperNotifyFrame(elapsed, -1f);

            layout.Animate(elapsed);
            layout.Draw(PVRContext.global_context);
        }
    }

    private static void InternalTriggerActionMenu(State state, bool selected, bool choosen) {
        if (state.map.week_index < 0 || state.map.week_index >= Funkin.weeks_array.size) return;
        WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];

        if (state.map.song_index < 0 || state.map.song_index >= weekinfo.songs_count) return;
        WeekInfo.Song[] songs = weekinfo.songs;

        string week_name = weekinfo.display_name ?? weekinfo.name;
        string song_name = songs[state.map.song_index].name;

        if (selected) FreeplayMenu.InternalModdingNotifyOption(state, true);

        GameMain.HelperTriggerActionMenu(state.layout, week_name, song_name, selected, choosen);
    }

    private static bool InternalModdingNotifyOption(State state, bool selected_or_choosen) {
        WeekInfo weekinfo = Funkin.weeks_array.array[state.map.week_index];
        WeekInfo.Song[] songs = weekinfo.songs;
        Menu menu = state.modding.native_menu;

        string week_name = weekinfo.display_name ?? weekinfo.name;
        string song_name = songs[state.map.song_index].name;
        string name = StringUtils.Concat(week_name, "\n", weekinfo.display_name, "\n", song_name);
        int index = menu.GetSelectedIndex();

        bool ret = state.modding.HelperNotifyOption2(selected_or_choosen, menu, index, name);
        //free(name);

        return ret;
    }

    private static void InternalModdingNotifyEvent(State state, bool difficult, bool alt_tracks) {
        if (difficult && alt_tracks) {
            state.modding.HelperNotifyEvent(state.map.is_locked ? "song-locked" : "song-not-locked");
        }
        if (difficult) {
            state.modding.HelperNotifyEvent(state.difficult_locked ? "difficult-locked" : "difficult-not-locked");

            if (state.difficult_index >= 0 && state.difficult_index < state.difficulties_size)
                state.modding.HelperNotifyEvent(state.difficulties[state.difficult_index].name);
            else
                state.modding.HelperNotifyEvent(null);
        }
        if (alt_tracks) {
            state.modding.HelperNotifyEvent(state.use_alternative ? "tracks-alt" : "tracks-not-alt");
        }
    }

    private static string InternalGetSongPath(WeekInfo weekinfo, bool use_alternative, int mapped_song_index) {
        WeekInfo.Song songinfo = weekinfo.songs[mapped_song_index];

        // guess the song filename or voice/instrumental part
        string tmp_song_path;

        if (StringUtils.IsNotEmpty(songinfo.freeplay_song_filename)) {
            tmp_song_path = songinfo.freeplay_song_filename;//strdup(songinfo.freeplay_song_filename);
        } else {
            string temp_nospaces = songinfo.name.Replace('\x20', '-');
            string temp_lowercase = StringUtils.ToLowerCase(temp_nospaces);
            //free(temp_nospaces);
            string temp_relativepath = StringUtils.Concat(Funkin.WEEK_SONGS_FOLDER, temp_lowercase, ".ogg");
            //free(temp_lowercase);
            tmp_song_path = WeekEnumerator.GetAsset(weekinfo, temp_relativepath);
            //free(temp_relativepath);
        }

        // get voice and instrumental tracks
        string path_voices, path_instrumental;
        bool is_not_splitted = SongPlayer.HelperGetTracks(
            tmp_song_path, use_alternative, out path_voices, out path_instrumental
        );

        // get the path of instrumetal track of the song (if applicable)
        if (path_instrumental != null) {
            //free(tmp_song_path);
            //free(path_voices);
            return path_instrumental;
        } else if (path_voices != null) {
            //free(tmp_song_path);
            //free(path_instrumental);
            return path_voices;
        } else if (is_not_splitted) {
            return tmp_song_path;
        }

        // missing file
        //free(tmp_song_path);
        return null;
    }



    public struct MappedSong {
        public int song_index;
        public int week_index;
        public bool is_locked;
        public int gameplaymanifest_index;
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

        public volatile int async_id_operation;

        public volatile SoundPlayer soundplayer;
        public volatile string soundplayer_path;

        public mutex_t mutex;
        public volatile int running_threads;

        public volatile Sprite background;
        public Layout layout;

        public TextSprite personal_best;
        public TextSprite info;
        public TextSprite description;

        public Difficult[] difficulties;
        public int difficulties_size;

        public float bg_info_width;
        public Modding modding;
        public float song_preview_volume;
    }

}
