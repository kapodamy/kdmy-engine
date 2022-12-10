using Engine.Externals.LuaInterop;
using Engine.Game.Common;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMessageBox {
        private const string MESSAGEBOX = "MessageBox";

        static int script_messagebox_set_buttons_text(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string left_text = L.luaL_optstring(2, null);
            string right_text = L.luaL_optstring(3, null);

            messagebox.SetButtonsText(left_text, right_text);

            return 0;
        }

        static int script_messagebox_set_button_single(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string center_text = L.luaL_optstring(2, null);

            messagebox.SetButtonSingle(center_text);

            return 0;
        }

        static int script_messagebox_set_buttons_icons(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string left_icon_name = L.luaL_optstring(2, null);
            string right_icon_name = L.luaL_optstring(3, null);

            messagebox.SetButtonsIcons(left_icon_name, right_icon_name);

            return 0;
        }

        static int script_messagebox_set_button_single_icon(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string center_icon_name = L.luaL_optstring(2, null);

            messagebox.SetButtonSingleIcon(center_icon_name);

            return 0;
        }

        static int script_messagebox_set_title(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string text = L.luaL_optstring(2, null);

            messagebox.SetTitle(text);

            return 0;
        }

        static int script_messagebox_set_image_background_color(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            long number = L.luaL_checkinteger(2);
            uint color_rgb8 = (uint)number;

            messagebox.SetImageBackgroundColor(color_rgb8);

            return 0;
        }

        static int script_messagebox_set_image_background_color_default(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            messagebox.SetImageBackgroundColorDefault();

            return 0;
        }

        static int script_messagebox_set_message(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string text = L.luaL_optstring(2, null);

            messagebox.SetMessage(text);

            return 0;
        }

        static int script_messagebox_hide_image_background(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool hide = L.lua_toboolean(2);

            messagebox.HideImageBackground(hide);

            return 0;
        }

        static int script_messagebox_hide_image(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool hide = L.lua_toboolean(2);

            messagebox.HideImage(hide);

            return 0;
        }

        static int script_messagebox_show_buttons_icons(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool show = L.lua_toboolean(2);

            messagebox.ShowButtonsIcons(show);

            return 0;
        }

        static int script_messagebox_use_small_size(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool small_or_normal = L.lua_toboolean(2);

            messagebox.UseSmallSize(small_or_normal);

            return 0;
        }

        static int script_messagebox_set_image_from_texture(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string filename = L.luaL_optstring(2, null);

            messagebox.SetImageFromTexture(filename);

            return 0;
        }

        static int script_messagebox_set_image_from_atlas(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            string filename = L.luaL_optstring(2, null);
            string entry_name = L.luaL_optstring(3, null);
            bool is_animation = L.lua_toboolean(4);

            messagebox.SetImageFromAtlas(filename, entry_name, is_animation);

            return 0;
        }

        static int script_messagebox_hide(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool animated = L.lua_toboolean(2);

            messagebox.Hide(animated);

            return 0;
        }

        static int script_messagebox_show(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool animated = L.lua_toboolean(2);

            messagebox.Show(animated);

            return 0;
        }

        static int script_messagebox_set_z_index(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            float z_index = (float)L.luaL_checknumber(2);

            messagebox.SetZIndex(z_index);

            return 0;
        }

        static int script_messagebox_get_modifier(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            Modifier modifier = messagebox.GetModifier();

            return ExportsModifier.script_modifier_new(L, modifier);
        }




        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] MESSAGEBOX_FUNCTIONS = {
            new LuaTableFunction("set_buttons_text", script_messagebox_set_buttons_text),
            new LuaTableFunction("set_button_single", script_messagebox_set_button_single),
            new LuaTableFunction("set_buttons_icons", script_messagebox_set_buttons_icons),
            new LuaTableFunction("set_button_single_icon", script_messagebox_set_button_single_icon),
            new LuaTableFunction("set_title", script_messagebox_set_title),
            new LuaTableFunction("set_image_background_color", script_messagebox_set_image_background_color),
            new LuaTableFunction("set_image_background_color_default", script_messagebox_set_image_background_color_default),
            new LuaTableFunction("set_message", script_messagebox_set_message),
            new LuaTableFunction("hide_image_background", script_messagebox_hide_image_background),
            new LuaTableFunction("hide_image", script_messagebox_hide_image),
            new LuaTableFunction("show_buttons_icons", script_messagebox_show_buttons_icons),
            new LuaTableFunction("use_small_size", script_messagebox_use_small_size),
            new LuaTableFunction("set_image_from_texture", script_messagebox_set_image_from_texture),
            new LuaTableFunction("set_image_from_atlas", script_messagebox_set_image_from_atlas),
            new LuaTableFunction("hide", script_messagebox_hide),
            new LuaTableFunction("show", script_messagebox_show),
            new LuaTableFunction("set_z_index", script_messagebox_set_z_index),
            new LuaTableFunction("get_modifier", script_messagebox_get_modifier),
            new LuaTableFunction(null, null)
        };

        public static object Messagebox { get; private set; }

        internal static int script_messagebox_new(LuaState L, MessageBox messagebox) {
            return L.CreateUserdata(MESSAGEBOX, messagebox);
        }

        static int script_messagebox_gc(LuaState L) {
            return L.GC_userdata(MESSAGEBOX);
        }

        static int script_messagebox_tostring(LuaState L) {
            return L.ToString_userdata(MESSAGEBOX);
        }


        private static readonly LuaCallback delegate_gc = script_messagebox_gc;
        private static readonly LuaCallback delegate_tostring = script_messagebox_tostring;

        internal static void script_messagebox_register(ManagedLuaState lua) {
            lua.RegisterMetaTable(MESSAGEBOX, delegate_gc, delegate_tostring, MESSAGEBOX_FUNCTIONS);
        }

    }
}

