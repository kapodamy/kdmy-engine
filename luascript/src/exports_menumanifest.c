#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(MenuManifest, menumanifest_init, (const char* src), {
    let ret = await menumanifest_init(kdmyEngine_ptrToString(src));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, menumanifest_destroy, (MenuManifest* menumanifest), {
    menumanifest_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(menumanifest)));
});
#endif


#define ADD_FIELD_INTEGER(L, name, integer) \
    {                                       \
        lua_pushinteger(L, integer);        \
        lua_setfield(L, -2, name);          \
    }

#define ADD_FIELD_STRING(L, name, string) \
    {                                     \
        lua_pushstring(L, (string));      \
        lua_setfield(L, -2, name);        \
    }

#define ADD_FIELD_NUMBER(L, name, number) \
    {                                     \
        lua_pushnumber(L, number);        \
        lua_setfield(L, -2, name);        \
    }

#define ADD_FIELD_BOOLEAN(L, name, boolean) \
    {                                       \
        lua_pushboolean(L, boolean);        \
        lua_setfield(L, -2, name);          \
    }

#define ADD_FIELD_NULL(L, name)    \
    {                              \
        lua_pushnil(L);            \
        lua_setfield(L, -2, name); \
    }

#define ADD_FIELD_TABLE(L, name, narr, nrec) \
    {                                        \
        lua_pushstring(L, name);             \
        lua_createtable(L, narr, nrec);      \
    }

#define ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, narr, nrec, index_in_table) \
    {                                                                \
        lua_pushinteger(L, index_in_table);                          \
        lua_createtable(L, narr, nrec);                              \
    }


