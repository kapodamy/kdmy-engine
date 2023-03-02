using Engine.Externals.LuaInterop;
using Engine.Game.Gameplay.Helpers;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMenuManifest {

        //public const string MENUMANIFEST = "MenuManifest";


        static int menumanifest_to_table(LuaState L, MenuManifest mm) {
            if (mm == null) {
                L.lua_pushnil();
                return 1;
            }

            L.lua_createtable(0, 3);

            LuaTableHelpers.AddFieldTable(L, "parameters", 0, 34);
            {
                LuaTableHelpers.AddFieldInteger(L, "font_color", mm.parameters.font_color);
                LuaTableHelpers.AddFieldString(L, "atlas", mm.parameters.atlas);
                LuaTableHelpers.AddFieldString(L, "animlist", mm.parameters.animlist);
                LuaTableHelpers.AddFieldString(L, "font", mm.parameters.font);
                LuaTableHelpers.AddFieldNumber(L, "font_size", mm.parameters.font_size);
                LuaTableHelpers.AddFieldString(L, "font_glyph_suffix", mm.parameters.font_glyph_suffix);
                LuaTableHelpers.AddFieldBoolean(L, "font_color_by_difference", mm.parameters.font_color_by_difference);
                LuaTableHelpers.AddFieldInteger(L, "font_border_color", mm.parameters.font_border_color);
                LuaTableHelpers.AddFieldString(L, "anim_discarded", mm.parameters.anim_discarded);
                LuaTableHelpers.AddFieldString(L, "anim_idle", mm.parameters.anim_idle);
                LuaTableHelpers.AddFieldString(L, "anim_rollback", mm.parameters.anim_rollback);
                LuaTableHelpers.AddFieldString(L, "anim_selected", mm.parameters.anim_selected);
                LuaTableHelpers.AddFieldString(L, "anim_choosen", mm.parameters.anim_choosen);
                LuaTableHelpers.AddFieldString(L, "anim_in", mm.parameters.anim_in);
                LuaTableHelpers.AddFieldString(L, "anim_out", mm.parameters.anim_out);
                LuaTableHelpers.AddFieldNumber(L, "anim_transition_in_delay", mm.parameters.anim_transition_in_delay);
                LuaTableHelpers.AddFieldNumber(L, "anim_transition_out_delay", mm.parameters.anim_transition_out_delay);
                LuaTableHelpers.AddFieldBoolean(L, "is_vertical", mm.parameters.is_vertical);
                LuaTableHelpers.AddFieldString(L, "items_align", LuascriptHelpers.StringifyAlign(mm.parameters.items_align));
                LuaTableHelpers.AddFieldNumber(L, "items_gap", mm.parameters.items_gap);
                LuaTableHelpers.AddFieldBoolean(L, "is_sparse", mm.parameters.is_sparse);
                LuaTableHelpers.AddFieldInteger(L, "static_index", mm.parameters.static_index);
                LuaTableHelpers.AddFieldBoolean(L, "is_per_page", mm.parameters.is_per_page);
                LuaTableHelpers.AddFieldNumber(L, "items_dimmen", mm.parameters.items_dimmen);
                LuaTableHelpers.AddFieldNumber(L, "font_border_size", mm.parameters.font_border_size);
                LuaTableHelpers.AddFieldNumber(L, "texture_scale", mm.parameters.texture_scale);
                LuaTableHelpers.AddFieldBoolean(L, "enable_horizontal_text_correction", mm.parameters.enable_horizontal_text_correction);
                LuaTableHelpers.AddFieldString(L, "suffix_selected", mm.parameters.suffix_selected);
                LuaTableHelpers.AddFieldString(L, "suffix_choosen", mm.parameters.suffix_choosen);
                LuaTableHelpers.AddFieldString(L, "suffix_discarded", mm.parameters.suffix_discarded);
                LuaTableHelpers.AddFieldString(L, "suffix_idle", mm.parameters.suffix_idle);
                LuaTableHelpers.AddFieldString(L, "suffix_rollback", mm.parameters.suffix_rollback);
                LuaTableHelpers.AddFieldString(L, "suffix_in", mm.parameters.suffix_in);
                LuaTableHelpers.AddFieldString(L, "suffix_out", mm.parameters.suffix_out);
            }
            L.lua_settable(-3);

            LuaTableHelpers.AddFieldTable(L, "items", mm.items_size, 0);
            {
                for (int i = 0 ; i < mm.items_size ; i++) {
                    LuaTableHelpers.AddFieldArrayItemAsTable(L, 0, 16, i + 1);
                    {
                        LuaTableHelpers.AddFieldTable(L, "placement", 0, 4);
                        {
                            LuaTableHelpers.AddFieldNumber(L, "gap", mm.items[i].placement.gap);
                            LuaTableHelpers.AddFieldNumber(L, "x", mm.items[i].placement.x);
                            LuaTableHelpers.AddFieldNumber(L, "y", mm.items[i].placement.y);
                            LuaTableHelpers.AddFieldNumber(L, "dimmen", mm.items[i].placement.dimmen);
                        }
                        L.lua_settable(-3);

                        if (mm.items[i].has_font_color) {
                            LuaTableHelpers.AddFieldInteger(L, "font_color", mm.items[i].font_color);
                        } else {
                            LuaTableHelpers.AddFieldNull(L, "font_color");
                        }

                        LuaTableHelpers.AddFieldString(L, "text", mm.items[i].text);
                        LuaTableHelpers.AddFieldString(L, "model", mm.items[i].model);
                        LuaTableHelpers.AddFieldNumber(L, "texture_scale", mm.items[i].texture_scale);
                        LuaTableHelpers.AddFieldString(L, "name", mm.items[i].name);
                        LuaTableHelpers.AddFieldString(L, "anim_selected", mm.items[i].anim_selected);
                        LuaTableHelpers.AddFieldString(L, "anim_choosen", mm.items[i].anim_choosen);
                        LuaTableHelpers.AddFieldString(L, "anim_discarded", mm.items[i].anim_discarded);
                        LuaTableHelpers.AddFieldString(L, "anim_idle", mm.items[i].anim_idle);
                        LuaTableHelpers.AddFieldString(L, "anim_rollback", mm.items[i].anim_rollback);
                        LuaTableHelpers.AddFieldString(L, "anim_in", mm.items[i].anim_in);
                        LuaTableHelpers.AddFieldString(L, "anim_out", mm.items[i].anim_out);
                        LuaTableHelpers.AddFieldBoolean(L, "hidden", mm.items[i].hidden);
                        LuaTableHelpers.AddFieldString(L, "description", mm.items[i].description);
                    }
                    L.lua_settable(-3);
                }
            }
            L.lua_settable(-3);

            return 1;
        }


        static int menumanifest_parse_from_file(LuaState L) {
            string src = L.luaL_checkstring(1);

            MenuManifest menumanifest = new MenuManifest(src);

            int ret = menumanifest_to_table(L, menumanifest);
            if (menumanifest != null) menumanifest.Destroy();

            return ret;
        }


        static readonly LuaTableFunction[] MENUMANIFEST_FUNCTIONS = {
            new LuaTableFunction("menumanifest_parse_from_file", menumanifest_parse_from_file),
            new LuaTableFunction(null, null)
        };

        public static void script_menumanifest_register(ManagedLuaState L) {
            L.RegisterGlobalFunctions(MENUMANIFEST_FUNCTIONS);
        }

    }
}
