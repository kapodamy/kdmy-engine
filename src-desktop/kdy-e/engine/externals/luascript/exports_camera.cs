using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsCamera {

        private const string CAMERA = "Camera";

        static int script_camera_set_interpolator_type(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            AnimInterpolator type = LuascriptHelpers.ParseInterpolator(L, L.luaL_optstring(2, null));

            camera.SetInterpolatorType(type);

            return 0;
        }

        static int script_camera_set_transition_duration(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool expresed_in_beats = L.lua_toboolean(2);
            float value = (float)L.luaL_checknumber(3);

            camera.SetTransitionDuration(expresed_in_beats, value);

            return 0;
        }

        static int script_camera_set_absolute_zoom(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float z = (float)L.luaL_checknumber(2);

            camera.SetAbsoluteZoom(z);

            return 0;
        }

        static int script_camera_set_absolute_position(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = (float)L.luaL_optnumber(2, Double.NaN);
            float y = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SetAbsolutePosition(x, y);

            return 0;
        }

        static int script_camera_set_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = (float)L.luaL_optnumber(2, Double.NaN);
            float y = (float)L.luaL_optnumber(3, Double.NaN);
            float z = (float)L.luaL_optnumber(4, Double.NaN);

            camera.SetOffset(x, y, z);

            return 0;
        }

        static int script_camera_get_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x, y, z;

            camera.GetOffset(out x, out y, out z);

            L.lua_pushnumber(x);
            L.lua_pushnumber(y);
            L.lua_pushnumber(z);

            return 3;
        }

        static int script_camera_get_modifier(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            Modifier modifier = camera.GetModifier();
            return ExportsModifier.script_modifier_new(L, modifier);
        }

        static int script_camera_move(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = (float)L.luaL_optnumber(2, Double.NaN);
            float y = (float)L.luaL_optnumber(3, Double.NaN);
            float z = (float)L.luaL_optnumber(4, Double.NaN);

            camera.Move(x, y, z);

            return 0;
        }

        static int script_camera_slide(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start_x = (float)L.luaL_optnumber(2, Double.NaN);
            float start_y = (float)L.luaL_optnumber(3, Double.NaN);
            float start_z = (float)L.luaL_optnumber(4, Double.NaN);
            float end_x = (float)L.luaL_optnumber(5, Double.NaN);
            float end_y = (float)L.luaL_optnumber(6, Double.NaN);
            float end_z = (float)L.luaL_optnumber(7, Double.NaN);

            camera.Slide(start_x, start_y, start_z, end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_x(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideX(start, end);

            return 0;
        }

        static int script_camera_slide_y(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideY(start, end);

            return 0;
        }

        static int script_camera_slide_z(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideZ(start, end);

            return 0;
        }

        static int script_camera_slide_to(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = (float)L.luaL_optnumber(2, Double.NaN);
            float y = (float)L.luaL_optnumber(3, Double.NaN);
            float z = (float)L.luaL_optnumber(4, Double.NaN);

            camera.SlideTo(x, y, z);

            return 0;
        }

        static int script_camera_from_layout(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            string camera_name = L.luaL_optstring(2, null);
            Layout layout = camera.GetParentLayout();

            if (layout == null) {
                L.lua_pushboolean(false);
                return 1;
            }

            bool ret = camera.FromLayout(layout, camera_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_camera_to_origin(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool should_slide = L.lua_toboolean(2);

            camera.ToOrigin(should_slide);

            return 0;
        }

        static int script_camera_repeat(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            camera.Repeat();

            return 0;
        }

        static int script_camera_stop(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            camera.Stop();

            return 0;
        }

        static int script_camera_end(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            camera.End();

            return 0;
        }

        static int script_camera_is_completed(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool ret = camera.IsCompleted();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_camera_debug_log_info(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            camera.DebugLogInfo();

            return 0;
        }

        static int script_camera_apply(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            camera.Apply(null);

            return 0;
        }

        static int script_camera_move_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float end_x = (float)L.luaL_optnumber(2, Double.NaN);
            float end_y = (float)L.luaL_optnumber(3, Double.NaN);
            float end_z = (float)L.luaL_optnumber(4, Double.NaN);

            camera.MoveOffset(end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start_x = (float)L.luaL_optnumber(2, Double.NaN);
            float start_y = (float)L.luaL_optnumber(3, Double.NaN);
            float start_z = (float)L.luaL_optnumber(4, Double.NaN);
            float end_x = (float)L.luaL_optnumber(5, Double.NaN);
            float end_y = (float)L.luaL_optnumber(6, Double.NaN);
            float end_z = (float)L.luaL_optnumber(7, Double.NaN);

            camera.SlideOffset(start_x, start_y, start_z, end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_x_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideXOffset(start, end);

            return 0;
        }

        static int script_camera_slide_y_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideYOffset(start, end);

            return 0;
        }

        static int script_camera_slide_z_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = (float)L.luaL_optnumber(2, Double.NaN);
            float end = (float)L.luaL_optnumber(3, Double.NaN);

            camera.SlideZOffset(start, end);

            return 0;
        }

        static int script_camera_slide_to_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = (float)L.luaL_optnumber(2, Double.NaN);
            float y = (float)L.luaL_optnumber(3, Double.NaN);
            float z = (float)L.luaL_optnumber(4, Double.NaN);

            camera.SlideToOffset(x, y, z);

            return 0;
        }

        static int script_camera_to_origin_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool should_slide = L.lua_toboolean(2);

            camera.ToOriginOffset(should_slide);

            return 0;
        }



        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] CAMERA_FUNCTIONS = {
            new LuaTableFunction("set_interpolator_type", script_camera_set_interpolator_type),
            new LuaTableFunction("set_transition_duration", script_camera_set_transition_duration),
            new LuaTableFunction("set_absolute_zoom", script_camera_set_absolute_zoom),
            new LuaTableFunction("set_absolute_position", script_camera_set_absolute_position),
            new LuaTableFunction("set_offset", script_camera_set_offset),
            new LuaTableFunction("get_offset", script_camera_get_offset),
            new LuaTableFunction("get_modifier", script_camera_get_modifier),
            new LuaTableFunction("move", script_camera_move),
            new LuaTableFunction("slide", script_camera_slide),
            new LuaTableFunction("slide_x", script_camera_slide_x),
            new LuaTableFunction("slide_y", script_camera_slide_y),
            new LuaTableFunction("slide_z", script_camera_slide_z),
            new LuaTableFunction("slide_to", script_camera_slide_to),
            new LuaTableFunction("from_layout", script_camera_from_layout),
            new LuaTableFunction("to_origin", script_camera_to_origin),
            new LuaTableFunction("repeat", script_camera_repeat),
            new LuaTableFunction("stop", script_camera_stop),
            new LuaTableFunction("end", script_camera_end),
            new LuaTableFunction("is_completed", script_camera_is_completed),
            new LuaTableFunction("debug_log_info", script_camera_debug_log_info),
            new LuaTableFunction("apply", script_camera_apply),
            new LuaTableFunction("move_offset", script_camera_move_offset),
            new LuaTableFunction("slide_offset", script_camera_slide_offset),
            new LuaTableFunction("slide_x_offset", script_camera_slide_x_offset),
            new LuaTableFunction("slide_y_offset", script_camera_slide_y_offset),
            new LuaTableFunction("slide_z_offset", script_camera_slide_z_offset),
            new LuaTableFunction("slide_to_offset", script_camera_slide_to_offset),
            new LuaTableFunction("to_origin_offset", script_camera_to_origin_offset),
            new LuaTableFunction(null, null)
        };

        internal static int script_camera_new(LuaState L, Camera camera) {
            return L.CreateUserdata(CAMERA, camera);
        }

        static int script_camera_gc(LuaState L) {
            return L.GC_userdata(CAMERA);
        }

        static int script_camera_tostring(LuaState L) {
            return L.ToString_userdata(CAMERA);
        }


        private static readonly LuaCallback delegate_gc = script_camera_gc;
        private static readonly LuaCallback delegate_tostring = script_camera_tostring;

        internal static void script_camera_register(ManagedLuaState L) {
            L.RegisterMetaTable(CAMERA, delegate_gc, delegate_tostring, CAMERA_FUNCTIONS);
        }

    }
}
