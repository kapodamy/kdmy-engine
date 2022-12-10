using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using static Engine.Game.Strum;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsStrum {
        public const string STRUM = "Strum";


        static int script_strum_update_draw_location(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);

            strum.UpdateDrawLocation(x, y);

            return 0;
        }

        static int script_strum_set_scroll_speed(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float speed = (float)L.luaL_checknumber(2);

            strum.SetScrollSpeed(speed);

            return 0;
        }

        static int script_strum_set_scroll_direction(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            ScrollDirection direction = LuascriptHelpers.luascript_parse_scrolldirection(L, L.luaL_optstring(2, null));

            strum.SetScrollDirection(direction);

            return 0;
        }

        static int script_strum_set_marker_duration_multiplier(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float multipler = (float)L.luaL_checknumber(2);

            strum.SetMarkerDurationMultiplier(multipler);

            return 0;
        }

        static int script_strum_reset(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float scroll_speed = (float)L.luaL_checknumber(2);
            string state_name = L.luaL_optstring(3, null);

            strum.Reset(scroll_speed, state_name);

            return 0;
        }

        static int script_strum_force_key_release(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            strum.ForceKeyRelease();

            return 0;
        }

        static int script_strum_get_press_state_changes(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            int ret = strum.GetPressStateChanges();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strum_get_press_state(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            StrumPressState ret = strum.GetPressState();

            L.lua_pushinteger((int)ret);
            return 1;
        }

        static int script_strum_get_name(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            string ret = strum.GetName();

            L.lua_pushstring(ret);
            return 1;
        }

        static int script_strum_get_marker_duration(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            float ret = strum.GetMarkerDuration();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_strum_set_player_id(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            int player_id = (int)L.luaL_checkinteger(2);

            strum.SetPlayerID(player_id);

            return 0;
        }

        static int script_strum_enable_background(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            bool enable = L.lua_toboolean(2);

            strum.EnableBackground(enable);

            return 0;
        }

        static int script_strum_enable_sick_effect(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            bool enable = L.lua_toboolean(2);

            strum.EnableSickEffect(enable);

            return 0;
        }

        static int script_strum_state_add(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            ModelHolder mdlhldr_mrkr = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            ModelHolder mdlhldr_sck_ffct = L.ReadNullableUserdata<ModelHolder>(3, ExportsModelHolder.MODELHOLDER);
            ModelHolder mdlhldr_bckgrnd = L.ReadNullableUserdata<ModelHolder>(4, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(5, null);

            strum.StateAdd(mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

            return 0;
        }

        static int script_strum_state_toggle(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            string state_name = L.luaL_optstring(2, null);

            int ret = strum.StateToggle(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strum_state_toggle_notes(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            string state_name = L.luaL_optstring(2, null);

            int ret = strum.StateToggleNotes(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strum_state_toggle_sick_effect(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            string state_name = L.luaL_optstring(2, null);

            bool ret = strum.StateToggleSickEffect(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_strum_state_toggle_marker(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            string state_name = L.luaL_optstring(2, null);

            int ret = strum.StateToggleMarker(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strum_state_toggle_background(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            string state_name = L.luaL_optstring(2, null);

            bool ret = strum.StateToggleBackground(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_strum_set_alpha_background(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float alpha = (float)L.luaL_checknumber(2);

            float ret = strum.SetAlphaBackground(alpha);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_strum_set_alpha_sick_effect(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float alpha = (float)L.luaL_checknumber(2);

            float ret = strum.SetAlphaSickEffect(alpha);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_strum_set_keep_aspect_ratio_background(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            bool enable = L.lua_toboolean(2);

            strum.SetKeepAspectRatioBackground(enable);

            return 0;
        }

        static int script_strum_draw_sick_effect_apart(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            bool enable = L.lua_toboolean(2);

            strum.DrawSickEffectApart(enable);

            return 0;
        }

        static int script_strum_set_extra_animation(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            StrumScriptTarget strum_script_target = LuascriptHelpers.luascript_parse_strumscripttarget(L, L.luaL_optstring(2, null));
            StrumScriptOn strum_script_on = LuascriptHelpers.luascript_parse_strumscripton(L, L.luaL_optstring(3, null));
            bool undo = L.lua_toboolean(4);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(5, ExportsAnimSprite.ANIMSPRITE);

            strum.SetExtraAnimation(strum_script_target, strum_script_on, undo, animsprite);

            return 0;
        }

        static int script_strum_set_extra_animation_continuous(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            StrumScriptTarget strum_script_target = LuascriptHelpers.luascript_parse_strumscripttarget(L, L.luaL_optstring(2, null));
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(3, ExportsAnimSprite.ANIMSPRITE);

            strum.SetExtraAnimationContinuous(strum_script_target, animsprite);

            return 0;
        }

        static int script_strum_set_notesmaker_tweenlerp(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            TweenLerp tweenlerp = L.ReadNullableUserdata<TweenLerp>(2, ExportsTweenLerp.TWEENLERP);
            bool apply_to_marker_too = L.lua_toboolean(3);

            strum.SetNotesmakerTweenlerp(tweenlerp, apply_to_marker_too);

            return 0;
        }

        static int script_strum_set_sickeffect_size_ratio(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float size_ratio = (float)L.luaL_checknumber(2);

            strum.SetSickeffectSizeRatio(size_ratio);

            return 0;
        }

        static int script_strum_set_alpha(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float alpha = (float)L.luaL_checknumber(2);

            strum.SetAlpha(alpha);

            return 0;
        }

        static int script_strum_set_visible(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            bool visible = L.lua_toboolean(2);

            strum.SetVisible(visible);

            return 0;
        }

        static int script_strum_set_draw_offset(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);
            float offset_milliseconds = (float)L.luaL_checknumber(2);

            strum.SetDrawOffset(offset_milliseconds);

            return 0;
        }

        static int script_strum_get_modifier(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            Modifier ret = strum.GetModifier();

            return ExportsModifier.script_modifier_new(L, ret);
        }

        static int script_strum_get_duration(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            double ret = strum.GetDuration();

            L.lua_pushnumber(ret);
            return 0;
        }

        static int script_strum_animation_restart(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            strum.AnimationRestart();

            return 0;
        }

        static int script_strum_animation_end(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            strum.AnimationEnd();

            return 0;
        }

        static int script_strum_get_drawable(LuaState L) {
            Strum strum = L.ReadUserdata<Strum>(STRUM);

            Drawable ret = strum.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }




        static readonly LuaTableFunction[] STRUM_FUNCTIONS = {
            new LuaTableFunction("update_draw_location", script_strum_update_draw_location),
            new LuaTableFunction("set_scroll_speed", script_strum_set_scroll_speed),
            new LuaTableFunction("set_scroll_direction", script_strum_set_scroll_direction),
            new LuaTableFunction("set_marker_duration_multiplier", script_strum_set_marker_duration_multiplier),
            new LuaTableFunction("reset", script_strum_reset),
            new LuaTableFunction("force_key_release", script_strum_force_key_release),
            new LuaTableFunction("get_press_state_changes", script_strum_get_press_state_changes),
            new LuaTableFunction("get_press_state", script_strum_get_press_state),
            new LuaTableFunction("get_name", script_strum_get_name),
            new LuaTableFunction("get_marker_duration", script_strum_get_marker_duration),
            new LuaTableFunction("set_player_id", script_strum_set_player_id),
            new LuaTableFunction("enable_background", script_strum_enable_background),
            new LuaTableFunction("enable_sick_effect", script_strum_enable_sick_effect),
            new LuaTableFunction("state_add", script_strum_state_add),
            new LuaTableFunction("state_toggle", script_strum_state_toggle),
            new LuaTableFunction("state_toggle_notes", script_strum_state_toggle_notes),
            new LuaTableFunction("state_toggle_sick_effect", script_strum_state_toggle_sick_effect),
            new LuaTableFunction("state_toggle_marker", script_strum_state_toggle_marker),
            new LuaTableFunction("state_toggle_background", script_strum_state_toggle_background),
            new LuaTableFunction("set_alpha_background", script_strum_set_alpha_background),
            new LuaTableFunction("set_alpha_sick_effect", script_strum_set_alpha_sick_effect),
            new LuaTableFunction("set_keep_aspect_ratio_background", script_strum_set_keep_aspect_ratio_background),
            new LuaTableFunction("draw_sick_effect_apart", script_strum_draw_sick_effect_apart),
            new LuaTableFunction("set_extra_animation", script_strum_set_extra_animation),
            new LuaTableFunction("set_extra_animation_continuous", script_strum_set_extra_animation_continuous),
            new LuaTableFunction("set_notesmaker_tweenlerp", script_strum_set_notesmaker_tweenlerp),
            new LuaTableFunction("set_sickeffect_size_ratio", script_strum_set_sickeffect_size_ratio),
            new LuaTableFunction("set_alpha", script_strum_set_alpha),
            new LuaTableFunction("set_visible", script_strum_set_visible),
            new LuaTableFunction("set_draw_offset", script_strum_set_draw_offset),
            new LuaTableFunction("get_modifier", script_strum_get_modifier),
            new LuaTableFunction("get_duration", script_strum_get_duration),
            new LuaTableFunction("animation_restart", script_strum_animation_restart),
            new LuaTableFunction("animation_end", script_strum_animation_end),
            new LuaTableFunction("get_drawable", script_strum_get_drawable),
            new LuaTableFunction(null, null)
        };

        public static int script_strum_new(LuaState L, Strum strum) {
            return L.CreateUserdata(STRUM, strum);
        }

        static int script_strum_gc(LuaState L) {
            return L.GC_userdata(STRUM);
        }

        static int script_strum_tostring(LuaState L) {
            return L.ToString_userdata(STRUM);
        }

        private static readonly LuaCallback delegate_gc = script_strum_gc;
        private static readonly LuaCallback delegate_tostring = script_strum_tostring;

        public static void script_strum_register(ManagedLuaState L) {
            L.RegisterMetaTable(STRUM, delegate_gc, delegate_tostring, STRUM_FUNCTIONS);
        }


    }
}

