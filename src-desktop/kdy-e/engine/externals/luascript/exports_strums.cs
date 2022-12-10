using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsStrums {
        public const string STRUMS = "Strums";
        static int script_strums_get_drawable(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);

            Drawable ret = strums.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }

        static int script_strums_set_scroll_speed(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float speed = (float)L.luaL_checknumber(2);

            strums.SetScrollSpeed(speed);

            return 0;
        }

        static int script_strums_set_scroll_direction(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            ScrollDirection direction = LuascriptHelpers.luascript_parse_scrolldirection(L, L.luaL_optstring(2, null));

            strums.SetScrollDirection(direction);

            return 0;
        }

        static int script_strums_set_marker_duration_multiplier(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float multipler = (float)L.luaL_checknumber(2);

            strums.SetMarkerDurationMultiplier(multipler);

            return 0;
        }

        static int script_strums_reset(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float scroll_speed = (float)L.luaL_checknumber(2);
            string state_name = L.luaL_optstring(3, null);

            strums.Reset(scroll_speed, state_name);

            return 0;
        }

        static int script_strums_force_key_release(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);

            strums.ForceKeyRelease();

            return 0;
        }

        static int script_strums_set_alpha(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float alpha = (float)L.luaL_checknumber(2);

            float ret = strums.SetAlpha(alpha);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_strums_enable_background(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            bool enable = L.lua_toboolean(2);

            strums.EnableBackground(enable);

            return 0;
        }

        static int script_strums_set_keep_aspect_ratio_background(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            bool enable = L.lua_toboolean(2);

            strums.SetKeepAspectRatioBackground(enable);

            return 0;
        }

        static int script_strums_set_alpha_background(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float alpha = (float)L.luaL_checknumber(2);

            strums.SetAlphaBackground(alpha);

            return 0;
        }

        static int script_strums_set_alpha_sick_effect(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float alpha = (float)L.luaL_checknumber(2);

            strums.SetAlphaSickEffect(alpha);

            return 0;
        }

        static int script_strums_set_draw_offset(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            float offset_milliseconds = (float)L.luaL_checknumber(2);

            strums.SetDrawOffset(offset_milliseconds);

            return 0;
        }

        static int script_strums_state_add(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            ModelHolder mdlhldr_mrkr = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            ModelHolder mdlhldr_sck_ffct = L.ReadNullableUserdata<ModelHolder>(3, ExportsModelHolder.MODELHOLDER);
            ModelHolder mdlhldr_bckgrnd = L.ReadNullableUserdata<ModelHolder>(4, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(5, null);

            strums.StateAdd(mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

            return 0;
        }

        static int script_strums_state_toggle(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            string state_name = L.luaL_optstring(2, null);

            int ret = strums.StateToggle(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strums_state_toggle_notes(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            string state_name = L.luaL_optstring(2, null);

            int ret = strums.StateToggleNotes(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strums_state_toggle_marker_and_sick_effect(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            string state_name = L.luaL_optstring(2, null);

            strums.StateToggleMarkerAndSickEffect(state_name);

            return 0;
        }

        static int script_strums_get_lines_count(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);

            int ret = strums.GetLinesCount();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_strums_get_strum_line(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            int index = (int)L.luaL_checkinteger(2);

            Strum ret = strums.GetStrumLine(index);

            return ExportsStrum.script_strum_new(L, ret);
        }

        static int script_strums_enable_post_sick_effect_draw(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            bool enable = L.lua_toboolean(2);

            strums.EnablePostSickEffectDraw(enable);

            return 0;
        }

        static int script_strums_animation_set(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            strums.AnimationSet(animsprite);

            return 0;
        }

        static int script_strums_animation_restart(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);

            strums.AnimationRestart();

            return 0;
        }

        static int script_strums_animation_end(LuaState L) {
            Strums strums = L.ReadUserdata<Strums>(STRUMS);

            strums.AnimationEnd();

            return 0;
        }




        static readonly LuaTableFunction[] STRUMS_FUNCTIONS = {
    new LuaTableFunction("get_drawable", script_strums_get_drawable),
    new LuaTableFunction("set_scroll_speed", script_strums_set_scroll_speed),
    new LuaTableFunction("set_scroll_direction", script_strums_set_scroll_direction),
    new LuaTableFunction("set_marker_duration_multiplier", script_strums_set_marker_duration_multiplier),
    new LuaTableFunction("reset", script_strums_reset),
    new LuaTableFunction("force_key_release", script_strums_force_key_release),
    new LuaTableFunction("set_alpha", script_strums_set_alpha),
    new LuaTableFunction("enable_background", script_strums_enable_background),
    new LuaTableFunction("set_keep_aspect_ratio_background", script_strums_set_keep_aspect_ratio_background),
    new LuaTableFunction("set_alpha_background", script_strums_set_alpha_background),
    new LuaTableFunction("set_alpha_sick_effect", script_strums_set_alpha_sick_effect),
    new LuaTableFunction("set_draw_offset", script_strums_set_draw_offset),
    new LuaTableFunction("state_add", script_strums_state_add),
    new LuaTableFunction("state_toggle", script_strums_state_toggle),
    new LuaTableFunction("state_toggle_notes", script_strums_state_toggle_notes),
    new LuaTableFunction("state_toggle_marker_and_sick_effect", script_strums_state_toggle_marker_and_sick_effect),
    new LuaTableFunction("get_lines_count", script_strums_get_lines_count),
    new LuaTableFunction("get_strum_line", script_strums_get_strum_line),
    new LuaTableFunction("enable_post_sick_effect_draw", script_strums_enable_post_sick_effect_draw),
    new LuaTableFunction("animation_set", script_strums_animation_set),
    new LuaTableFunction("animation_restart", script_strums_animation_restart),
    new LuaTableFunction("animation_end", script_strums_animation_end),
    new LuaTableFunction(null, null)
};

        public static int script_strums_new(LuaState L, Strums strums) {
            return L.CreateUserdata(STRUMS, strums);
        }

        static int script_strums_gc(LuaState L) {
            return L.GC_userdata(STRUMS);
        }

        static int script_strums_tostring(LuaState L) {
            return L.ToString_userdata(STRUMS);
        }

        private static readonly LuaCallback delegate_gc = script_strums_gc;
        private static readonly LuaCallback delegate_tostring = script_strums_tostring;

        public static void script_strums_register(ManagedLuaState L) {
            L.RegisterMetaTable(STRUMS, delegate_gc, delegate_tostring, STRUMS_FUNCTIONS);
        }


    }
}

