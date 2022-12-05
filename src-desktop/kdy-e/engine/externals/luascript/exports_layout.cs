using System;
using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsLayout {
        private const string LAYOUT = "Layout";



        static int script_layout_trigger_any(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string action_triger_camera_trigger_name = L.luaL_optstring(2, null);

            int ret = layout.TriggerAny(action_triger_camera_trigger_name);
            L.lua_pushinteger(ret);

            return 1;
        }

        static int script_layout_trigger_action(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string target_name = L.luaL_optstring(2, null);
            string action_name = L.luaL_optstring(3, null);

            int ret = layout.TriggerAction(target_name, action_name);
            L.lua_pushinteger(ret);

            return 1;
        }

        static int script_layout_trigger_camera(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string camera_name = L.luaL_optstring(2, null);

            bool ret = layout.TriggerCamera(camera_name);
            L.lua_pushboolean(ret);

            return 1;
        }

        static int script_layout_trigger_trigger(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string trigger_name = L.luaL_optstring(2, null);

            int ret = layout.TriggerTrigger(trigger_name);
            L.lua_pushinteger(ret);

            return 1;
        }

        static int script_layout_contains_action(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string target_name = L.luaL_optstring(2, null);
            string action_name = L.luaL_optstring(3, null);

            int ret = layout.ContainsAction(target_name, action_name);
            L.lua_pushinteger(ret);

            return 1;
        }

        static int script_layout_stop_all_triggers(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            layout.StopAllTriggers();
            return 0;
        }

        static int script_layout_stop_trigger(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string trigger_name = L.luaL_optstring(2, null);

            layout.StopTrigger(trigger_name);

            return 0;
        }

        static int script_layout_animation_is_completed(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string item_name = L.luaL_optstring(2, null);

            int ret = layout.AnimationIsCompleted(item_name);
            L.lua_pushinteger(ret);

            return 1;
        }

        static int script_layout_camera_set_view(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);
            float z = (float)L.luaL_checknumber(4);

            layout.CameraSetView(x, y, z);

            return 0;
        }

        static int script_layout_camera_is_completed(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            bool ret = layout.CameraIsCompleted();
            L.lua_pushboolean(ret);

            return 1;
        }

        static int script_layout_get_camera_helper(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            Camera camera = layout.GetCameraHelper();

            return ExportsCamera.script_camera_new(L, camera);
        }

        static int script_layout_get_secondary_camera_helper(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            Camera camera = layout.GetSecondaryCameraHelper();

            return ExportsCamera.script_camera_new(L, camera);
        }

        static int script_layout_get_textsprite(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);

            TextSprite textsprite = layout.GetTextsprite(name);

            return ExportsTextSprite.script_textsprite_new(L, textsprite);
        }

        static int script_layout_get_soundplayer(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);

            SoundPlayer soundplayer = layout.GetSoundplayer(name);

            return ExportsSoundPlayer.script_soundplayer_new(L, soundplayer);
        }

        static int script_layout_get_sprite(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);

            Sprite sprite = layout.GetSprite(name);

            return ExportsSprite.script_sprite_new(L, sprite);
        }

        static int script_layout_get_viewport_size(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            float size_width, size_height;
            layout.GetViewportSize(out size_width, out size_height);

            L.lua_pushnumber(size_width);
            L.lua_pushnumber(size_height);

            return 2;
        }

        static int script_layout_get_attached_value(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string name = L.luaL_optstring(2, null);

            if (name == null) {
                L.lua_pushnil();
                return 1;
            }

            object value;
            AttachedValueType value_type = layout.GetAttachedValue2(name, out value);

            switch (value_type) {
                case AttachedValueType.STRING:
                    if (value != null)
                        L.lua_pushstring((string)value);
                    else
                        L.lua_pushnil();
                    break;
                case AttachedValueType.FLOAT:
                    L.lua_pushnumber((double)value);
                    break;
                case AttachedValueType.INTEGER:
                    L.lua_pushinteger((long)value);
                    break;
                case AttachedValueType.HEX:
                    L.lua_pushinteger((uint)value);
                    break;
                case AttachedValueType.BOOLEAN:
                    L.lua_pushboolean((bool)value);
                    break;
                case AttachedValueType.NOTFOUND:
                default:
                    L.lua_pushvalue(3);// default value (taken from the arguments)
                    break;
            }

            return 1;
        }

        static int script_layout_set_group_visibility(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string group_name = L.luaL_optstring(2, null);
            bool visible = L.luaL_toboolean(3);

            layout.SetGroupVisibility(group_name, visible);

            return 0;
        }

        static int script_layout_set_group_alpha(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string group_name = L.luaL_optstring(2, null);
            float alpha = (float)L.luaL_checknumber(3);

            layout.SetGroupAlpha(group_name, alpha);

            return 0;
        }

        static int script_layout_set_group_offsetcolor(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            string group_name = L.luaL_optstring(2, null);
            float r = (float)L.luaL_optnumber(3, Double.NaN);
            float g = (float)L.luaL_optnumber(4, Double.NaN);
            float b = (float)L.luaL_optnumber(5, Double.NaN);
            float a = (float)L.luaL_optnumber(6, Double.NaN);

            layout.SetGroupOffsetcolor(group_name, r, g, b, a);

            return 0;
        }

        static int script_layout_suspend(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            layout.Suspend();
            return 0;
        }

        static int script_layout_resume(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);
            layout.Resume();
            return 0;
        }

        static int script_layout_get_placeholder(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);

            LayoutPlaceholder placeholder = layout.GetPlaceholder(name);
            if (placeholder != null) {
                LuaTableBuilder table = new LuaTableBuilder(12);

                table.AddInteger("group_id", placeholder.group_id);

                table.AddString("align_vertical", LuascriptHelpers.StringifyAlign(placeholder.align_vertical));
                table.AddString("align_horizontal", LuascriptHelpers.StringifyAlign(placeholder.align_horizontal));

                table.AddNumber("x", placeholder.x);
                table.AddNumber("y", placeholder.y);
                table.AddNumber("z", placeholder.z);

                table.AddNumber("height", placeholder.height);
                table.AddNumber("width", placeholder.width);

                table.AddNumber("parallax_x", placeholder.parallax.x);
                table.AddNumber("parallax_y", placeholder.parallax.y);
                table.AddNumber("parallax_z", placeholder.parallax.z);

                table.AddBoolean("static_camera", placeholder.static_camera);
                table.PushTable(L);
            } else {
                L.lua_pushnil();
            }

            return 1;
        }

        static int script_layout_disable_antialiasing(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            bool disable = L.luaL_toboolean(3);

            layout.DisableAntialiasing(disable);

            return 0;
        }

        static int script_layout_set_group_antialiasing(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string group_name = L.luaL_optstring(2, null);
            PVRContextFlag antialiasing = LuascriptHelpers.ParsePVRFLAG(L, L.luaL_optstring(3, null));

            layout.SetGroupAntialiasing(group_name, antialiasing);

            return 0;
        }

        static int script_layout_get_group_modifier(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string group_name = L.luaL_optstring(2, null);

            Modifier modifier = layout.GetGroupModifier(group_name);

            return ExportsModifier.script_modifier_new(L, modifier);
        }

        static int script_layout_get_group_shader(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);

            PSShader psshader = layout.GetGroupShader(name);

            if (psshader == null) {
                L.lua_pushnil();
                return 1;
            }

            return ExportsPSShader.script_psshader_new(L, psshader);
        }

        static int script_layout_set_group_shader(LuaState L) {
            Layout layout = L.ReadUserdata<Layout>(LAYOUT);

            string name = L.luaL_optstring(2, null);
            PSShader psshader = L.ReadNullableUserdata<PSShader>(3, ExportsPSShader.PSSHADER);

            bool ret = layout.SetGroupShader(name, psshader);

            L.lua_pushboolean(ret);

            return 1;
        }

        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] LAYOUT_FUNCTIONS = {
           new LuaTableFunction() { name = "trigger_any", func = script_layout_trigger_any},
           new LuaTableFunction() { name = "trigger_action", func = script_layout_trigger_action},
           new LuaTableFunction() { name = "trigger_camera", func = script_layout_trigger_camera},
           new LuaTableFunction() { name = "trigger_trigger", func = script_layout_trigger_trigger},
           new LuaTableFunction() { name = "contains_action", func = script_layout_contains_action},
           new LuaTableFunction() { name = "stop_all_triggers", func = script_layout_stop_all_triggers},
           new LuaTableFunction() { name = "stop_trigger", func = script_layout_stop_trigger},
           new LuaTableFunction() { name = "animation_is_completed", func = script_layout_animation_is_completed},
           new LuaTableFunction() { name = "camera_set_view", func = script_layout_camera_set_view},
           new LuaTableFunction() { name = "camera_is_completed", func = script_layout_camera_is_completed},
           new LuaTableFunction() { name = "get_camera_helper", func = script_layout_get_camera_helper},
           new LuaTableFunction() { name = "get_secondary_camera_helper", func = script_layout_get_secondary_camera_helper},
           new LuaTableFunction() { name = "get_textsprite", func = script_layout_get_textsprite},
           new LuaTableFunction() { name = "get_sprite", func = script_layout_get_sprite},
           new LuaTableFunction() { name = "get_soundplayer", func = script_layout_get_soundplayer},
           new LuaTableFunction() { name = "get_viewport_size", func = script_layout_get_viewport_size},
           new LuaTableFunction() { name = "get_attached_value", func = script_layout_get_attached_value},
           new LuaTableFunction() { name = "set_group_visibility", func = script_layout_set_group_visibility},
           new LuaTableFunction() { name = "set_group_alpha", func = script_layout_set_group_alpha},
           new LuaTableFunction() { name = "set_group_offsetcolor", func = script_layout_set_group_offsetcolor},
           new LuaTableFunction() { name = "suspend", func = script_layout_suspend},
           new LuaTableFunction() { name = "resume", func = script_layout_resume},
           new LuaTableFunction() { name = "get_placeholder", func = script_layout_get_placeholder},
           new LuaTableFunction() { name = "disable_antialiasing", func = script_layout_disable_antialiasing},
           new LuaTableFunction() { name = "set_group_antialiasing", func = script_layout_set_group_antialiasing},
           new LuaTableFunction() { name = "get_group_modifier", func = script_layout_get_group_modifier},
           new LuaTableFunction() { name = "get_group_shader", func = script_layout_get_group_shader},
           new LuaTableFunction() { name = "set_group_shader", func = script_layout_set_group_shader},
           new LuaTableFunction() { name = null, func = null}
        };


        internal static int script_layout_new(LuaState L, Layout layout) {
            return L.CreateUserdata(LAYOUT, layout);
        }

        static int script_layout_gc(LuaState L) {
            return L.GC_userdata(LAYOUT);
        }

        static int script_layout_tostring(LuaState L) {
            return L.ToString_userdata(LAYOUT);
        }


        private static readonly LuaCallback delegate_gc = script_layout_gc;
        private static readonly LuaCallback delegate_tostring = script_layout_tostring;

        internal static void script_layout_register(ManagedLuaState L) {
            L.RegisterMetaTable(LAYOUT, delegate_gc, delegate_tostring, LAYOUT_FUNCTIONS);
        }

    }
}
