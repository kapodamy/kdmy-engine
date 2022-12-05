using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsCharacter {

        private const string CHARACTER = "Character";



        static int script_character_use_alternate_sing_animations(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool enable = L.luaL_toboolean(2);

            character.UseAlternateSingAnimations(enable);

            return 0;
        }

        static int script_character_set_draw_location(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);

            character.SetDrawLocation(x, y);

            return 0;
        }

        static int script_character_set_draw_align(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string str1 = L.luaL_optstring(2, null);
            string str2 = L.luaL_optstring(3, null);

            Align align_vertical = VertexProps.ParseAlign2(str1);
            Align align_horizontal = VertexProps.ParseAlign2(str2);

            character.SetDrawAlign(align_vertical, align_horizontal);

            return 0;
        }

        static int script_character_update_reference_size(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float width = (float)L.luaL_optnumber(2, Double.NaN);
            float height = (float)L.luaL_optnumber(3, Double.NaN);

            character.UpdateReferenceSize(width, height);

            return 0;
        }

        static int script_character_enable_reference_size(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool enable = L.luaL_toboolean(2);

            character.EnableReferenceSize(enable);

            return 0;
        }

        static int script_character_set_offset(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float offset_x = (float)L.luaL_checknumber(2);
            float offset_y = (float)L.luaL_checknumber(3);

            character.SetOffset(offset_x, offset_y);

            return 0;
        }

        static int script_character_state_add(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            ModelHolder modelholder = L.luaL_testudata<ModelHolder>(2, "MODELHOLDER");
            string state_name = L.luaL_optstring(3, null);

            bool ret = character.StateAdd(modelholder, state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_state_toggle(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string state_name = L.luaL_optstring(2, null);

            bool ret = character.StateToggle(state_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_play_hey(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool ret = character.PlayHey();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_play_idle(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            int ret = character.PlayIdle();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_character_play_sing(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string direction = L.luaL_optstring(2, null);
            bool prefer_sustain = L.luaL_toboolean(3);

            bool ret = character.PlaySing(direction, prefer_sustain);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_play_miss(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string direction = L.luaL_optstring(2, null);
            bool keep_in_hold = L.luaL_toboolean(3);

            int ret = character.PlayMiss(direction, keep_in_hold);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_character_play_extra(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string extra_animation_name = L.luaL_optstring(2, null);
            bool prefer_sustain = L.luaL_toboolean(3);

            bool ret = character.PlayExtra(extra_animation_name, prefer_sustain);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_set_idle_speed(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float speed = (float)L.luaL_checknumber(2);

            character.SetIdleSpeed(speed);

            return 0;
        }

        static int script_character_set_scale(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float scale_factor = (float)L.luaL_checknumber(2);

            character.SetScale(scale_factor);

            return 0;
        }

        static int script_character_reset(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            character.Reset();

            return 0;
        }

        static int script_character_enable_continuous_idle(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool enable = L.luaL_toboolean(2);

            character.EnableContinuousIdle(enable);

            return 0;
        }

        static int script_character_is_idle_active(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool ret = character.IsIdleActive();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_enable_flip_correction(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool enable = L.luaL_toboolean(2);

            character.EnableFlipCorrection(enable);

            return 0;
        }

        static int script_character_flip_orientation(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool enable = L.luaL_toboolean(2);

            character.FlipOrientation(enable);

            return 0;
        }

        static int script_character_face_as_opponent(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool face_as_opponent = L.luaL_toboolean(2);

            character.FaceAsOpponent(face_as_opponent);

            return 0;
        }

        static int script_character_set_z_index(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float z = (float)L.luaL_checknumber(2);

            character.SetZIndex(z);

            return 0;
        }

        static int script_character_set_z_offset(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float z_offset = (float)L.luaL_checknumber(2);

            character.SetZOffset(z_offset);

            return 0;
        }

        static int script_character_animation_set(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            AnimSprite animsprite = L.luaL_testudata<AnimSprite>(2, "ANIMSPRITE");

            character.AnimationSet(animsprite);

            return 0;
        }

        static int script_character_animation_restart(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            character.AnimationRestart();

            return 0;
        }

        static int script_character_animation_end(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            character.AnimationEnd();

            return 0;
        }

        static int script_character_set_color_offset(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float r = (float)L.luaL_optnumber(2, Double.NaN);
            float g = (float)L.luaL_optnumber(3, Double.NaN);
            float b = (float)L.luaL_optnumber(4, Double.NaN);
            float a = (float)L.luaL_optnumber(5, Double.NaN);

            character.SetColorOffset(r, g, b, a);

            return 0;
        }

        static int script_character_set_color_offset_to_default(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            character.SetColorOffsetToDefault();

            return 0;
        }

        static int script_character_set_alpha(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            float alpha = (float)L.luaL_checknumber(2);

            character.SetAlpha(alpha);

            return 0;
        }

        static int script_character_set_visible(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            bool visible = L.luaL_toboolean(2);

            character.SetVisible(visible);

            return 0;
        }

        static int script_character_get_modifier(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            Modifier modifier = character.GetModifier();

            return ExportsModifier.script_modifier_new(L, modifier);
        }

        static int script_character_has_direction(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string name = L.luaL_optstring(2, null);
            bool is_extra = L.luaL_toboolean(3);

            bool ret = character.HasDirection(name, is_extra);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_character_get_play_calls(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            int ret = character.GetPlayCalls();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_character_get_current_action(LuaState L) {
            Character character = L.ReadUserdata<Character>(CHARACTER);

            string ret = LuascriptHelpers.StringifyActiontype(character.GetCurrentAction());

            L.lua_pushstring(ret);
            return 1;
        }



        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        private static readonly LuaTableFunction[] CHARACTER_FUNCTIONS = {
            new LuaTableFunction() { name = "use_alternate_sing_animations", func = script_character_use_alternate_sing_animations},
            new LuaTableFunction() { name = "set_draw_location", func = script_character_set_draw_location },
            new LuaTableFunction() { name = "set_draw_align", func = script_character_set_draw_align },
            new LuaTableFunction() { name = "update_reference_size", func = script_character_update_reference_size },
            new LuaTableFunction() { name = "enable_reference_size", func = script_character_enable_reference_size },
            new LuaTableFunction() { name = "set_offset", func = script_character_set_offset },
            new LuaTableFunction() { name = "state_add", func = script_character_state_add },
            new LuaTableFunction() { name = "state_toggle", func = script_character_state_toggle },
            new LuaTableFunction() { name = "play_hey", func = script_character_play_hey },
            new LuaTableFunction() { name = "play_idle", func = script_character_play_idle },
            new LuaTableFunction() { name = "play_sing", func = script_character_play_sing },
            new LuaTableFunction() { name = "play_miss", func = script_character_play_miss },
            new LuaTableFunction() { name = "play_extra", func = script_character_play_extra },
            new LuaTableFunction() { name = "set_idle_speed", func = script_character_set_idle_speed },
            new LuaTableFunction() { name = "set_scale", func = script_character_set_scale },
            new LuaTableFunction() { name = "reset", func = script_character_reset },
            new LuaTableFunction() { name = "enable_continuous_idle", func = script_character_enable_continuous_idle },
            new LuaTableFunction() { name = "is_idle_active", func = script_character_is_idle_active },
            new LuaTableFunction() { name = "enable_flip_correction", func = script_character_enable_flip_correction },
            new LuaTableFunction() { name = "flip_orientation", func = script_character_flip_orientation },
            new LuaTableFunction() { name = "face_as_opponent", func = script_character_face_as_opponent },
            new LuaTableFunction() { name = "set_z_index", func = script_character_set_z_index },
            new LuaTableFunction() { name = "set_z_offset", func = script_character_set_z_offset },
            new LuaTableFunction() { name = "animation_set", func = script_character_animation_set },
            new LuaTableFunction() { name = "animation_restart", func = script_character_animation_restart },
            new LuaTableFunction() { name = "animation_end", func = script_character_animation_end },
            new LuaTableFunction() { name = "set_color_offset", func = script_character_set_color_offset },
            new LuaTableFunction() { name = "set_color_offset_to_default", func = script_character_set_color_offset_to_default },
            new LuaTableFunction() { name = "set_alpha", func = script_character_set_alpha },
            new LuaTableFunction() { name = "set_visible", func = script_character_set_visible },
            new LuaTableFunction() { name = "get_modifier", func = script_character_get_modifier },
            new LuaTableFunction() { name = "has_direction", func = script_character_has_direction },
            new LuaTableFunction() { name = "get_play_calls", func = script_character_get_play_calls },
            new LuaTableFunction() { name = "get_current_action", func = script_character_get_current_action },
            new LuaTableFunction() { name = null, func = null }
        };

        internal static int script_character_new(LuaState L, Character character) {
            return L.CreateUserdata<Character>(CHARACTER, character);
        }

        static int script_character_gc(LuaState L) {
            return L.GC_userdata(CHARACTER);
        }

        static int script_character_tostring(LuaState L) {
            return L.ToString_userdata(CHARACTER);
        }


        private static readonly LuaCallback delegate_gc = script_character_gc;
        private static readonly LuaCallback delegate_tostring = script_character_tostring;


        internal static void script_character_register(ManagedLuaState L) {
            L.RegisterMetaTable(CHARACTER, delegate_gc, delegate_tostring, CHARACTER_FUNCTIONS);
        }

    }
}

