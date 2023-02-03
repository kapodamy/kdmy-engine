using System;
using Engine.Externals.LuaInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMenu {

        public const string MENU = "Menu";


        static MenuManifest table_to_menumanifest(LuaState L, int idx) {
            MenuManifest mm = new MenuManifest();
            mm.parameters = new MenuManifest.Parameters();


            if (LuaTableHelpers.GetFieldTable(L, idx, "parameters")) {
                int idx2 = idx + 1;
                mm.parameters.font_color = (uint)LuaTableHelpers.GetFieldInteger(L, idx2, "font_color", 0x000000L);
                mm.parameters.atlas = LuaTableHelpers.GetFieldString(L, idx2, "atlas", null);
                mm.parameters.animlist = LuaTableHelpers.GetFieldString(L, idx2, "animlist", null);
                mm.parameters.font = LuaTableHelpers.GetFieldString(L, idx2, "font", null);
                mm.parameters.font_size = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "font_size", 0.0);
                mm.parameters.font_glyph_suffix = LuaTableHelpers.GetFieldString(L, idx2, "font_glyph_suffix", null);
                mm.parameters.font_color_by_difference = LuaTableHelpers.GetFieldBoolean(L, idx2, "font_color_by_difference", false);
                mm.parameters.font_border_color = (uint)LuaTableHelpers.GetFieldInteger(L, idx2, "font_border_color", 0x000000000);
                mm.parameters.anim_discarded = LuaTableHelpers.GetFieldString(L, idx2, "anim_discarded", null);
                mm.parameters.anim_idle = LuaTableHelpers.GetFieldString(L, idx2, "anim_idle", null);
                mm.parameters.anim_rollback = LuaTableHelpers.GetFieldString(L, idx2, "anim_rollback", null);
                mm.parameters.anim_selected = LuaTableHelpers.GetFieldString(L, idx2, "anim_selected", null);
                mm.parameters.anim_choosen = LuaTableHelpers.GetFieldString(L, idx2, "anim_choosen", null);
                mm.parameters.anim_in = LuaTableHelpers.GetFieldString(L, idx2, "anim_in", null);
                mm.parameters.anim_out = LuaTableHelpers.GetFieldString(L, idx2, "anim_out", null);
                mm.parameters.anim_transition_in_delay = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "anim_transition_in_delay", 0.0);
                mm.parameters.anim_transition_out_delay = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "anim_transition_out_delay", 0.0);
                mm.parameters.is_vertical = LuaTableHelpers.GetFieldBoolean(L, idx2, "is_vertical", false);
                mm.parameters.items_align = VertexProps.ParseAlign2(LuaTableHelpers.GetFieldString(L, idx2, "items_align", "CENTER"));
                mm.parameters.items_gap = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "items_gap", 0.0);
                mm.parameters.is_sparse = LuaTableHelpers.GetFieldBoolean(L, idx2, "is_sparse", false);
                mm.parameters.static_index = (int)LuaTableHelpers.GetFieldInteger(L, idx2, "static_index", 0);
                mm.parameters.is_per_page = LuaTableHelpers.GetFieldBoolean(L, idx2, "is_per_page", false);
                mm.parameters.items_dimmen = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "items_dimmen", 0.0);
                mm.parameters.font_border_size = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "font_border_size", 0.0);
                mm.parameters.texture_scale = (float)LuaTableHelpers.GetFieldNumber(L, idx2, "texture_scale", 0.0);
                mm.parameters.enable_horizontal_text_correction = LuaTableHelpers.GetFieldBoolean(L, idx2, "enable_horizontal_text_correction", false);
                mm.parameters.suffix_selected = LuaTableHelpers.GetFieldString(L, idx2, "suffix_selected", null);
                mm.parameters.suffix_choosen = LuaTableHelpers.GetFieldString(L, idx2, "suffix_choosen", null);
                mm.parameters.suffix_discarded = LuaTableHelpers.GetFieldString(L, idx2, "suffix_discarded", null);
                mm.parameters.suffix_idle = LuaTableHelpers.GetFieldString(L, idx2, "suffix_idle", null);
                mm.parameters.suffix_rollback = LuaTableHelpers.GetFieldString(L, idx2, "suffix_rollback", null);
                mm.parameters.suffix_in = LuaTableHelpers.GetFieldString(L, idx2, "suffix_in", null);
                mm.parameters.suffix_out = LuaTableHelpers.GetFieldString(L, idx2, "suffix_out", null);
                L.lua_pop(1);
            }


            if (LuaTableHelpers.GetFieldTable(L, idx, "items")) {
                int idx2 = L.lua_gettop();
                int length = (int)L.lua_rawlen(idx2);

                mm.items_size = length;
                mm.items = new MenuManifest.Item[length];

                for (int i = 0 ; i < length ; i++) {
                    if (LuaTableHelpers.GetArrayItemAsTable(L, idx2, i + 1)) {
                        int idx3 = L.lua_gettop();
                        int idx4 = -1;

                        if (LuaTableHelpers.GetFieldTable(L, idx3, "placement")) {
                            idx4 = L.lua_gettop();
                            mm.items[i].placement.gap = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "gap", Double.NaN);
                            mm.items[i].placement.x = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "x", Double.NaN);
                            mm.items[i].placement.y = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "y", Double.NaN);
                            mm.items[i].placement.dimmen = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "dimmen", Double.NaN);
                            L.lua_pop(1);
                        } else {
                            mm.items[i].placement.gap = Single.NaN;
                            mm.items[i].placement.x = Single.NaN;
                            mm.items[i].placement.y = Single.NaN;
                            mm.items[i].placement.dimmen = Single.NaN;
                        }

                        idx4 = L.lua_gettop();
                        mm.items[i].has_font_color = LuaTableHelpers.HasTableField(L, idx4, "font_color", LuaType.TNUMBER);
                        mm.items[i].font_color = (uint)LuaTableHelpers.GetFieldInteger(L, idx4, "font_color", 0x000000);
                        mm.items[i].text = LuaTableHelpers.GetFieldString(L, idx4, "text", null);
                        mm.items[i].modelholder = LuaTableHelpers.GetFieldString(L, idx4, "modelholder", null);
                        mm.items[i].texture_scale = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "texture_scale", Double.NaN);
                        mm.items[i].name = LuaTableHelpers.GetFieldString(L, idx4, "name", null);
                        mm.items[i].anim_selected = LuaTableHelpers.GetFieldString(L, idx4, "anim_selected", null);
                        mm.items[i].anim_choosen = LuaTableHelpers.GetFieldString(L, idx4, "anim_choosen", null);
                        mm.items[i].anim_discarded = LuaTableHelpers.GetFieldString(L, idx4, "anim_discarded", null);
                        mm.items[i].anim_idle = LuaTableHelpers.GetFieldString(L, idx4, "anim_idle", null);
                        mm.items[i].anim_rollback = LuaTableHelpers.GetFieldString(L, idx4, "anim_rollback", null);
                        mm.items[i].anim_in = LuaTableHelpers.GetFieldString(L, idx4, "anim_in", null);
                        mm.items[i].anim_out = LuaTableHelpers.GetFieldString(L, idx4, "anim_out", null);
                        mm.items[i].gap = (float)LuaTableHelpers.GetFieldNumber(L, idx4, "gap", Double.NaN);
                        mm.items[i].hidden = LuaTableHelpers.GetFieldBoolean(L, idx4, "hidden", false);

                        L.lua_pop(1);
                    }
                }
                L.lua_pop(1);
            }

            return mm;
        }

        static int script_menu_init(LuaState L) {
            MenuManifest menumanifest = table_to_menumanifest(L, 1);
            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);
            float z = (float)L.luaL_checknumber(4);
            float width = (float)L.luaL_checknumber(5);
            float height = (float)L.luaL_checknumber(6);

            if (menumanifest == null) return L.luaL_error("menumanifest argument can not be nil (null)");

            if (menumanifest.items_size < 1) {
                menumanifest.Destroy();
                return L.luaL_error("menumanifest does not contains items");
            }

            Menu ret = new Menu(menumanifest, x, y, z, width, height);

            menumanifest.Destroy();

            return L.CreateAllocatedUserdata<Menu>(MENU, ret);
        }

        /*
        // warning: do not use, this can lead to exploiting Drawable function pointers
        static int script_menu_destroy(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            if (L.IsUserdataAllocated(MENU))
                menu.Destroy();
            else
                Console.Error.WriteLine("[ERROR] script_menu_destroy() object was not allocated by lua\n");

            return 0;
        }*/

        static int script_menu_get_drawable(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            Drawable ret = menu.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }

        static int script_menu_trasition_in(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            menu.TrasitionIn();

            return 0;
        }

        static int script_menu_trasition_out(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            menu.TrasitionOut();

            return 0;
        }

        static int script_menu_select_item(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            string name = L.luaL_optstring(2, null);

            menu.SelectItem(name);

            return 0;
        }

        static int script_menu_select_index(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int index = (int)L.luaL_checkinteger(2);

            menu.SelectIndex(index);

            return 0;
        }

        static int script_menu_select_vertical(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int offset = (int)L.luaL_checkinteger(2);

            bool ret = menu.SelectVertical(offset);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_menu_select_horizontal(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int offset = (int)L.luaL_checkinteger(2);

            bool ret = menu.SelectHorizontal(offset);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_menu_toggle_choosen(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            bool enable = (bool)L.lua_toboolean(2);

            menu.ToggleChoosen(enable);

            return 0;
        }

        static int script_menu_get_selected_index(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            int ret = menu.GetSelectedIndex();

            L.lua_pushinteger((long)ret);
            return 1;
        }

        static int script_menu_get_items_count(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            int ret = menu.GetItemsCount();

            L.lua_pushinteger((long)ret);
            return 1;
        }

        static int script_menu_set_item_text(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int index = (int)L.luaL_checkinteger(2);
            string text = L.luaL_optstring(3, null);

            bool ret = menu.SetItemText(index, text);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_menu_set_item_visibility(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int index = (int)L.luaL_checkinteger(2);
            bool visible = (bool)L.lua_toboolean(3);

            bool ret = menu.SetItemVisibility(index, visible);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_menu_get_item_rect(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int index = (int)L.luaL_checkinteger(2);
            float x, y, width, height;

            bool ret = menu.GetItemRect(index, out x, out y, out width, out height);

            if (!ret) {
                L.lua_pushnil();
                return 1;
            }

            L.lua_pushnumber(x);
            L.lua_pushnumber(y);
            L.lua_pushnumber(width);
            L.lua_pushnumber(height);

            return 4;
        }

        static int script_menu_get_selected_item_name(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);

            string ret = menu.GetSelectedItemName();

            L.lua_pushstring(ret);
            return 1;
        }

        static int script_menu_set_text_force_case(LuaState L) {
            Menu menu = L.ReadUserdata<Menu>(MENU);
            int none_or_lowercase_or_uppercase = LuascriptHelpers.ParseForcecase(L, L.luaL_optstring(2, null));

            menu.SetTextForceCase(none_or_lowercase_or_uppercase);

            return 0;
        }



        static readonly LuaTableFunction[] MENU_FUNCTIONS = {
            new LuaTableFunction("init", script_menu_init),
//          new LuaTableFunction("destroy", script_menu_destroy),
            new LuaTableFunction("get_drawable", script_menu_get_drawable),
            new LuaTableFunction("trasition_in", script_menu_trasition_in),
            new LuaTableFunction("trasition_out", script_menu_trasition_out),
            new LuaTableFunction("select_item", script_menu_select_item),
            new LuaTableFunction("select_index", script_menu_select_index),
            new LuaTableFunction("select_vertical", script_menu_select_vertical),
            new LuaTableFunction("select_horizontal", script_menu_select_horizontal),
            new LuaTableFunction("toggle_choosen", script_menu_toggle_choosen),
            new LuaTableFunction("get_selected_index", script_menu_get_selected_index),
            new LuaTableFunction("get_items_count", script_menu_get_items_count),
            new LuaTableFunction("set_item_text", script_menu_set_item_text),
            new LuaTableFunction("set_item_visibility", script_menu_set_item_visibility),
            new LuaTableFunction("get_item_rect", script_menu_get_item_rect),
            new LuaTableFunction("get_selected_item_name", script_menu_get_selected_item_name),
            new LuaTableFunction("set_text_force_case", script_menu_set_text_force_case),
            new LuaTableFunction(null, null)
        };

        
        internal static int script_menu_new(LuaState L, Menu menu) {
            return L.CreateUserdata(MENU, menu);
        }

        internal static int script_menu_gc(LuaState L) {
            /*
            // if this object was allocated by lua, call the destructor
            return L.DestroyUserdata(MENU);
            */
            return L.GC_userdata(MENU);
        }

        static int script_menu_tostring(LuaState L) {
            return L.ToString_userdata(MENU);
        }

        
        private static readonly LuaCallback gc = script_menu_gc;
        private static readonly LuaCallback tostring = script_menu_tostring;

        internal static void script_menu_register(ManagedLuaState L) {
            L.RegisterMetaTable(MENU, gc, tostring, MENU_FUNCTIONS);
        }


    }
}
