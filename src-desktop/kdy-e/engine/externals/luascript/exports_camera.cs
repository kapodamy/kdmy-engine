using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Game.Gameplay;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsCamera {

        private const string CAMERA = "Camera";

        private static Layout FindLayout(Layout layout, Camera camera) {
            if (layout == null) return null;
            return layout.GetCameraHelper() == camera ? layout : null;
        }


        static int script_camera_set_interpolator_type(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            AnimInterpolator type;
            string type_str = L.luaL_optstring(2, null);

            if (type_str == "ease")
                type = AnimInterpolator.EASE;
            else if (type_str == "ease-in")
                type = AnimInterpolator.EASE_IN;
            else if (type_str == "ease-out")
                type = AnimInterpolator.EASE_OUT;
            else if (type_str == "ease-in-out")
                type = AnimInterpolator.EASE_IN_OUT;
            else if (type_str == "linear")
                type = AnimInterpolator.LINEAR;
            else if (type_str == "steps")
                type = AnimInterpolator.STEPS;
            else
                return L.luaL_error("invalid interpolator type");

            camera.SetInterpolatorType(type);

            return 0;
        }

        static int script_camera_set_transition_duration(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool expresed_in_beats = L.luaL_checkboolean(2);
            float value = L.luaL_checkfloat(3);

            camera.SetTransitionDuration(expresed_in_beats, value);

            return 0;
        }

        static int script_camera_set_absolute_zoom(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float z = L.luaL_checkfloat(2);

            camera.SetAbsoluteZoom(z);

            return 0;
        }

        static int script_camera_set_absolute_position(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = L.luaL_optionalfloat(2);
            float y = L.luaL_optionalfloat(3);

            camera.SetAbsolutePosition(x, y);

            return 0;
        }

        static int script_camera_set_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = L.luaL_optionalfloat(2);
            float y = L.luaL_optionalfloat(3);
            float z = L.luaL_optionalfloat(4);

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

            float x = L.luaL_optionalfloat(2);
            float y = L.luaL_optionalfloat(3);
            float z = L.luaL_optionalfloat(4);

            camera.Move(x, y, z);

            return 0;
        }

        static int script_camera_slide(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start_x = L.luaL_optionalfloat(2);
            float start_y = L.luaL_optionalfloat(3);
            float start_z = L.luaL_optionalfloat(4);
            float end_x = L.luaL_optionalfloat(5);
            float end_y = L.luaL_optionalfloat(6);
            float end_z = L.luaL_optionalfloat(7);

            camera.Slide(start_x, start_y, start_z, end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_x(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideX(start, end);

            return 0;
        }

        static int script_camera_slide_y(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideY(start, end);

            return 0;
        }

        static int script_camera_slide_z(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideZ(start, end);

            return 0;
        }

        static int script_camera_slide_to(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = L.luaL_optionalfloat(2);
            float y = L.luaL_optionalfloat(3);
            float z = L.luaL_optionalfloat(4);

            camera.SlideTo(x, y, z);

            return 0;
        }

        static int script_camera_from_layout(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            string camera_name = L.luaL_optstring(2, null);
            Layout layout = null;
            object context = L.Context;

            if (context is RoundContext) {
                RoundContext roundcontext = (RoundContext)context;
                layout = FindLayout(Week.GetStageLayout(roundcontext), camera);

                if (layout == null) {
                    layout = FindLayout(Week.UIGetLayout(roundcontext), camera);
                }
            } else if (context is Modding) {
                Modding modding = (Modding)context;
                layout = FindLayout(modding.GetLayout(), camera);
            }

            if (layout == null) {
                Console.WriteLine("script_camera_from_layout() Can not find the parent layout");
                L.lua_pushboolean(false);
                return 1;
            }

            bool ret = camera.FromLayout(layout, camera_name);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_camera_to_origin(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool should_slide = L.luaL_checkboolean(2);

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

            float end_x = L.luaL_optionalfloat(2);
            float end_y = L.luaL_optionalfloat(3);
            float end_z = L.luaL_optionalfloat(4);

            camera.MoveOffset(end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start_x = L.luaL_optionalfloat(2);
            float start_y = L.luaL_optionalfloat(3);
            float start_z = L.luaL_optionalfloat(4);
            float end_x = L.luaL_optionalfloat(5);
            float end_y = L.luaL_optionalfloat(6);
            float end_z = L.luaL_optionalfloat(7);

            camera.SlideOffset(start_x, start_y, start_z, end_x, end_y, end_z);

            return 0;
        }

        static int script_camera_slide_x_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideXOffset(start, end);

            return 0;
        }

        static int script_camera_slide_y_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideYOffset(start, end);

            return 0;
        }

        static int script_camera_slide_z_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float start = L.luaL_optionalfloat(2);
            float end = L.luaL_optionalfloat(3);

            camera.SlideZOffset(start, end);

            return 0;
        }

        static int script_camera_slide_to_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            float x = L.luaL_optionalfloat(2);
            float y = L.luaL_optionalfloat(3);
            float z = L.luaL_optionalfloat(4);

            camera.SlideToOffset(x, y, z);

            return 0;
        }

        static int script_camera_to_origin_offset(LuaState L) {
            Camera camera = L.ReadUserdata<Camera>(CAMERA);

            bool should_slide = L.luaL_checkboolean(2);

            camera.ToOriginOffset(should_slide);

            return 0;
        }



        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] CAMERA_FUNCTIONS = {
            new LuaTableFunction() { name = "set_interpolator_type", func = script_camera_set_interpolator_type},
            new LuaTableFunction() { name = "set_transition_duration", func = script_camera_set_transition_duration},
            new LuaTableFunction() { name = "set_absolute_zoom", func = script_camera_set_absolute_zoom},
            new LuaTableFunction() { name = "set_absolute_position", func = script_camera_set_absolute_position},
            new LuaTableFunction() { name = "set_offset", func = script_camera_set_offset},
            new LuaTableFunction() { name = "get_offset", func = script_camera_get_offset},
            new LuaTableFunction() { name = "get_modifier", func = script_camera_get_modifier},
            new LuaTableFunction() { name = "move", func = script_camera_move},
            new LuaTableFunction() { name = "slide", func = script_camera_slide},
            new LuaTableFunction() { name = "slide_x", func = script_camera_slide_x},
            new LuaTableFunction() { name = "slide_y", func = script_camera_slide_y},
            new LuaTableFunction() { name = "slide_z", func = script_camera_slide_z},
            new LuaTableFunction() { name = "slide_to", func = script_camera_slide_to},
            new LuaTableFunction() { name = "from_layout", func = script_camera_from_layout},
            new LuaTableFunction() { name = "to_origin", func = script_camera_to_origin},
            new LuaTableFunction() { name = "repeat", func = script_camera_repeat},
            new LuaTableFunction() { name = "stop", func = script_camera_stop},
            new LuaTableFunction() { name = "end", func = script_camera_end},
            new LuaTableFunction() { name = "is_completed", func = script_camera_is_completed},
            new LuaTableFunction() { name = "debug_log_info", func = script_camera_debug_log_info},
            new LuaTableFunction() { name = "apply", func = script_camera_apply},
            new LuaTableFunction() { name =  "move_offset", func = script_camera_move_offset },
            new LuaTableFunction() { name =  "slide_offset", func = script_camera_slide_offset },
            new LuaTableFunction() { name =  "slide_x_offset", func = script_camera_slide_x_offset },
            new LuaTableFunction() { name =  "slide_y_offset", func = script_camera_slide_y_offset },
            new LuaTableFunction() { name =  "slide_z_offset", func = script_camera_slide_z_offset },
            new LuaTableFunction() { name =  "slide_to_offset", func = script_camera_slide_to_offset },
            new LuaTableFunction() { name =  "to_origin_offset", func = script_camera_to_origin_offset },
            new LuaTableFunction() { name = null, func = null }
        };

        internal static int script_camera_new(LuaState L, Camera camera) {
            return L.CreateUserdata(CAMERA, camera);
        }

        static int script_camera_gc(LuaState L) {
            return L.NullifyUserdata(CAMERA);
        }

        static int script_camera_tostring(LuaState L) {
            L.lua_pushstring("[Camera]");
            return 1;
        }


        private static readonly LuaCallback delegate_gc = script_camera_gc;
        private static readonly LuaCallback delegate_tostring = script_camera_tostring;

        internal static void register_camera(ManagedLuaState L) {
            L.RegisterMetaTable(
                CAMERA,
                delegate_gc,
                delegate_tostring,
                CAMERA_FUNCTIONS
            );
        }

    }
}