static int menumanifest_to_table(lua_State* L, MenuManifest mm) {
    if (!mm) {
        lua_pushnil(L);
        return 1;
    }

    lua_createtable(L, 0, 3);

    ADD_FIELD_TABLE(L, "parameters", 0, 34);
    {
        ADD_FIELD_INTEGER(L, "font_color", mm->parameters.font_color);
        ADD_FIELD_STRING(L, "atlas", mm->parameters.atlas);
        ADD_FIELD_STRING(L, "animlist", mm->parameters.animlist);
        ADD_FIELD_STRING(L, "font", mm->parameters.font);
        ADD_FIELD_NUMBER(L, "font_size", mm->parameters.font_size);
        ADD_FIELD_STRING(L, "font_glyph_suffix", mm->parameters.font_glyph_suffix);
        ADD_FIELD_BOOLEAN(L, "font_color_by_difference", mm->parameters.font_color_by_difference);
        ADD_FIELD_INTEGER(L, "font_border_color", mm->parameters.font_border_color);
        ADD_FIELD_STRING(L, "anim_discarded", mm->parameters.anim_discarded);
        ADD_FIELD_STRING(L, "anim_idle", mm->parameters.anim_idle);
        ADD_FIELD_STRING(L, "anim_rollback", mm->parameters.anim_rollback);
        ADD_FIELD_STRING(L, "anim_selected", mm->parameters.anim_selected);
        ADD_FIELD_STRING(L, "anim_choosen", mm->parameters.anim_choosen);
        ADD_FIELD_STRING(L, "anim_in", mm->parameters.anim_in);
        ADD_FIELD_STRING(L, "anim_out", mm->parameters.anim_out);
        ADD_FIELD_NUMBER(L, "anim_transition_in_delay", mm->parameters.anim_transition_in_delay);
        ADD_FIELD_NUMBER(L, "anim_transition_out_delay", mm->parameters.anim_transition_out_delay);
        ADD_FIELD_BOOLEAN(L, "is_vertical", mm->parameters.is_vertical);
        ADD_FIELD_STRING(L, "items_align", luascript_stringify_align(mm->parameters.items_align));
        ADD_FIELD_NUMBER(L, "items_gap", mm->parameters.items_gap);
        ADD_FIELD_BOOLEAN(L, "is_sparse", mm->parameters.is_sparse);
        ADD_FIELD_INTEGER(L, "static_index", mm->parameters.static_index);
        ADD_FIELD_BOOLEAN(L, "is_per_page", mm->parameters.is_per_page);
        ADD_FIELD_NUMBER(L, "items_dimmen", mm->parameters.items_dimmen);
        ADD_FIELD_NUMBER(L, "font_border_size", mm->parameters.font_border_size);
        ADD_FIELD_NUMBER(L, "texture_scale", mm->parameters.texture_scale);
        ADD_FIELD_BOOLEAN(L, "enable_horizontal_text_correction", mm->parameters.enable_horizontal_text_correction);
        ADD_FIELD_STRING(L, "suffix_selected", mm->parameters.suffix_selected);
        ADD_FIELD_STRING(L, "suffix_choosen", mm->parameters.suffix_choosen);
        ADD_FIELD_STRING(L, "suffix_discarded", mm->parameters.suffix_discarded);
        ADD_FIELD_STRING(L, "suffix_idle", mm->parameters.suffix_idle);
        ADD_FIELD_STRING(L, "suffix_rollback", mm->parameters.suffix_rollback);
        ADD_FIELD_STRING(L, "suffix_in", mm->parameters.suffix_in);
        ADD_FIELD_STRING(L, "suffix_out", mm->parameters.suffix_out);
    }
    lua_settable(L, -3);

    ADD_FIELD_TABLE(L, "items", mm->items_size, 0);
    {
        for (int32_t i = 0; i < mm->items_size; i++) {
            ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, 0, 16, i + 1);
            {
                ADD_FIELD_TABLE(L, "placement", 0, 4);
                {
                    ADD_FIELD_NUMBER(L, "gap", mm->items[i].placement.gap);
                    ADD_FIELD_NUMBER(L, "x", mm->items[i].placement.x);
                    ADD_FIELD_NUMBER(L, "y", mm->items[i].placement.y);
                    ADD_FIELD_NUMBER(L, "dimmen", mm->items[i].placement.dimmen);
                }
                lua_settable(L, -3);

                if (mm->items[i].has_font_color) {
                    ADD_FIELD_INTEGER(L, "font_color", mm->items[i].font_color);
                } else {
                    ADD_FIELD_NULL(L, "font_color");
                }

                ADD_FIELD_STRING(L, "text", mm->items[i].text);
                ADD_FIELD_STRING(L, "model", mm->items[i].model);
                ADD_FIELD_NUMBER(L, "texture_scale", mm->items[i].texture_scale);
                ADD_FIELD_STRING(L, "name", mm->items[i].name);
                ADD_FIELD_STRING(L, "anim_selected", mm->items[i].anim_selected);
                ADD_FIELD_STRING(L, "anim_choosen", mm->items[i].anim_choosen);
                ADD_FIELD_STRING(L, "anim_discarded", mm->items[i].anim_discarded);
                ADD_FIELD_STRING(L, "anim_idle", mm->items[i].anim_idle);
                ADD_FIELD_STRING(L, "anim_rollback", mm->items[i].anim_rollback);
                ADD_FIELD_STRING(L, "anim_in", mm->items[i].anim_in);
                ADD_FIELD_STRING(L, "anim_out", mm->items[i].anim_out);
                ADD_FIELD_BOOLEAN(L, "hidden", mm->items[i].hidden);
                ADD_FIELD_STRING(L, "description", mm->items[i].description);
            }
            lua_settable(L, -3);
        }
    }
    lua_settable(L, -3);

    return 1;
}


static int menumanifest_parse_from_file(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);

    MenuManifest menumanifest = menumanifest_init(src);

    int ret = menumanifest_to_table(L, menumanifest);
    if (menumanifest) menumanifest_destroy(&menumanifest);

    return ret;
}


static const luaL_Reg MENUMANIFEST_FUNCTIONS[] = {
    {"menumanifest_parse_from_file", menumanifest_parse_from_file},
    {NULL, NULL}
};

void script_menumanifest_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (MENUMANIFEST_FUNCTIONS[i].name == NULL || MENUMANIFEST_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MENUMANIFEST_FUNCTIONS[i].func);
        lua_setglobal(L, MENUMANIFEST_FUNCTIONS[i].name);
    }
}
