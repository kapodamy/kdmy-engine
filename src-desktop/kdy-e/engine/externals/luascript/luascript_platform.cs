using System;
using System.Diagnostics;
using Engine.Externals.GLFW;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public class LuascriptPlatform {

        private static readonly MouseCallback CALLBACK_POSITION = InternalCallbackMousePosition;
        private static readonly MouseEnterCallback CALLBACK_ENTER = InternalCallbackMouseEnter;
        private static readonly MouseButtonCallback CALLBACK_BUTTON = InternalCallbackMouseButton;
        private static readonly MouseCallback CALLBACK_SCROLL = InternalCallbackMouseScroll;

        private static ArrayList<Luascript> luascript_instances;
        private static double last_mouse_position_x;
        private static double last_mouse_position_y;
        private static bool last_window_focused;
        private static bool last_window_minimized;
        private static bool title_was_changed;
        private static int last_resolution_changes;

        static LuascriptPlatform() {
            luascript_instances = new ArrayList<Luascript>(2);
        }

        public static void InitializeCallbacks() {
            Window window = PVRContext.InternalNativeWindow;

            last_mouse_position_x = Double.NaN;
            last_mouse_position_y = Double.NaN;
            last_window_focused = !PVRContext.global_context.IsOffscreen();
            last_window_minimized = PVRContext.global_context.IsMinimized();
            last_resolution_changes = PVRContext.global_context.resolution_changes;

            //Glfw.SetKeyCallback(window, InternalCallbackKeyboard);// used by KOS wrapper
            Glfw.SetCursorPositionCallback(window, CALLBACK_POSITION);
            Glfw.SetCursorEnterCallback(window, CALLBACK_ENTER);
            Glfw.SetMouseButtonCallback(window, CALLBACK_BUTTON);
            Glfw.SetScrollCallback(window, CALLBACK_SCROLL);
        }

        public static void AddLuascript(Luascript luascript) {
            if (luascript == null || luascript_instances.Has(luascript)) return;
            luascript_instances.Add(luascript);

            // notify input states if they do not have default values
            if (!Double.IsNaN(last_mouse_position_x) && !Double.IsNaN(last_mouse_position_y)) {
                luascript.notify_input_mouse_position(last_mouse_position_x, last_mouse_position_y);
            }
            if (!last_window_focused) luascript.notify_modding_window_focus(last_window_focused);
            if (last_window_minimized) luascript.notify_modding_window_focus(last_window_minimized);
        }

        public static void RemoveLuascript(Luascript luascript) {
            luascript_instances.Remove(luascript);
        }

        public static void PollWindowState() {
            PVRContext pvr_context = PVRContext.global_context;

            bool focused = !pvr_context.IsOffscreen();
            bool minimized = pvr_context.IsMinimized();
            int resolution_changes = pvr_context.resolution_changes;

            bool focused_updated = focused != last_window_focused;
            bool minimize_updated = minimized != last_window_minimized;
            bool resolution_updated = resolution_changes != last_resolution_changes;

            if (focused_updated || minimize_updated || resolution_updated) {
                Luascript[] array = luascript_instances.PeekArray();
                int size = luascript_instances.Size();
                for (int i = 0 ; i < size ; i++) {
                    Luascript luascript = array[i];
                    if (focused_updated) luascript.notify_modding_window_focus(focused);
                    if (minimize_updated) luascript.notify_modding_window_minimized(minimized);
                    if (resolution_updated) luascript.notify_window_size_changed(pvr_context.ScreenWidth, pvr_context.ScreenHeight);
                }

                last_window_focused = focused;
                last_window_minimized = minimized;
            }
        }


        public static void OpenWWWLink(string url) {
            if (String.IsNullOrEmpty(url)) return;
            Process.Start(new ProcessStartInfo() {
                //Verb = "open",
                FileName = url,
                UseShellExecute = true
            });
        }

        public static void RequestWindowAttention() {
            Glfw.RequestWindowAttention(PVRContext.InternalNativeWindow);
        }

        public static void ChangeWindowTitle(string title, bool changed_from_modding_context) {
            Glfw.SetWindowTitle(
                PVRContext.InternalNativeWindow, title ?? PVRContext.global_context.native_window_title
            );

            if (!changed_from_modding_context && title != null)
                title_was_changed = true;
            else if (title == null)
                title_was_changed = false;
        }

        public static void RestoreWindowTitleIfChanged() {
            if (!title_was_changed) return;
            Glfw.SetWindowTitle(
                PVRContext.InternalNativeWindow, PVRContext.global_context.native_window_title
            );
        }


        internal static void InternalCallbackKeyboard(Window window, Keys key, int scancode, InputState action, ModKeys mods) {
            Luascript[] array = luascript_instances.PeekArray();
            int size = luascript_instances.Size();

            for (int i = 0 ; i < size ; i++) {
                array[i].notify_input_keyboard((int)key, scancode, action == InputState.Press, (int)mods);
            }
        }

        private static void InternalCallbackMousePosition(Window window, double x, double y) {
            PVRContext.UnHideCursor();

            Luascript[] array = luascript_instances.PeekArray();
            int size = luascript_instances.Size();

            for (int i = 0 ; i < size ; i++) {
                array[i].notify_input_mouse_position(x, y);
            }
        }

        private static void InternalCallbackMouseEnter(Window window, bool entering) {
            Luascript[] array = luascript_instances.PeekArray();
            int size = luascript_instances.Size();

            for (int i = 0 ; i < size ; i++) {
                array[i].notify_input_mouse_enter(entering);
            }
        }

        private static void InternalCallbackMouseButton(Window window, MouseButtons btn, InputState state, ModKeys mods) {
            PVRContext.UnHideCursor();

            Luascript[] array = luascript_instances.PeekArray();
            int size = luascript_instances.Size();

            for (int i = 0 ; i < size ; i++) {
                array[i].notify_input_mouse_button((int)btn, state == InputState.Press, (int)mods);
            }
        }

        private static void InternalCallbackMouseScroll(Window window, double x, double y) {
            PVRContext.UnHideCursor();

            Luascript[] array = luascript_instances.PeekArray();
            int size = luascript_instances.Size();

            for (int i = 0 ; i < size ; i++) {
                array[i].notify_input_mouse_scroll(x, y);
            }
        }

    }
}
