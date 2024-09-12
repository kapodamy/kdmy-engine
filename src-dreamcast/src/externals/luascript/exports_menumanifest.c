#include "luascript_internal.h"

#include "game/gameplay/helpers/menumanifest.h"
#include "luascript_enums.h"


static int menumanifest_to_table(lua_State* L, MenuManifest mm) {
    if (!mm) {
        lua_pushnil(L);
        return 1;
    }

    lua_createtable(L, 0, 3);

    luascript_helper_add_field_table(L, "parameters", 0, 34);
    {
        luascript_helper_add_field_integer(L, "font_color", (lua_Integer)mm->parameters.font_color);
        luascript_helper_add_field_string(L, "atlas", mm->parameters.atlas);
        luascript_helper_add_field_string(L, "animlist", mm->parameters.animlist);
        luascript_helper_add_field_string(L, "font", mm->parameters.font);
        luascript_helper_add_field_number(L, "font_size", mm->parameters.font_size);
        luascript_helper_add_field_string(L, "font_glyph_suffix", mm->parameters.font_glyph_suffix);
        luascript_helper_add_field_boolean(L, "font_color_by_addition", mm->parameters.font_color_by_addition);
        luascript_helper_add_field_integer(L, "font_border_color", (lua_Integer)mm->parameters.font_border_color);
        luascript_helper_add_field_string(L, "anim_discarded", mm->parameters.anim_discarded);
        luascript_helper_add_field_string(L, "anim_idle", mm->parameters.anim_idle);
        luascript_helper_add_field_string(L, "anim_rollback", mm->parameters.anim_rollback);
        luascript_helper_add_field_string(L, "anim_selected", mm->parameters.anim_selected);
        luascript_helper_add_field_string(L, "anim_choosen", mm->parameters.anim_choosen);
        luascript_helper_add_field_string(L, "anim_in", mm->parameters.anim_in);
        luascript_helper_add_field_string(L, "anim_out", mm->parameters.anim_out);
        luascript_helper_add_field_number(L, "anim_transition_in_delay", mm->parameters.anim_transition_in_delay);
        luascript_helper_add_field_number(L, "anim_transition_out_delay", mm->parameters.anim_transition_out_delay);
        luascript_helper_add_field_boolean(L, "is_vertical", mm->parameters.is_vertical);
        luascript_helper_add_field_string(L, "items_align", luascript_helper_enums_stringify(LUASCRIPT_ENUMS_Align, mm->parameters.items_align));
        luascript_helper_add_field_number(L, "items_gap", mm->parameters.items_gap);
        luascript_helper_add_field_boolean(L, "is_sparse", mm->parameters.is_sparse);
        luascript_helper_add_field_integer(L, "static_index", mm->parameters.static_index);
        luascript_helper_add_field_boolean(L, "is_per_page", mm->parameters.is_per_page);
        luascript_helper_add_field_number(L, "items_dimmen", mm->parameters.items_dimmen);
        luascript_helper_add_field_number(L, "font_border_size", mm->parameters.font_border_size);
        luascript_helper_add_field_number(L, "texture_scale", mm->parameters.texture_scale);
        luascript_helper_add_field_boolean(L, "enable_horizontal_text_correction", mm->parameters.enable_horizontal_text_correction);
        luascript_helper_add_field_string(L, "suffix_selected", mm->parameters.suffix_selected);
        luascript_helper_add_field_string(L, "suffix_choosen", mm->parameters.suffix_choosen);
        luascript_helper_add_field_string(L, "suffix_discarded", mm->parameters.suffix_discarded);
        luascript_helper_add_field_string(L, "suffix_idle", mm->parameters.suffix_idle);
        luascript_helper_add_field_string(L, "suffix_rollback", mm->parameters.suffix_rollback);
        luascript_helper_add_field_string(L, "suffix_in", mm->parameters.suffix_in);
        luascript_helper_add_field_string(L, "suffix_out", mm->parameters.suffix_out);
    }
    lua_settable(L, -3);

    luascript_helper_add_field_table(L, "items", mm->items_size, 0);
    {
        for (int32_t i = 0; i < mm->items_size; i++) {
            luascript_helper_add_field_array_item_as_table(L, 0, 16, i + 1);
            {
                luascript_helper_add_field_table(L, "placement", 0, 4);
                {
                    luascript_helper_add_field_number(L, "gap", mm->items[i].placement.gap);
                    luascript_helper_add_field_number(L, "x", mm->items[i].placement.x);
                    luascript_helper_add_field_number(L, "y", mm->items[i].placement.y);
                    luascript_helper_add_field_number(L, "dimmen", mm->items[i].placement.dimmen);
                }
                lua_settable(L, -3);

                if (mm->items[i].has_font_color) {
                    luascript_helper_add_field_integer(L, "font_color", (lua_Integer)mm->items[i].font_color);
                } else {
                    luascript_helper_add_field_null(L, "font_color");
                }

                luascript_helper_add_field_string(L, "text", mm->items[i].text);
                luascript_helper_add_field_string(L, "model", mm->items[i].model);
                luascript_helper_add_field_number(L, "texture_scale", mm->items[i].texture_scale);
                luascript_helper_add_field_string(L, "name", mm->items[i].name);
                luascript_helper_add_field_string(L, "anim_selected", mm->items[i].anim_selected);
                luascript_helper_add_field_string(L, "anim_choosen", mm->items[i].anim_choosen);
                luascript_helper_add_field_string(L, "anim_discarded", mm->items[i].anim_discarded);
                luascript_helper_add_field_string(L, "anim_idle", mm->items[i].anim_idle);
                luascript_helper_add_field_string(L, "anim_rollback", mm->items[i].anim_rollback);
                luascript_helper_add_field_string(L, "anim_in", mm->items[i].anim_in);
                luascript_helper_add_field_string(L, "anim_out", mm->items[i].anim_out);
                luascript_helper_add_field_boolean(L, "hidden", mm->items[i].hidden);
                luascript_helper_add_field_string(L, "description", mm->items[i].description);
            }
            lua_settable(L, -3);
        }
    }
    lua_settable(L, -3);

    return 1;
}


static int script_menumanifest_parse_from_file(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);

    luascript_change_working_folder(L);
    MenuManifest menumanifest = menumanifest_init(src);
    luascript_restore_working_folder(L);

    int ret = menumanifest_to_table(L, menumanifest);
    if (menumanifest) menumanifest_destroy(&menumanifest);

    return ret;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg MENUMANIFEST_FUNCTIONS[] = {
    {"menumanifest_parse_from_file", script_menumanifest_parse_from_file},
    {NULL, NULL}
};

void script_menumanifest_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (MENUMANIFEST_FUNCTIONS[i].name == NULL || MENUMANIFEST_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MENUMANIFEST_FUNCTIONS[i].func);
        lua_setglobal(L, MENUMANIFEST_FUNCTIONS[i].name);
    }
}
