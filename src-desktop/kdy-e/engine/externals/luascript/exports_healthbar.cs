using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsHealthBar {
        public const string HEALTHBAR = "HealthBar";


        static int script_healthbar_enable_extra_length(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool extra_enabled = L.lua_toboolean(2);

            healthbar.EnableExtraLength(extra_enabled);

            return 0;
        }

        static int script_healthbar_enable_vertical(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool enable_vertical = L.lua_toboolean(2);

            healthbar.EnableVertical(enable_vertical);

            return 0;
        }

        static int script_healthbar_state_opponent_add(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder icon_mdlhldr = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            ModelHolder bar_mdlhldr = L.ReadNullableUserdata<ModelHolder>(3, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(4, null);

            int ret = healthbar.StateOpponentAdd(icon_mdlhldr, bar_mdlhldr, state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_healthbar_state_opponent_add2(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder icon_mdlhldr = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            uint bar_color_rgb8 = (uint)L.luaL_checkinteger(3);
            string state_name = L.luaL_optstring(4, null);

            int ret = healthbar.StateOpponentAdd2(icon_mdlhldr, bar_color_rgb8, state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_healthbar_state_player_add(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder icon_mdlhldr = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            ModelHolder bar_mdlhldr = L.ReadNullableUserdata<ModelHolder>(3, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(4, null);

            int ret = healthbar.StatePlayerAdd(icon_mdlhldr, bar_mdlhldr, state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_healthbar_state_player_add2(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder icon_modelholder = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            uint bar_color_rgb8 = (uint)L.luaL_checkinteger(3);
            string state_name = L.luaL_optstring(4, null);

            int ret = healthbar.StatePlayerAdd2(icon_modelholder, bar_color_rgb8, state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_healthbar_state_background_add(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder modelholder = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(3, null);

            bool ret = healthbar.StateBackgroundAdd(modelholder, state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_state_background_add2(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            uint color_rgb8 = (uint)L.luaL_checkinteger(2);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(3, ExportsAnimSprite.ANIMSPRITE);
            string state_name = L.luaL_optstring(4, null);

            bool ret = healthbar.StateBackgroundAdd2(color_rgb8, animsprite, state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_load_warnings(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            ModelHolder modelholder = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            bool use_alt_icons = L.lua_toboolean(3);

            bool ret = healthbar.LoadWarnings(modelholder, use_alt_icons);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_set_opponent_bar_color_rgb8(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            uint color_rgb8 = (uint)L.luaL_checkinteger(2);

            healthbar.SetOpponentBarColorRGB8(color_rgb8);

            return 0;
        }

        static int script_healthbar_set_opponent_bar_color(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float r = (float)L.luaL_checknumber(2);
            float g = (float)L.luaL_checknumber(3);
            float b = (float)L.luaL_checknumber(4);

            healthbar.SetOpponentBarColor(r, g, b);

            return 0;
        }

        static int script_healthbar_set_player_bar_color_rgb8(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            uint color_rgb8 = (uint)L.luaL_checkinteger(2);

            healthbar.SetPlayerBarColorRGB8(color_rgb8);

            return 0;
        }

        static int script_healthbar_set_player_bar_color(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float r = (float)L.luaL_checknumber(2);
            float g = (float)L.luaL_checknumber(3);
            float b = (float)L.luaL_checknumber(4);

            healthbar.SetPlayerBarColor(r, g, b);

            return 0;
        }

        static int script_healthbar_state_toggle(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            string state_name = L.luaL_optstring(2, null);

            int ret = healthbar.StateToggle(state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_healthbar_state_toggle_background(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            string state_name = L.luaL_optstring(2, null);

            bool ret = healthbar.StateToggleBackground(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_state_toggle_player(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            string state_name = L.luaL_optstring(2, null);

            bool ret = healthbar.StateTogglePlayer(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_state_toggle_opponent(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            string state_name = L.luaL_optstring(2, null);

            bool ret = healthbar.StateToggleOpponent(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_healthbar_set_bump_animation_opponent(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            healthbar.SetBumpAnimationOpponent(animsprite);

            return 0;
        }

        static int script_healthbar_set_bump_animation_player(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            healthbar.SetBumpAnimationPlayer(animsprite);

            return 0;
        }

        static int script_healthbar_bump_enable(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool enable_bump = L.lua_toboolean(2);

            healthbar.BumpEnable(enable_bump);

            return 0;
        }

        static int script_healthbar_set_bpm(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float beats_per_minute = (float)L.luaL_checknumber(2);

            healthbar.SetBpm(beats_per_minute);

            return 0;
        }

        static int script_healthbar_set_alpha(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float alpha = (float)L.luaL_checknumber(2);

            healthbar.SetAlpha(alpha);

            return 0;
        }

        static int script_healthbar_set_visible(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool visible = L.lua_toboolean(2);

            healthbar.SetVisible(visible);

            return 0;
        }

        static int script_healthbar_get_drawable(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            Drawable ret = healthbar.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }

        static int script_healthbar_animation_set(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            healthbar.AnimationSet(animsprite);

            return 0;
        }

        static int script_healthbar_animation_restart(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            healthbar.AnimationRestart();

            return 0;
        }

        static int script_healthbar_animation_end(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            healthbar.AnimationEnd();

            return 0;
        }

        static int script_healthbar_disable_progress_animation(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool disable = L.lua_toboolean(2);

            healthbar.DisableProgressAnimation(disable);

            return 0;
        }

        static int script_healthbar_set_health_position(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float max_health = (float)L.luaL_checknumber(2);
            float health = (float)L.luaL_checknumber(3);
            bool opponent_recover = L.lua_toboolean(4);

            float ret = healthbar.SetHealthPosition(max_health, health, opponent_recover);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_healthbar_set_health_position2(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float percent = (float)L.luaL_checknumber(2);

            healthbar.SetHealthPosition2(percent);

            return 0;
        }

        static int script_healthbar_disable_icon_overlap(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool disable = L.lua_toboolean(2);

            healthbar.DisableIconOverlap(disable);

            return 0;
        }

        static int script_healthbar_disable_warnings(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool disable = L.lua_toboolean(2);

            healthbar.DisableWarnings(disable);

            return 0;
        }

        static int script_healthbar_enable_low_health_flash_warning(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool enable = L.lua_toboolean(2);

            healthbar.EnableLowHealthFlashWarning(enable);

            return 0;
        }

        static int script_healthbar_hide_warnings(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            healthbar.HideWarnings();

            return 0;
        }

        static int script_healthbar_show_drain_warning(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            bool use_fast_drain = L.lua_toboolean(2);

            healthbar.ShowDrainWarning(use_fast_drain);

            return 0;
        }

        static int script_healthbar_show_locked_warning(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            healthbar.ShowLockedWarning();

            return 0;
        }

        static int script_healthbar_get_bar_midpoint(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);
            float x, y;

            healthbar.GetBarMidpoint(out x, out y);

            L.lua_pushnumber(x);
            L.lua_pushnumber(y);
            return 0;
        }

        static int script_healthbar_get_percent(LuaState L) {
            HealthBar healthbar = L.ReadUserdata<HealthBar>(HEALTHBAR);

            float ret = healthbar.GetPercent();

            L.lua_pushnumber(ret);
            return 1;
        }




        static readonly LuaTableFunction[] HEALTHBAR_FUNCTIONS = {
            new LuaTableFunction("enable_extra_length", script_healthbar_enable_extra_length),
            new LuaTableFunction("enable_vertical", script_healthbar_enable_vertical),
            new LuaTableFunction("state_opponent_add", script_healthbar_state_opponent_add),
            new LuaTableFunction("state_opponent_add2", script_healthbar_state_opponent_add2),
            new LuaTableFunction("state_player_add", script_healthbar_state_player_add),
            new LuaTableFunction("state_player_add2", script_healthbar_state_player_add2),
            new LuaTableFunction("state_background_add", script_healthbar_state_background_add),
            new LuaTableFunction("state_background_add2", script_healthbar_state_background_add2),
            new LuaTableFunction("load_warnings", script_healthbar_load_warnings),
            new LuaTableFunction("set_opponent_bar_color_rgb8", script_healthbar_set_opponent_bar_color_rgb8),
            new LuaTableFunction("set_opponent_bar_color", script_healthbar_set_opponent_bar_color),
            new LuaTableFunction("set_player_bar_color_rgb8", script_healthbar_set_player_bar_color_rgb8),
            new LuaTableFunction("set_player_bar_color", script_healthbar_set_player_bar_color),
            new LuaTableFunction("state_toggle", script_healthbar_state_toggle),
            new LuaTableFunction("state_toggle_background", script_healthbar_state_toggle_background),
            new LuaTableFunction("state_toggle_player", script_healthbar_state_toggle_player),
            new LuaTableFunction("state_toggle_opponent", script_healthbar_state_toggle_opponent),
            new LuaTableFunction("set_bump_animation_opponent", script_healthbar_set_bump_animation_opponent),
            new LuaTableFunction("set_bump_animation_player", script_healthbar_set_bump_animation_player),
            new LuaTableFunction("bump_enable", script_healthbar_bump_enable),
            new LuaTableFunction("set_bpm", script_healthbar_set_bpm),
            new LuaTableFunction("set_alpha", script_healthbar_set_alpha),
            new LuaTableFunction("set_visible", script_healthbar_set_visible),
            new LuaTableFunction("get_drawable", script_healthbar_get_drawable),
            new LuaTableFunction("animation_set", script_healthbar_animation_set),
            new LuaTableFunction("animation_restart", script_healthbar_animation_restart),
            new LuaTableFunction("animation_end", script_healthbar_animation_end),
            new LuaTableFunction("disable_progress_animation", script_healthbar_disable_progress_animation),
            new LuaTableFunction("set_health_position", script_healthbar_set_health_position),
            new LuaTableFunction("set_health_position2", script_healthbar_set_health_position2),
            new LuaTableFunction("disable_icon_overlap", script_healthbar_disable_icon_overlap),
            new LuaTableFunction("disable_warnings", script_healthbar_disable_warnings),
            new LuaTableFunction("enable_low_health_flash_warning", script_healthbar_enable_low_health_flash_warning),
            new LuaTableFunction("hide_warnings", script_healthbar_hide_warnings),
            new LuaTableFunction("show_drain_warning", script_healthbar_show_drain_warning),
            new LuaTableFunction("show_locked_warning", script_healthbar_show_locked_warning),
            new LuaTableFunction("get_bar_midpoint", script_healthbar_get_bar_midpoint),
            new LuaTableFunction("get_percent", script_healthbar_get_percent),
            new LuaTableFunction(null, null)
        };

        public static int script_healthbar_new(LuaState L, HealthBar healthbar) {
            return L.CreateUserdata(HEALTHBAR, healthbar);
        }

        static int script_healthbar_gc(LuaState L) {
            return L.GC_userdata(HEALTHBAR);
        }

        static int script_healthbar_tostring(LuaState L) {
            return L.ToString_userdata(HEALTHBAR);
        }

        private static readonly LuaCallback delegate_gc = script_healthbar_gc;
        private static readonly LuaCallback delegate_tostring = script_healthbar_tostring;

        public static void script_healthbar_register(ManagedLuaState L) {
            L.RegisterMetaTable(HEALTHBAR, delegate_gc, delegate_tostring, HEALTHBAR_FUNCTIONS);
        }

    }
}

