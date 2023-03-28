#include "luascript_internal.h"
#include "string.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(Menu, menu_init, (MenuManifest menumanifest, float x, float y, float z, float width, float height), {
    let ret = await menu_init(kdmyEngine_obtain(menumanifest), x, y, z, width, height);
    kdmyEngine_forget(menumanifest);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Menu, menu_destroy, (Menu * menu), {
    let ret = menu_destroy(kdmyEngine_obtain(menu));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Drawable, menu_get_drawable, (Menu menu), {
    let ret = menu_get_drawable(kdmyEngine_obtain(menu));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, menu_trasition_in, (Menu menu), {
    menu_trasition_in(kdmyEngine_obtain(menu));
});
EM_JS_PRFX(void, menu_trasition_out, (Menu menu), {
    menu_trasition_out(kdmyEngine_obtain(menu));
});
EM_JS_PRFX(bool, menu_select_item, (Menu menu, const char* name), {
    let ret = menu_select_item(kdmyEngine_obtain(menu), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, menu_select_index, (Menu menu, int32_t index), {
    menu_select_index(kdmyEngine_obtain(menu), index);
});
EM_JS_PRFX(bool, menu_select_vertical, (Menu menu, int32_t offset), {
    let ret = menu_select_vertical(kdmyEngine_obtain(menu), offset);
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, menu_select_horizontal, (Menu menu, int32_t offset), {
    let ret = menu_select_horizontal(kdmyEngine_obtain(menu), offset);
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, menu_toggle_choosen, (Menu menu, bool enable), {
    menu_toggle_choosen(kdmyEngine_obtain(menu), enable);
});
EM_JS_PRFX(int32_t, menu_get_selected_index, (Menu menu), {
    let ret = menu_get_selected_index(kdmyEngine_obtain(menu));
    return ret;
});
EM_JS_PRFX(int32_t, menu_get_items_count, (Menu menu), {
    let ret = menu_get_items_count(kdmyEngine_obtain(menu));
    return ret;
});
EM_JS_PRFX(bool, menu_set_item_text, (Menu menu, int32_t index, const char* text), {
    let ret = menu_set_item_text(kdmyEngine_obtain(menu), index, kdmyEngine_ptrToString(text));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, menu_set_item_visibility, (Menu menu, int32_t index, bool visible), {
    let ret = menu_set_item_visibility(kdmyEngine_obtain(menu), index, visible);
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, menu_get_item_rect, (Menu menu, int32_t index, float* x, float* y, float* width, float* height), {
    const output_location = [0, 0];
    const output_size = [0, 0];

    let ret = menu_get_item_rect(kdmyEngine_obtain(menu), index, output_location, output_size);

    kdmyEngine_set_float32(x, output_location[0]);
    kdmyEngine_set_float32(y, output_location[1]);
    kdmyEngine_set_float32(width, output_size[2]);
    kdmyEngine_set_float32(height, output_size[3]);

    return ret ? 1 : 0;
});
EM_JS_PRFX(const char*, menu_get_selected_item_name, (Menu menu), {
    let ret = menu_get_selected_item_name(kdmyEngine_obtain(menu));
    return kdmyEngine_stringToPtr(ret);
});
EM_JS_PRFX(void, menu_set_text_force_case, (Menu menu, TextSpriteForceCase none_or_lowercase_or_uppercase), {
    menu_set_text_force_case(kdmyEngine_obtain(menu), none_or_lowercase_or_uppercase);
});
EM_JS_PRFX(bool, menu_has_item, (Menu menu, const char* name), {
    let ret = menu_has_item(kdmyEngine_obtain(menu), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(int32_t, menu_index_of_item, (Menu menu, const char* name), {
    let ret = menu_index_of_item(kdmyEngine_obtain(menu), kdmyEngine_ptrToString(name));
    return ret;
});

static void menumanifest_destroy_JS(MenuManifest* menumanifest) {
    MenuManifest obj = *menumanifest;

    for (int32_t i = 0; i < obj->items_size; i++) {
        free(obj->items[i].name);
        free(obj->items[i].text);
        free(obj->items[i].anim_selected);
        free(obj->items[i].anim_choosen);
        free(obj->items[i].anim_discarded);
        free(obj->items[i].anim_idle);
        free(obj->items[i].anim_rollback);
        free(obj->items[i].anim_in);
        free(obj->items[i].anim_out);
    }

    free(obj->parameters.suffix_selected);
    free(obj->parameters.suffix_choosen);
    free(obj->parameters.suffix_discarded);
    free(obj->parameters.suffix_idle);
    free(obj->parameters.suffix_rollback);
    free(obj->parameters.suffix_in);
    free(obj->parameters.suffix_out);

    free(obj->parameters.atlas);
    free(obj->parameters.animlist);

    free(obj->parameters.anim_selected);
    free(obj->parameters.anim_choosen);
    free(obj->parameters.anim_discarded);
    free(obj->parameters.anim_idle);
    free(obj->parameters.anim_rollback);
    free(obj->parameters.anim_in);
    free(obj->parameters.anim_out);

    free(obj->parameters.font);
    free(obj->parameters.font_glyph_suffix);

    free(obj->items);
    free(obj);

    menumanifest = NULL;
}

static MenuManifest menumanifest_as_object_JS(MenuManifest mm) {
    void* obj = kdmy_create_object();

    void* parameters = kdmy_create_object();
    kdmy_write_prop_object(obj, "parameters", parameters);
    kdmy_write_prop_integer(parameters, "font_color", (int32_t)mm->parameters.font_color);
    kdmy_write_prop_string(parameters, "atlas", mm->parameters.atlas);
    kdmy_write_prop_string(parameters, "animlist", mm->parameters.animlist);
    kdmy_write_prop_string(parameters, "font", mm->parameters.font);
    kdmy_write_prop_float(parameters, "font_size", mm->parameters.font_size);
    kdmy_write_prop_string(parameters, "font_glyph_suffix", mm->parameters.font_glyph_suffix);
    kdmy_write_prop_boolean(parameters, "font_color_by_difference", mm->parameters.font_color_by_difference);
    kdmy_write_prop_integer(parameters, "font_border_color", mm->parameters.font_border_color);
    kdmy_write_prop_string(parameters, "anim_discarded", mm->parameters.anim_discarded);
    kdmy_write_prop_string(parameters, "anim_idle", mm->parameters.anim_idle);
    kdmy_write_prop_string(parameters, "anim_rollback", mm->parameters.anim_rollback);
    kdmy_write_prop_string(parameters, "anim_selected", mm->parameters.anim_selected);
    kdmy_write_prop_string(parameters, "anim_choosen", mm->parameters.anim_choosen);
    kdmy_write_prop_string(parameters, "anim_in", mm->parameters.anim_in);
    kdmy_write_prop_string(parameters, "anim_out", mm->parameters.anim_out);
    kdmy_write_prop_float(parameters, "anim_transition_in_delay", mm->parameters.anim_transition_in_delay);
    kdmy_write_prop_float(parameters, "anim_transition_out_delay", mm->parameters.anim_transition_out_delay);
    kdmy_write_prop_boolean(parameters, "is_vertical", mm->parameters.is_vertical);
    kdmy_write_prop_integer(parameters, "items_align", mm->parameters.items_align);
    kdmy_write_prop_float(parameters, "items_gap", mm->parameters.items_gap);
    kdmy_write_prop_boolean(parameters, "is_sparse", mm->parameters.is_sparse);
    kdmy_write_prop_integer(parameters, "static_index", mm->parameters.static_index);
    kdmy_write_prop_boolean(parameters, "is_per_page", mm->parameters.is_per_page);
    kdmy_write_prop_float(parameters, "items_dimmen", mm->parameters.items_dimmen);
    kdmy_write_prop_float(parameters, "font_border_size", mm->parameters.font_border_size);
    kdmy_write_prop_float(parameters, "texture_scale", mm->parameters.texture_scale);
    kdmy_write_prop_boolean(parameters, "enable_horizontal_text_correction", mm->parameters.enable_horizontal_text_correction);
    kdmy_write_prop_string(parameters, "suffix_selected", mm->parameters.suffix_selected);
    kdmy_write_prop_string(parameters, "suffix_choosen", mm->parameters.suffix_choosen);
    kdmy_write_prop_string(parameters, "suffix_discarded", mm->parameters.suffix_discarded);
    kdmy_write_prop_string(parameters, "suffix_idle", mm->parameters.suffix_idle);
    kdmy_write_prop_string(parameters, "suffix_rollback", mm->parameters.suffix_rollback);
    kdmy_write_prop_string(parameters, "suffix_in", mm->parameters.suffix_in);
    kdmy_write_prop_string(parameters, "suffix_out", mm->parameters.suffix_out);
    kdmy_forget_obtained(parameters);

    void* items = kdmy_create_array(mm->items_size);
    kdmy_write_prop_object(obj, "items", items);
    kdmy_write_prop_integer(obj, "items_size", mm->items_size);

    for (int32_t i = 0; i < mm->items_size; i++) {
        void* item = kdmy_create_object();
        kdmy_write_in_array_object(items, i, item);
 
        void* placement = kdmy_create_object();
        kdmy_write_prop_object(item, "placement", placement);
        kdmy_write_prop_float(placement, "gap", mm->items[i].placement.gap);
        kdmy_write_prop_float(placement, "x", mm->items[i].placement.x);
        kdmy_write_prop_float(placement, "y", mm->items[i].placement.y);
        kdmy_write_prop_float(placement, "dimmen", mm->items[i].placement.dimmen);
        kdmy_forget_obtained(placement);

        kdmy_write_prop_boolean(items, "has_font_color", mm->items[i].has_font_color);
        kdmy_write_prop_integer(items, "font_color", (int32_t)mm->items[i].font_color);
        kdmy_write_prop_string(items, "text", mm->items[i].text);
        kdmy_write_prop_string(items, "model", mm->items[i].model);
        kdmy_write_prop_float(items, "texture_scale", mm->items[i].texture_scale);
        kdmy_write_prop_string(items, "name", mm->items[i].name);
        kdmy_write_prop_string(items, "anim_selected", mm->items[i].anim_selected);
        kdmy_write_prop_string(items, "anim_choosen", mm->items[i].anim_choosen);
        kdmy_write_prop_string(items, "anim_discarded", mm->items[i].anim_discarded);
        kdmy_write_prop_string(items, "anim_idle", mm->items[i].anim_idle);
        kdmy_write_prop_string(items, "anim_rollback", mm->items[i].anim_rollback);
        kdmy_write_prop_string(items, "anim_in", mm->items[i].anim_in);
        kdmy_write_prop_string(items, "anim_out", mm->items[i].anim_out);
        kdmy_write_prop_boolean(items, "hidden", mm->items[i].hidden);
        kdmy_write_prop_string(items, "description", mm->items[i].description);

        kdmy_forget_obtained(item);
    }
    kdmy_forget_obtained(items);

    return obj;
}

#endif



static lua_Integer GET_FIELD_INTEGER(lua_State* L, int idx, const char* name, lua_Integer def_value) {
    lua_getfield(L, idx, name);
    lua_Integer ret = luaL_optinteger(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

static char* GET_FIELD_STRING(lua_State* L, int idx, const char* name, const char* def_value) {
    lua_getfield(L, idx, name);
    char* ret = luascript_get_string_copy(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

static Align GET_FIELD_ALIGN(lua_State* L, int idx, const char* name, Align def_value) {
    lua_getfield(L, idx, name);
    const char* value = luaL_optstring(L, lua_gettop(L), NULL);

    Align ret;
    if (value == NULL || value[0] == '\0')
        ret = def_value;
    else
        ret = luascript_parse_align2(value);

    lua_pop(L, 1);
    return ret;
}

static lua_Number GET_FIELD_NUMBER(lua_State* L, int idx, const char* name, lua_Number def_value) {
    lua_getfield(L, idx, name);
    lua_Number ret = luaL_optnumber(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

static bool GET_FIELD_BOOLEAN(lua_State* L, int idx, const char* name, bool def_value) {
    int type = lua_getfield(L, idx, name);

    bool ret;
    if (type == LUA_TNONE)
        ret = def_value;
    else
        ret = lua_toboolean(L, lua_gettop(L));

    lua_pop(L, 1);
    return ret;
}

static bool GET_FIELD_TABLE(lua_State* L, int idx, const char* name) {
    int type = lua_getfield(L, idx, name);
    if (type == LUA_TTABLE) return true;

    lua_pop(L, 1);
    return false;
}

static bool GET_ARRAY_ITEM_AS_TABLE(lua_State* L, int idx, size_t index_in_table) {
    // this performs "local item = array[index_in_table]"
    lua_pushinteger(L, index_in_table);
    int type = lua_gettable(L, idx);

    if (type == LUA_TTABLE) return true;

    lua_pop(L, 1);
    return false;
}

static bool HAS_TABLE_FIELD(lua_State* L, int idx, const char* name, int desired_type) {
    int type = lua_getfield(L, idx, name);
    lua_pop(L, 1);
    return type == desired_type;
}

#define NAN_DOUBLE (0.0 / 0.0)


static MenuManifest table_to_menumanifest(lua_State* L, int idx) {
    MenuManifest mm = calloc(sizeof(MenuManifest_t), 1);

    if (GET_FIELD_TABLE(L, idx, "parameters")) {
        int idx2 = idx + 1;
        mm->parameters.font_color = (uint32_t)GET_FIELD_INTEGER(L, idx2, "font_color", 0x000000);
        mm->parameters.atlas = GET_FIELD_STRING(L, idx2, "atlas", NULL);
        mm->parameters.animlist = GET_FIELD_STRING(L, idx2, "animlist", NULL);
        mm->parameters.font = GET_FIELD_STRING(L, idx2, "font", NULL);
        mm->parameters.font_size = (float)GET_FIELD_NUMBER(L, idx2, "font_size", 0.0);
        mm->parameters.font_glyph_suffix = GET_FIELD_STRING(L, idx2, "font_glyph_suffix", NULL);
        mm->parameters.font_color_by_difference = GET_FIELD_BOOLEAN(L, idx2, "font_color_by_difference", false);
        mm->parameters.font_border_color = (uint32_t)GET_FIELD_INTEGER(L, idx2, "font_border_color", 0x000000000);
        mm->parameters.anim_discarded = GET_FIELD_STRING(L, idx2, "anim_discarded", NULL);
        mm->parameters.anim_idle = GET_FIELD_STRING(L, idx2, "anim_idle", NULL);
        mm->parameters.anim_rollback = GET_FIELD_STRING(L, idx2, "anim_rollback", NULL);
        mm->parameters.anim_selected = GET_FIELD_STRING(L, idx2, "anim_selected", NULL);
        mm->parameters.anim_choosen = GET_FIELD_STRING(L, idx2, "anim_choosen", NULL);
        mm->parameters.anim_in = GET_FIELD_STRING(L, idx2, "anim_in", NULL);
        mm->parameters.anim_out = GET_FIELD_STRING(L, idx2, "anim_out", NULL);
        mm->parameters.anim_transition_in_delay = (float)GET_FIELD_NUMBER(L, idx2, "anim_transition_in_delay", 0.0);
        mm->parameters.anim_transition_out_delay = (float)GET_FIELD_NUMBER(L, idx2, "anim_transition_out_delay", 0.0);
        mm->parameters.is_vertical = GET_FIELD_BOOLEAN(L, idx2, "is_vertical", false);
        mm->parameters.items_align = GET_FIELD_ALIGN(L, idx2, "items_align", ALIGN_CENTER);
        mm->parameters.items_gap = (float)GET_FIELD_NUMBER(L, idx2, "items_gap", 0.0);
        mm->parameters.is_sparse = GET_FIELD_BOOLEAN(L, idx2, "is_sparse", false);
        mm->parameters.static_index = (int32_t)GET_FIELD_INTEGER(L, idx2, "static_index", 0);
        mm->parameters.is_per_page = GET_FIELD_BOOLEAN(L, idx2, "is_per_page", false);
        mm->parameters.items_dimmen = (float)GET_FIELD_NUMBER(L, idx2, "items_dimmen", 0.0);
        mm->parameters.font_border_size = (float)GET_FIELD_NUMBER(L, idx2, "font_border_size", 0.0);
        mm->parameters.texture_scale = (float)GET_FIELD_NUMBER(L, idx2, "texture_scale", 0.0);
        mm->parameters.enable_horizontal_text_correction = GET_FIELD_BOOLEAN(L, idx2, "enable_horizontal_text_correction", false);
        mm->parameters.suffix_selected = GET_FIELD_STRING(L, idx2, "suffix_selected", NULL);
        mm->parameters.suffix_choosen = GET_FIELD_STRING(L, idx2, "suffix_choosen", NULL);
        mm->parameters.suffix_discarded = GET_FIELD_STRING(L, idx2, "suffix_discarded", NULL);
        mm->parameters.suffix_idle = GET_FIELD_STRING(L, idx2, "suffix_idle", NULL);
        mm->parameters.suffix_rollback = GET_FIELD_STRING(L, idx2, "suffix_rollback", NULL);
        mm->parameters.suffix_in = GET_FIELD_STRING(L, idx2, "suffix_in", NULL);
        mm->parameters.suffix_out = GET_FIELD_STRING(L, idx2, "suffix_out", NULL);
        lua_pop(L, 1);
    }


    if (GET_FIELD_TABLE(L, idx, "items")) {
        int idx2 = lua_gettop(L);
        const size_t length = lua_rawlen(L, idx2);

        mm->items_size = length;
        mm->items = calloc(sizeof(MenuManifestItem), length);

        for (size_t i = 0; i < length; i++) {
            if (GET_ARRAY_ITEM_AS_TABLE(L, idx2, i + 1)) {
                int idx3 = lua_gettop(L);
                int idx4 = -1;

                if (GET_FIELD_TABLE(L, idx3, "placement")) {
                    idx4 = lua_gettop(L);
                    mm->items[i].placement.gap = (float)GET_FIELD_NUMBER(L, idx4, "gap", NAN_DOUBLE);
                    mm->items[i].placement.x = (float)GET_FIELD_NUMBER(L, idx4, "x", NAN_DOUBLE);
                    mm->items[i].placement.y = (float)GET_FIELD_NUMBER(L, idx4, "y", NAN_DOUBLE);
                    mm->items[i].placement.dimmen = (float)GET_FIELD_NUMBER(L, idx4, "dimmen", NAN_DOUBLE);
                    lua_pop(L, 1);
                } else {
                    mm->items[i].placement.gap = NAN;
                    mm->items[i].placement.x = NAN;
                    mm->items[i].placement.y = NAN;
                    mm->items[i].placement.dimmen = NAN;
                }

                idx4 = lua_gettop(L);
                mm->items[i].has_font_color = HAS_TABLE_FIELD(L, idx4, "font_color", LUA_TNUMBER);
                mm->items[i].font_color = (uint32_t)GET_FIELD_INTEGER(L, idx4, "font_color", 0x000000);
                mm->items[i].text = GET_FIELD_STRING(L, idx4, "text", NULL);
                mm->items[i].model = GET_FIELD_STRING(L, idx4, "model", NULL);
                mm->items[i].texture_scale = (float)GET_FIELD_NUMBER(L, idx4, "texture_scale", NAN_DOUBLE);
                mm->items[i].name = GET_FIELD_STRING(L, idx4, "name", NULL);
                mm->items[i].anim_selected = GET_FIELD_STRING(L, idx4, "anim_selected", NULL);
                mm->items[i].anim_choosen = GET_FIELD_STRING(L, idx4, "anim_choosen", NULL);
                mm->items[i].anim_discarded = GET_FIELD_STRING(L, idx4, "anim_discarded", NULL);
                mm->items[i].anim_idle = GET_FIELD_STRING(L, idx4, "anim_idle", NULL);
                mm->items[i].anim_rollback = GET_FIELD_STRING(L, idx4, "anim_rollback", NULL);
                mm->items[i].anim_in = GET_FIELD_STRING(L, idx4, "anim_in", NULL);
                mm->items[i].anim_out = GET_FIELD_STRING(L, idx4, "anim_out", NULL);
                mm->items[i].hidden = GET_FIELD_BOOLEAN(L, idx4, "hidden", false);
                mm->items[i].description = GET_FIELD_STRING(L, idx4, "description", NULL);

                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }

    return mm;
}


static int script_menu_init(lua_State* L) {
    MenuManifest menumanifest = table_to_menumanifest(L, 2);
    float x = (float)luaL_checknumber(L, 3);
    float y = (float)luaL_checknumber(L, 4);
    float z = (float)luaL_checknumber(L, 5);
    float width = (float)luaL_checknumber(L, 6);
    float height = (float)luaL_checknumber(L, 7);

    if (!menumanifest) return luaL_error(L, "menumanifest argument can not be nil (null)");

#ifdef JAVASCRIPT
    if (menumanifest->items_size < 1) {
        menumanifest_destroy_JS(&menumanifest);
        return luaL_error(L, "menumanifest does not contains items");
    }

    MenuManifest menumanifest_jsobj = menumanifest_as_object_JS(menumanifest);

    Menu ret = menu_init(menumanifest_jsobj, x, y, z, width, height);

    menumanifest_destroy_JS(&menumanifest);

    return luascript_userdata_allocnew(L, MENU, ret);
#else
    if (menumanifest->items_size < 1) {
        menumanifest_destroy(&menumanifest);
        return luaL_error(L, "menumanifest does not contains items");
    }

    Menu ret = menu_init(menumanifest, x, y, z, width, height);

    menumanifest_destroy(&menumanifest);

    return luascript_userdata_allocnew(L, MENU, ret);
#endif
}

/*
// warning: do not use, this can lead to exploiting Drawable function pointers
static int script_menu_destroy(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    if (luascript_userdata_is_allocated(L, MENU))
        menu_destroy(&menu);
    else
        printf("script_menu_destroy() object was not allocated by lua\n");

    return 0;
}*/

static int script_menu_get_drawable(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    Drawable ret = menu_get_drawable(menu);

    return script_drawable_new(L, ret);
}

static int script_menu_trasition_in(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    menu_trasition_in(menu);

    return 0;
}

static int script_menu_trasition_out(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    menu_trasition_out(menu);

    return 0;
}

static int script_menu_select_item(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    const char* name = luaL_optstring(L, 2, NULL);

    bool ret = menu_select_item(menu, name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_select_index(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    menu_select_index(menu, index);

    return 0;
}

static int script_menu_select_vertical(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t offset = (int32_t)luaL_checkinteger(L, 2);

    bool ret = menu_select_vertical(menu, offset);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_select_horizontal(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t offset = (int32_t)luaL_checkinteger(L, 2);

    bool ret = menu_select_horizontal(menu, offset);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_toggle_choosen(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    bool enable = (bool)lua_toboolean(L, 2);

    menu_toggle_choosen(menu, enable);

    return 0;
}

static int script_menu_get_selected_index(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    int32_t ret = menu_get_selected_index(menu);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_menu_get_items_count(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    int32_t ret = menu_get_items_count(menu);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_menu_set_item_text(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    const char* text = luaL_optstring(L, 3, NULL);

    bool ret = menu_set_item_text(menu, index, text);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_set_item_visibility(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    bool visible = (bool)lua_toboolean(L, 3);

    bool ret = menu_set_item_visibility(menu, index, visible);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_get_item_rect(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    float x, y, width, height;

    bool ret = menu_get_item_rect(menu, index, &x, &y, &width, &height);

    if (!ret) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, width);
    lua_pushnumber(L, height);

    return 4;
}

static int script_menu_get_selected_item_name(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    const char* ret = menu_get_selected_item_name(menu);

    lua_pushstring(L, ret);

#ifdef JAVASCRIPT
    free((char*)ret);
#endif
    return 1;
}

static int script_menu_set_text_force_case(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    TextSpriteForceCase none_or_lowercase_or_uppercase = luascript_parse_forcecase(L, luaL_optstring(L, 2, NULL));

    menu_set_text_force_case(menu, none_or_lowercase_or_uppercase);

    return 0;
}

static int script_menu_has_item(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    const char* name = luaL_optstring(L, 2, NULL);

    bool ret = menu_has_item(menu, name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_menu_index_of_item(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    const char* name = luaL_optstring(L, 2, NULL);

    int32_t ret = menu_index_of_item(menu, name);

    lua_pushinteger(L, ret);
    return 1;
}






static const luaL_Reg MENU_FUNCTIONS[] = {
    {"init", script_menu_init},
//    { "destroy", script_menu_destroy },
    { "get_drawable", script_menu_get_drawable },
    { "trasition_in", script_menu_trasition_in },
    { "trasition_out", script_menu_trasition_out },
    { "select_item", script_menu_select_item },
    { "select_index", script_menu_select_index },
    { "select_vertical", script_menu_select_vertical },
    { "select_horizontal", script_menu_select_horizontal },
    { "toggle_choosen", script_menu_toggle_choosen },
    { "get_selected_index", script_menu_get_selected_index },
    { "get_items_count", script_menu_get_items_count },
    { "set_item_text", script_menu_set_item_text },
    { "set_item_visibility", script_menu_set_item_visibility },
    { "get_item_rect", script_menu_get_item_rect },
    { "get_selected_item_name", script_menu_get_selected_item_name },
    { "set_text_force_case", script_menu_set_text_force_case },
    { "has_item", script_menu_has_item },
    { "index_of_item", script_menu_index_of_item},
    { NULL, NULL }
};

int script_menu_new(lua_State* L, Menu menu) {
    return luascript_userdata_new(L, MENU, menu);
}

static int script_menu_gc(lua_State* L) {
    /*
    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, MENU, (Destructor)menu_destroy);
    */
   return luascript_userdata_gc(L, MENU);
}

static int script_menu_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, MENU);
}

void script_menu_register(lua_State* L) {
    luascript_register(L, MENU, script_menu_gc, script_menu_tostring, MENU_FUNCTIONS);
}
