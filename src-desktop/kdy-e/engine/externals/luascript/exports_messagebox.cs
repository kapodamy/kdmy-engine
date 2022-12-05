using Engine.Externals.LuaInterop;
using Engine.Game.Common;

namespace Engine.Externals.LuaScriptInterop {

    public class ExportsMessageBox {
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

            bool hide = L.luaL_toboolean(2);

            messagebox.HideImageBackground(hide);

            return 0;
        }

        static int script_messagebox_hide_image(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool hide = L.luaL_toboolean(2);

            messagebox.HideImage(hide);

            return 0;
        }

        static int script_messagebox_show_buttons_icons(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool show = L.luaL_toboolean(2);

            messagebox.ShowButtonsIcons(show);

            return 0;
        }

        static int script_messagebox_use_small_size(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool small_or_normal = L.luaL_toboolean(2);

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
            bool is_animation = L.luaL_toboolean(4);

            messagebox.SetImageFromAtlas(filename, entry_name, is_animation);

            return 0;
        }

        static int script_messagebox_hide(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool animated = L.luaL_toboolean(2);

            messagebox.Hide(animated);

            return 0;
        }

        static int script_messagebox_show(LuaState L) {
            MessageBox messagebox = L.ReadUserdata<MessageBox>(MESSAGEBOX);

            bool animated = L.luaL_toboolean(2);

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
            new LuaTableFunction() { name = "set_buttons_text", func = script_messagebox_set_buttons_text},
            new LuaTableFunction() { name = "set_button_single", func = script_messagebox_set_button_single},
            new LuaTableFunction() { name = "set_buttons_icons", func = script_messagebox_set_buttons_icons},
            new LuaTableFunction() { name = "set_button_single_icon", func = script_messagebox_set_button_single_icon},
            new LuaTableFunction() { name = "set_title", func = script_messagebox_set_title},
            new LuaTableFunction() { name = "set_image_background_color", func = script_messagebox_set_image_background_color},
            new LuaTableFunction() { name = "set_image_background_color_default", func = script_messagebox_set_image_background_color_default},
            new LuaTableFunction() { name = "set_message", func = script_messagebox_set_message},
            new LuaTableFunction() { name = "hide_image_background", func = script_messagebox_hide_image_background},
            new LuaTableFunction() { name = "hide_image", func = script_messagebox_hide_image},
            new LuaTableFunction() { name = "show_buttons_icons", func = script_messagebox_show_buttons_icons},
            new LuaTableFunction() { name = "use_small_size", func = script_messagebox_use_small_size},
            new LuaTableFunction() { name = "set_image_from_texture", func = script_messagebox_set_image_from_texture},
            new LuaTableFunction() { name = "set_image_from_atlas", func = script_messagebox_set_image_from_atlas},
            new LuaTableFunction() { name = "hide", func = script_messagebox_hide},
            new LuaTableFunction() { name = "show", func = script_messagebox_show},
            new LuaTableFunction() { name = "set_z_index", func = script_messagebox_set_z_index},
            new LuaTableFunction() { name = "get_modifier", func = script_messagebox_get_modifier},
            new LuaTableFunction() { name = null, func = null}
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

