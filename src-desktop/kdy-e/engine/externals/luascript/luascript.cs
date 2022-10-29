using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;
using System;
using System.Diagnostics;

namespace Engine.Externals.LuaScriptInterop {
    public class Luascript {

        private readonly ManagedLuaState lua;
        private readonly LuaState L;
        private Luascript(ManagedLuaState lua) {
            this.lua = lua;
            this.L = lua.LuaStateHandle;
        }


        private static void RegisterObjects(ManagedLuaState lua, bool is_week) {
            lua.RegisterConstantString("ENGINE_NAME", GameMain.ENGINE_NAME);
            lua.RegisterConstantString("ENGINE_VERSION", GameMain.ENGINE_VERSION);

            lua.RegisterConstantInteger("GAMEPAD_A", (uint)GamepadButtons.A);
            lua.RegisterConstantInteger("GAMEPAD_B", (uint)GamepadButtons.B);
            lua.RegisterConstantInteger("GAMEPAD_X", (uint)GamepadButtons.X);
            lua.RegisterConstantInteger("GAMEPAD_Y", (uint)GamepadButtons.Y);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_UP", (uint)GamepadButtons.DPAD_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_DOWN", (uint)GamepadButtons.DPAD_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_RIGHT", (uint)GamepadButtons.DPAD_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_LEFT", (uint)GamepadButtons.DPAD_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_START", (uint)GamepadButtons.START);
            lua.RegisterConstantInteger("GAMEPAD_SELECT", (uint)GamepadButtons.SELECT);
            lua.RegisterConstantInteger("GAMEPAD_TRIGGER_LEFT", (uint)GamepadButtons.TRIGGER_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_TRIGGER_RIGHT", (uint)GamepadButtons.TRIGGER_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_BUMPER_LEFT", (uint)GamepadButtons.BUMPER_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_BUMPER_RIGHT", (uint)GamepadButtons.BUMPER_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_APAD_UP", (uint)GamepadButtons.APAD_UP);
            lua.RegisterConstantInteger("GAMEPAD_APAD_DOWN", (uint)GamepadButtons.APAD_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_APAD_RIGHT", (uint)GamepadButtons.APAD_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_APAD_LEFT", (uint)GamepadButtons.APAD_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_UP", (uint)GamepadButtons.DPAD2_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_DOWN", (uint)GamepadButtons.DPAD2_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_RIGHT", (uint)GamepadButtons.DPAD2_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_LEFT", (uint)GamepadButtons.DPAD2_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_UP", (uint)GamepadButtons.DPAD3_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_DOWN", (uint)GamepadButtons.DPAD3_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_RIGHT", (uint)GamepadButtons.DPAD3_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_LEFT", (uint)GamepadButtons.DPAD3_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_UP", (uint)GamepadButtons.DPAD4_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_DOWN", (uint)GamepadButtons.DPAD4_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_RIGHT", (uint)GamepadButtons.DPAD4_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_LEFT", (uint)GamepadButtons.DPAD4_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_BACK", (uint)GamepadButtons.BACK);


            // register all objects (metatables) and functions
            ExportsCamera.register_camera(lua);
            ExportsCharacter.register_character(lua);
            ExportsLayout.register_layout(lua);
            ExportsMessageBox.register_messagebox(lua);
            ExportsModifier.register_modifier(lua);
            ExportsSoundPlayer.register_soundplayer(lua);
            ExportsSprite.register_sprite(lua);
            ExportsTextSprite.register_textsprite(lua);

            if (is_week) {
                ExportsSongPlayer.register_songplayer(lua);
                ExportsWeek.register_week(lua);
            } else {
                ExportsModding.register_modding(lua);
            }

            ExportsMath2D.register_math2d(lua);
            ExportsTimer.register_timer(lua);
            ExportsFS.register_fs(lua);
        }

        private static void RegisterSandbox(ManagedLuaState lua) {
            const string SANDBOX =
                "debug.debug = nil\n" +
                "debug.getfenv = getfenv\n" +
                "debug.getregistry = nil\n" +
                "debug = nil\n" +
                "dofile = nil\n" +
                "io = nil\n" +
                "load = nil\n" +
                "loadfile = nil\n" +
                "dofile = nil\n" +
                "os.execute = nil\n" +
                "os.getenv = nil\n" +
                "os.remove = nil\n" +
                "os.tmpname = nil\n" +
                "os.setlocale = nil\n" +
                "os.rename = nil\n" +
                //"os.exit = nil\n" +
                //"loadstring = nil\n" +
                "package.loaded.io = nil\n" +
                "package.loaded.package = nil\n" +
                "package.cpath = nil\n" +
                "package.loaded = nil\n" +
                "package.loaders= nil\n" +
                "package.loadlib= nil\n" +
                "package.path= nil\n" +
                "package.preload= nil\n" +
                "package.seeall= nil\n" +
                "package.searchpath= nil\n" +
                "package.searchers= nil\n" +
                "package = nil\n" +
                "require = nil\n" +
                "newproxy = nil\n";


            int result = lua.EvaluateString(SANDBOX);
            Debug.Assert(result == 0, "luascript_register_sandbox() failed");
        }

        public static Luascript Init(string lua_sourcecode, string filename, object context, bool is_week) {
            ManagedLuaState lua = ManagedLuaState.Init(context);

            if (lua == null) {
                Console.Error.WriteLine("[ERROR] luascript_init() cannot create lua state, not enough memory");
                return null;
            }

            RegisterObjects(lua, is_week);
            RegisterSandbox(lua);

            int status = lua.EvaluateString(lua_sourcecode, filename);

            if (status != ManagedLuaState.LUA_OK) {
                string error_message = lua.LuaStateHandle.lua_tostring(-1);
                Console.Error.WriteLine($"luascript_init() luaL_loadfile() failed: {error_message}");

                lua.Dispose();
                return null;
            }

            return new Luascript(lua);
        }

        public void Destroy() {
            this.lua.Dispose();
        }

        public void DropShared() {
            // In C# this function is not relevant
        }



        public void notify_weekinit(int freeplay_track_index) {
            const string FUNCTION = "f_weekinit";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(freeplay_track_index);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_beforeready(bool from_retry) {
            const string FUNCTION = "f_beforeready";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(from_retry);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_ready() {
            const string FUNCTION = "f_ready";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_aftercountdown() {
            const string FUNCTION = "f_aftercountdown";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_frame(float elapsed) {
            const string FUNCTION = "f_frame";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(elapsed);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_roundend(bool loose) {
            const string FUNCTION = "f_roundend";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(loose);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_weekend(bool giveup) {
            const string FUNCTION = "f_weekend";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(giveup);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_diedecision(bool retry_or_giveup) {
            const string FUNCTION = "f_diedecision";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(retry_or_giveup);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_pause(bool pause_or_resume) {
            const string FUNCTION = "f_pause";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(pause_or_resume);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_weekleave() {
            const string FUNCTION = "f_weekleave";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_afterresults() {
            const string FUNCTION = "f_afterresults";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_scriptchange() {
            const string FUNCTION = "f_scriptchange";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_pause_optionselected(int option_index, uint buttons) {
            const string FUNCTION = "f_pause_optionselected";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(option_index);
            L.lua_pushinteger(buttons);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_pause_menuvisible(bool shown_or_hidden) {
            const string FUNCTION = "f_pause_menuvisible";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(shown_or_hidden);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_note(double timestamp, int id, double duration, double data, bool special, int player_id, uint state) {
            const string FUNCTION = "f_note";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(timestamp);
            L.lua_pushinteger(id);
            L.lua_pushnumber(duration);
            L.lua_pushnumber(data);
            L.lua_pushboolean(special);
            L.lua_pushinteger(player_id);
            L.lua_pushinteger(state);

            lua.CallPushedGlobalFunction(7);
        }

        public void notify_buttons(int player_id, uint buttons) {
            const string FUNCTION = "f_buttons";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(player_id);
            L.lua_pushinteger(buttons);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_unknownnote(int player_id, double timestamp, int id, double duration, double data) {
            const string FUNCTION = "f_unknownnote";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(player_id);
            L.lua_pushnumber(timestamp);
            L.lua_pushinteger(id);
            L.lua_pushnumber(duration);
            L.lua_pushnumber(data);

            lua.CallPushedGlobalFunction(5);
        }

        public void notify_beat(int current_beat, float since) {
            const string FUNCTION = "f_beat";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(current_beat);
            L.lua_pushnumber(since);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_quarter(int current_quarter, float since) {
            const string FUNCTION = "f_quarter";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(current_quarter);
            L.lua_pushnumber(since);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_timer_run(double timestamp) {
            const string FUNCTION = "______kdmyEngine_timer_run";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(timestamp);

            lua.CallPushedGlobalFunction(1);
        }


        ////
        //// Unimplemented and/or exported functions for lua
        ////
        /*

        // unimplemented
        modelholder_init(src);
        modelholder_is_invalid(modelholder);
        modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null);// internal alloc
        modelholder_get_atlas_entry(modelholder, atlas_entry_name);// managed
        animlist_init(src);
        animlist_create_animsprite(animlist, animation_name);// managed
        atlas_init(src);
        atlas_get_entry(atlas, entry_name);
        atlas_apply(atlas, sprite, name, override_draw_size);


        math2d_random(min, max);
        math2d_random_int(min, max);
        math2d_lerp(start, end, step);
        math2d_inverselerp(start, end, value);
        math2d_nearestdown(value, step);
        math2d_cubicbezier(offset, point0, point1, point2, point3);
        math2d_points_distance(x1, y1, x2, y2);


        healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
        healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rbb8, state_name);
        healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
        healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name);
        healthbar_state_background_add(healthbar, modelholder, state_name);
        healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name);
        healthbar_set_opponent_bar_color(healthbar, color_rgb8);
        healthbar_set_player_bar_color(healthbar, color_rgb8);
        healthbar_state_toggle(healthbar, state_name);
        healthbar_state_toggle_background(healthbar, state_name);
        healthbar_state_toggle_player(healthbar, state_name);
        healthbar_state_toggle_opponent(healthbar, state_name);

        healthbar_set_bump_animation(healthbar, animlist);
        healthbar_set_bump_animation_opponent(healthbar, animsprite);
        healthbar_set_bump_animation_player(healthbar, animsprite);
        healthbar_bump_enable(healthbar, enable_bump);
        healthbar_set_visible(healthbar, visible);
        healthbar_set_alpha(healthbar, alpha);
        healthbar_get_modifier(healthbar);
        healthbar_animation_set(healthbar, animsprite);
        healthbar_disable_progress_animation(healthbar, disable);
        healthbar_disable_icon_overlap(healthbar, disable);
        healthbar_hide_warnings(healthbar);
        healthbar_show_drain_warning(healthbar, use_fast_drain);
        healthbar_show_locked_warning(healthbar);


        notepool_change_alpha_alone(notepool, alpha);
        notepool_change_alpha_sustain(notepool, alpha);
        notepool_change_alpha(notepool, alpha);


        playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference);
        playerstats_add_sustain(playerstats, quarters, is_released);
        playerstats_add_penality(playerstats, on_empty_strum);
        playerstats_add_miss(playerstats, multiplier);
        playerstats_add_extra_health(playerstats, multiplier);
        playerstats_enable_health_recover(playerstats, enable);
        playerstats_get_health(playerstats);
        playerstats_get_last_ranking(playerstats);
        playerstats_get_last_difference(playerstats);
        playerstats_get_combo_streak(playerstats);
        playerstats_get_combo_breaks(playerstats);
        playerstats_get_score(playerstats);
        playerstats_get_hits(playerstats);
        playerstats_get_misses(playerstats);
        playerstats_get_penalties(playerstats);
        playerstats_set_health(playerstats, health);
        playerstats_add_health(playerstats, health, die_if_negative);
        playerstats_raise(playerstats, with_full_health);


        rankingcounter_add_state(rankingcounter, modelholder, state_name);
        rankingcounter_toggle_state(rankingcounter, state_name);
        rankingcounter_reset(rankingcounter);
        rankingcounter_hide_accuracy(rankingcounter, hide);
        rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter);
        rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);
        rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite);
        rankingcounter_set_alpha(rankingcounter, alpha);
        rankingcounter_set_offsetcolor(rankingcounter, r, g, b, a);
        rankingcounter_set_offsetcolor_to_default(rankingcounter);
        rankingcounter_get_modifier(rankingcounter);


        roundstats_hide(roundstats, hide);
        roundstats_reset(roundstats);
        roundstats_get_drawable(roundstats);
        roundstats_tweenlerp_set_on_beat(roundstats, tweenlerp, rollback_beats, beat_duration);
        roundstats_tweenlerp_set_on_hit(roundstats, tweenlerp, rollback_beats, beat_duration);
        roundstats_tweenlerp_set_on_miss(roundstats, tweenlerp, rollback_beats, beat_duration);


        streakcounter_hide_combo_sprite(streakcounter, hide);
        streakcounter_set_combo_draw_location(streakcounter, x, y);
        streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name);
        streakcounter_state_toggle(streakcounter, state_name);
        streakcounter_set_alpha(streakcounter, alpha);
        streakcounter_set_offsetcolor(streakcounter, r, g, b, a);
        streakcounter_set_offsetcolor_to_default(streakcounter);
        streakcounter_get_modifier(streakcounter);


        strum_set_scroll_speed(strum, speed);
        strum_set_scroll_direction(strum, direction);
        strum_reset(strum, scroll_speed, state_name);
        strum_force_key_release(strum);
        strum_get_press_state(strum);
        strum_get_name(strum);
        strum_get_marker_duration(strum);
        strum_set_marker_duration(strum, duration);
        strum_enable_background(strum, enable);
        strum_enable_sick_effect(strum, enable);
        strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
        strum_state_toggle(strum, state_name);
        strum_state_toggle_notes(strum, state_name);
        strum_state_toggle_sick_effect(strum, state_name);
        strum_state_toggle_marker(strum, state_name);
        strum_state_toggle_background(strum, state_name);
        strum_set_alpha_background(strum, alpha);
        strum_set_alpha_sick_effect(strum, alpha);
        strum_set_keep_aspect_ratio_background(strum, enable);
        strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite);// managed
        strum_set_extra_animation_continuous(strum, strum_script_target, animsprite);// managed
        strum_set_notesmaker_tweenlerp(strum, tweenlerp, apply_to_marker_too);// not implemented
        strum_set_sickeffect_size_ratio(strum, size_ratio);
        strum_set_offsetcolor(strum, r, g, b, a);
        strum_set_alpha(strum, alpha);
        strum_set_visible(strum, visible);
        strum_set_draw_offset(strum, offset_milliseconds);
        strum_get_modifier(strum);
        strum_animation_restart(strum);
        strum_animation_end(strum);



        conductor_poll_reset(conductor);
        conductor_set_character(conductor, character);
        conductor_use_strum_line(conductor, strum);
        conductor_use_strums(conductor, strums);
        conductor_disable_strum_line(conductor, strum, should_disable);
        conductor_remove_strum(conductor, strum);
        conductor_clear_mapping(conductor);
        conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name);
        conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name);
        conductor_map_strum_to_player_sing_remove(conductor, strum_name, sing_direction_name);
        conductor_map_strum_to_player_extra_remove(conductor, strum_name, extra_animation_name);
        conductor_map_automatically(conductor, should_map_extras);
        conductor_disable(conductor, disable);
        conductor_play_idle(conductor);
        conductor_play_hey(conductor);
        conductor_get_character(conductor);


        countdown_set_bpm(countdown, bpm);
        countdown_ready(countdown);
        countdown_start(countdown);
        countdown_has_ended(countdown);


        imgutils_calc_rectangle(x, y, max_width, max_height, src_width, src_height, align_hrzntl, align_vrtcl);// managed + multiple result
        imgutils_calc_centered_location(x, y, width, height, ref_width, ref_height);



        drawable_set_antialiasing(drawable, antialiasing);
        statesprite_set_antialiasing(drawable, antialiasing);

        // engine_feature******
        healthbar_enable_extra_length(healthbar, extra_enabled);
        healthbar_disable_warnings(healthbar, disable);
        playerstats_enable_penality_on_empty_strum(playerstats, enable);

        */

    }

}