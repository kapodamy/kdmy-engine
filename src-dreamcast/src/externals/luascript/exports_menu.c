#include "luascript_internal.h"

#include "game/common/menu.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "logger.h"
#include "luascript_enums.h"
#include "malloc_utils.h"
#include "math2d.h"


static MenuManifest table_to_menumanifest(lua_State* L, int idx) {
    MenuManifest mm = calloc_for_type(struct MenuManifest_s);
    malloc_assert(mm, MenuManifest);

    if (luascript_helper_get_field_table(L, idx, "parameters")) {
        int idx2 = idx + 1;
        mm->parameters.font_color = (uint32_t)luascript_helper_get_field_integer(L, idx2, "font_color", 0x000000);
        mm->parameters.atlas = luascript_helper_get_field_string(L, idx2, "atlas", NULL);
        mm->parameters.animlist = luascript_helper_get_field_string(L, idx2, "animlist", NULL);
        mm->parameters.font = luascript_helper_get_field_string(L, idx2, "font", NULL);
        mm->parameters.font_size = (float)luascript_helper_get_field_number(L, idx2, "font_size", 0.0);
        mm->parameters.font_glyph_suffix = luascript_helper_get_field_string(L, idx2, "font_glyph_suffix", NULL);
        mm->parameters.font_color_by_addition = luascript_helper_get_field_boolean(L, idx2, "font_color_by_addition", false);
        mm->parameters.font_border_color = (uint32_t)luascript_helper_get_field_integer(L, idx2, "font_border_color", 0x000000000);
        mm->parameters.anim_discarded = luascript_helper_get_field_string(L, idx2, "anim_discarded", NULL);
        mm->parameters.anim_idle = luascript_helper_get_field_string(L, idx2, "anim_idle", NULL);
        mm->parameters.anim_rollback = luascript_helper_get_field_string(L, idx2, "anim_rollback", NULL);
        mm->parameters.anim_selected = luascript_helper_get_field_string(L, idx2, "anim_selected", NULL);
        mm->parameters.anim_choosen = luascript_helper_get_field_string(L, idx2, "anim_choosen", NULL);
        mm->parameters.anim_in = luascript_helper_get_field_string(L, idx2, "anim_in", NULL);
        mm->parameters.anim_out = luascript_helper_get_field_string(L, idx2, "anim_out", NULL);
        mm->parameters.anim_transition_in_delay = (float)luascript_helper_get_field_number(L, idx2, "anim_transition_in_delay", 0.0);
        mm->parameters.anim_transition_out_delay = (float)luascript_helper_get_field_number(L, idx2, "anim_transition_out_delay", 0.0);
        mm->parameters.is_vertical = luascript_helper_get_field_boolean(L, idx2, "is_vertical", false);
        mm->parameters.items_align = luascript_helper_get_field_align(L, idx2, "items_align", ALIGN_CENTER);
        mm->parameters.items_gap = (float)luascript_helper_get_field_number(L, idx2, "items_gap", 0.0);
        mm->parameters.is_sparse = luascript_helper_get_field_boolean(L, idx2, "is_sparse", false);
        mm->parameters.static_index = (int32_t)luascript_helper_get_field_integer(L, idx2, "static_index", 0);
        mm->parameters.is_per_page = luascript_helper_get_field_boolean(L, idx2, "is_per_page", false);
        mm->parameters.items_dimmen = (float)luascript_helper_get_field_number(L, idx2, "items_dimmen", 0.0);
        mm->parameters.font_border_size = (float)luascript_helper_get_field_number(L, idx2, "font_border_size", 0.0);
        mm->parameters.texture_scale = (float)luascript_helper_get_field_number(L, idx2, "texture_scale", 0.0);
        mm->parameters.enable_horizontal_text_correction = luascript_helper_get_field_boolean(L, idx2, "enable_horizontal_text_correction", false);
        mm->parameters.suffix_selected = luascript_helper_get_field_string(L, idx2, "suffix_selected", NULL);
        mm->parameters.suffix_choosen = luascript_helper_get_field_string(L, idx2, "suffix_choosen", NULL);
        mm->parameters.suffix_discarded = luascript_helper_get_field_string(L, idx2, "suffix_discarded", NULL);
        mm->parameters.suffix_idle = luascript_helper_get_field_string(L, idx2, "suffix_idle", NULL);
        mm->parameters.suffix_rollback = luascript_helper_get_field_string(L, idx2, "suffix_rollback", NULL);
        mm->parameters.suffix_in = luascript_helper_get_field_string(L, idx2, "suffix_in", NULL);
        mm->parameters.suffix_out = luascript_helper_get_field_string(L, idx2, "suffix_out", NULL);
        lua_pop(L, 1);
    }

    if (luascript_helper_get_field_table(L, idx, "items")) {
        int idx2 = lua_gettop(L);
        const lua_Unsigned length = lua_rawlen(L, idx2);
        assert(length <= INT32_MAX);

        mm->items_size = (int32_t)length;
        mm->items = calloc_for_array(length, MenuManifestItem);

        for (int32_t i = 0; i < mm->items_size; i++) {
            if (luascript_helper_get_array_item_as_table(L, idx2, i + 1)) {
                int idx3 = lua_gettop(L);
                int idx4 = -1;

                if (luascript_helper_get_field_table(L, idx3, "placement")) {
                    idx4 = lua_gettop(L);
                    mm->items[i].placement.gap = (float)luascript_helper_get_field_number(L, idx4, "gap", DOUBLE_NaN);
                    mm->items[i].placement.x = (float)luascript_helper_get_field_number(L, idx4, "x", DOUBLE_NaN);
                    mm->items[i].placement.y = (float)luascript_helper_get_field_number(L, idx4, "y", DOUBLE_NaN);
                    mm->items[i].placement.dimmen = (float)luascript_helper_get_field_number(L, idx4, "dimmen", DOUBLE_NaN);
                    lua_pop(L, 1);
                } else {
                    mm->items[i].placement.gap = FLOAT_NaN;
                    mm->items[i].placement.x = FLOAT_NaN;
                    mm->items[i].placement.y = FLOAT_NaN;
                    mm->items[i].placement.dimmen = FLOAT_NaN;
                }

                idx4 = lua_gettop(L);
                mm->items[i].has_font_color = luascript_helper_has_table_field(L, idx4, "font_color", LUA_TNUMBER);
                mm->items[i].font_color = (uint32_t)luascript_helper_get_field_integer(L, idx4, "font_color", 0x000000);
                mm->items[i].text = luascript_helper_get_field_string(L, idx4, "text", NULL);
                mm->items[i].model = luascript_helper_get_field_string(L, idx4, "model", NULL);
                mm->items[i].texture_scale = (float)luascript_helper_get_field_number(L, idx4, "texture_scale", DOUBLE_NaN);
                mm->items[i].name = luascript_helper_get_field_string(L, idx4, "name", NULL);
                mm->items[i].anim_selected = luascript_helper_get_field_string(L, idx4, "anim_selected", NULL);
                mm->items[i].anim_choosen = luascript_helper_get_field_string(L, idx4, "anim_choosen", NULL);
                mm->items[i].anim_discarded = luascript_helper_get_field_string(L, idx4, "anim_discarded", NULL);
                mm->items[i].anim_idle = luascript_helper_get_field_string(L, idx4, "anim_idle", NULL);
                mm->items[i].anim_rollback = luascript_helper_get_field_string(L, idx4, "anim_rollback", NULL);
                mm->items[i].anim_in = luascript_helper_get_field_string(L, idx4, "anim_in", NULL);
                mm->items[i].anim_out = luascript_helper_get_field_string(L, idx4, "anim_out", NULL);
                mm->items[i].hidden = luascript_helper_get_field_boolean(L, idx4, "hidden", false);
                mm->items[i].description = luascript_helper_get_field_string(L, idx4, "description", NULL);

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

    if (!menumanifest) {
        return luaL_error(L, "menumanifest argument can not be nil (null)");
    }

    if (menumanifest->items_size < 1) {
        menumanifest_destroy(&menumanifest);
        return luaL_error(L, "menumanifest does not contains items");
    }

    Menu ret = menu_init(menumanifest, x, y, z, width, height);
    menumanifest_destroy(&menumanifest);

    return luascript_userdata_allocnew(L, MENU, ret);
}

/*
// warning: do not use, this can lead to exploiting Drawable function pointers
static int script_menu_destroy(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    if (luascript_userdata_is_allocated(L, MENU))
        menu_destroy(&menu);
    else
        logger_warn("script_menu_destroy() object was not allocated by lua");

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

    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);
    lua_pushnumber(L, (lua_Number)width);
    lua_pushnumber(L, (lua_Number)height);

    return 4;
}

static int script_menu_get_selected_item_name(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    const char* ret = menu_get_selected_item_name(menu);

    lua_pushstring(L, ret);

    return 1;
}

static int script_menu_set_text_force_case(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);

    int32_t none_or_lowercase_or_uppercase = (int32_t)luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_TextSpriteForceCase);

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

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_menu_set_item_image(lua_State* L) {
    Menu menu = luascript_read_userdata(L, MENU);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    const char* atlas_or_animlist_entry_name = luaL_optstring(L, 4, NULL);

    menu_set_item_image(menu, index, modelholder, atlas_or_animlist_entry_name);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg MENU_FUNCTIONS[] = {
    {"init", script_menu_init},
    //    { "destroy", script_menu_destroy },
    {"get_drawable", script_menu_get_drawable},
    {"trasition_in", script_menu_trasition_in},
    {"trasition_out", script_menu_trasition_out},
    {"select_item", script_menu_select_item},
    {"select_index", script_menu_select_index},
    {"select_vertical", script_menu_select_vertical},
    {"select_horizontal", script_menu_select_horizontal},
    {"toggle_choosen", script_menu_toggle_choosen},
    {"get_selected_index", script_menu_get_selected_index},
    {"get_items_count", script_menu_get_items_count},
    {"set_item_text", script_menu_set_item_text},
    {"set_item_visibility", script_menu_set_item_visibility},
    {"get_item_rect", script_menu_get_item_rect},
    {"get_selected_item_name", script_menu_get_selected_item_name},
    {"set_text_force_case", script_menu_set_text_force_case},
    {"has_item", script_menu_has_item},
    {"index_of_item", script_menu_index_of_item},
    {"set_item_image", script_menu_set_item_image},
    {NULL, NULL}
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
