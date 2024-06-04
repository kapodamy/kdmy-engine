"use strict";

const MENU = "Menu";


function luascript_helper_table_to_menumanifest(L, idx) {
    let mm = {};

    if (luascript_helper_get_field_table(L, idx, "parameters")) {
        let idx2 = idx + 1;
        mm.parameters.font_color = luascript_helper_get_field_integer(L, idx2, "font_color", 0x000000);
        mm.parameters.atlas = luascript_helper_get_field_string(L, idx2, "atlas", null);
        mm.parameters.animlist = luascript_helper_get_field_string(L, idx2, "animlist", null);
        mm.parameters.font = luascript_helper_get_field_string(L, idx2, "font", null);
        mm.parameters.font_size = luascript_helper_get_field_number(L, idx2, "font_size", 0.0);
        mm.parameters.font_glyph_suffix = luascript_helper_get_field_string(L, idx2, "font_glyph_suffix", null);
        mm.parameters.font_color_by_addition = luascript_helper_get_field_boolean(L, idx2, "font_color_by_addition", false);
        mm.parameters.font_border_color = luascript_helper_get_field_integer(L, idx2, "font_border_color", 0x000000000);
        mm.parameters.anim_discarded = luascript_helper_get_field_string(L, idx2, "anim_discarded", null);
        mm.parameters.anim_idle = luascript_helper_get_field_string(L, idx2, "anim_idle", null);
        mm.parameters.anim_rollback = luascript_helper_get_field_string(L, idx2, "anim_rollback", null);
        mm.parameters.anim_selected = luascript_helper_get_field_string(L, idx2, "anim_selected", null);
        mm.parameters.anim_choosen = luascript_helper_get_field_string(L, idx2, "anim_choosen", null);
        mm.parameters.anim_in = luascript_helper_get_field_string(L, idx2, "anim_in", null);
        mm.parameters.anim_out = luascript_helper_get_field_string(L, idx2, "anim_out", null);
        mm.parameters.anim_transition_in_delay = luascript_helper_get_field_number(L, idx2, "anim_transition_in_delay", 0.0);
        mm.parameters.anim_transition_out_delay = luascript_helper_get_field_number(L, idx2, "anim_transition_out_delay", 0.0);
        mm.parameters.is_vertical = luascript_helper_get_field_boolean(L, idx2, "is_vertical", false);
        mm.parameters.items_align = luascript_helper_get_field_align(L, idx2, "items_align", ALIGN_CENTER);
        mm.parameters.items_gap = luascript_helper_get_field_number(L, idx2, "items_gap", 0.0);
        mm.parameters.is_sparse = luascript_helper_get_field_boolean(L, idx2, "is_sparse", false);
        mm.parameters.static_index = luascript_helper_get_field_integer(L, idx2, "static_index", 0);
        mm.parameters.is_per_page = luascript_helper_get_field_boolean(L, idx2, "is_per_page", false);
        mm.parameters.items_dimmen = luascript_helper_get_field_number(L, idx2, "items_dimmen", 0.0);
        mm.parameters.font_border_size = luascript_helper_get_field_number(L, idx2, "font_border_size", 0.0);
        mm.parameters.texture_scale = luascript_helper_get_field_number(L, idx2, "texture_scale", 0.0);
        mm.parameters.enable_horizontal_text_correction = luascript_helper_get_field_boolean(L, idx2, "enable_horizontal_text_correction", false);
        mm.parameters.suffix_selected = luascript_helper_get_field_string(L, idx2, "suffix_selected", null);
        mm.parameters.suffix_choosen = luascript_helper_get_field_string(L, idx2, "suffix_choosen", null);
        mm.parameters.suffix_discarded = luascript_helper_get_field_string(L, idx2, "suffix_discarded", null);
        mm.parameters.suffix_idle = luascript_helper_get_field_string(L, idx2, "suffix_idle", null);
        mm.parameters.suffix_rollback = luascript_helper_get_field_string(L, idx2, "suffix_rollback", null);
        mm.parameters.suffix_in = luascript_helper_get_field_string(L, idx2, "suffix_in", null);
        mm.parameters.suffix_out = luascript_helper_get_field_string(L, idx2, "suffix_out", null);
        LUA.lua_pop(L, 1);
    }

    if (luascript_helper_get_field_table(L, idx, "items")) {
        let idx2 = LUA.lua_gettop(L);
        let length = LUA.lua_rawlen(L, idx2);

        mm.items_size = length;
        mm.items = new Array(length);

        for (let i = 0; i < length; i++) {
            if (luascript_helper_get_array_item_as_table(L, idx2, i + 1)) {
                let idx3 = LUA.lua_gettop(L);
                let idx4 = -1;

                if (luascript_helper_get_field_table(L, idx3, "placement")) {
                    idx4 = LUA.lua_gettop(L);
                    mm.items[i].placement.gap = luascript_helper_get_field_number(L, idx4, "gap", NaN);
                    mm.items[i].placement.x = luascript_helper_get_field_number(L, idx4, "x", NaN);
                    mm.items[i].placement.y = luascript_helper_get_field_number(L, idx4, "y", NaN);
                    mm.items[i].placement.dimmen = luascript_helper_get_field_number(L, idx4, "dimmen", NaN);
                    LUA.lua_pop(L, 1);
                } else {
                    mm.items[i].placement.gap = NaN;
                    mm.items[i].placement.x = NaN;
                    mm.items[i].placement.y = NaN;
                    mm.items[i].placement.dimmen = NaN;
                }

                idx4 = LUA.lua_gettop(L);
                mm.items[i].has_font_color = luascript_helper_has_table_field(L, idx4, "font_color", LUA.LUA_TNUMBER);
                mm.items[i].font_color = luascript_helper_get_field_integer(L, idx4, "font_color", 0x000000);
                mm.items[i].text = luascript_helper_get_field_string(L, idx4, "text", null);
                mm.items[i].model = luascript_helper_get_field_string(L, idx4, "model", null);
                mm.items[i].texture_scale = luascript_helper_get_field_number(L, idx4, "texture_scale", NaN);
                mm.items[i].name = luascript_helper_get_field_string(L, idx4, "name", null);
                mm.items[i].anim_selected = luascript_helper_get_field_string(L, idx4, "anim_selected", null);
                mm.items[i].anim_choosen = luascript_helper_get_field_string(L, idx4, "anim_choosen", null);
                mm.items[i].anim_discarded = luascript_helper_get_field_string(L, idx4, "anim_discarded", null);
                mm.items[i].anim_idle = luascript_helper_get_field_string(L, idx4, "anim_idle", null);
                mm.items[i].anim_rollback = luascript_helper_get_field_string(L, idx4, "anim_rollback", null);
                mm.items[i].anim_in = luascript_helper_get_field_string(L, idx4, "anim_in", null);
                mm.items[i].anim_out = luascript_helper_get_field_string(L, idx4, "anim_out", null);
                mm.items[i].hidden = luascript_helper_get_field_boolean(L, idx4, "hidden", false);
                mm.items[i].description = luascript_helper_get_field_string(L, idx4, "description", null);

                LUA.lua_pop(L, 1);
            }
        }
        LUA.lua_pop(L, 1);
    }

    return mm;
}


async function script_menu_init(L) {
    let menumanifest = luascript_helper_table_to_menumanifest(L, 2);
    let x = LUA.luaL_checknumber(L, 3);
    let y = LUA.luaL_checknumber(L, 4);
    let z = LUA.luaL_checknumber(L, 5);
    let width = LUA.luaL_checknumber(L, 6);
    let height = LUA.luaL_checknumber(L, 7);

    if (!menumanifest) {
        return LUA.luaL_error(L, "menumanifest argument can not be nil (null)");
    }

    if (menumanifest.items_size < 1) {
        menumanifest_destroy(menumanifest);
        return LUA.luaL_error(L, "menumanifest does not contains items");
    }

    let ret = await menu_init(menumanifest, x, y, z, width, height);
    menumanifest_destroy(menumanifest);

    return luascript_userdata_allocnew(L, MENU, ret);
}

/*
// warning: do not use, this can lead to exploiting Drawable function pointers
function script_menu_destroy(L) {
    let menu = luascript_read_userdata(L, MENU);

    if (luascript_userdata_is_allocated(L, MENU))
        menu_destroy(menu);
    else
        console.error("script_menu_destroy() object was not allocated by lua");

    return 0;
}*/

function script_menu_get_drawable(L) {
    let menu = luascript_read_userdata(L, MENU);

    let ret = menu_get_drawable(menu);

    return script_drawable_new(L, ret);
}

function script_menu_trasition_in(L) {
    let menu = luascript_read_userdata(L, MENU);

    menu_trasition_in(menu);

    return 0;
}

function script_menu_trasition_out(L) {
    let menu = luascript_read_userdata(L, MENU);

    menu_trasition_out(menu);

    return 0;
}

function script_menu_select_item(L) {
    let menu = luascript_read_userdata(L, MENU);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = menu_select_item(menu, name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_select_index(L) {
    let menu = luascript_read_userdata(L, MENU);
    let index = LUA.luaL_checkinteger(L, 2);

    menu_select_index(menu, index);

    return 0;
}

function script_menu_select_vertical(L) {
    let menu = luascript_read_userdata(L, MENU);
    let offset = LUA.luaL_checkinteger(L, 2);

    let ret = menu_select_vertical(menu, offset);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_select_horizontal(L) {
    let menu = luascript_read_userdata(L, MENU);
    let offset = LUA.luaL_checkinteger(L, 2);

    let ret = menu_select_horizontal(menu, offset);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_toggle_choosen(L) {
    let menu = luascript_read_userdata(L, MENU);
    let enable = LUA.lua_toboolean(L, 2);

    menu_toggle_choosen(menu, enable);

    return 0;
}

function script_menu_get_selected_index(L) {
    let menu = luascript_read_userdata(L, MENU);

    let ret = menu_get_selected_index(menu);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_menu_get_items_count(L) {
    let menu = luascript_read_userdata(L, MENU);

    let ret = menu_get_items_count(menu);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_menu_set_item_text(L) {
    let menu = luascript_read_userdata(L, MENU);
    let index = LUA.luaL_checkinteger(L, 2);
    let text = LUA.luaL_optstring(L, 3, null);

    let ret = menu_set_item_text(menu, index, text);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_set_item_visibility(L) {
    let menu = luascript_read_userdata(L, MENU);
    let index = LUA.luaL_checkinteger(L, 2);
    let visible = LUA.lua_toboolean(L, 3);

    let ret = menu_set_item_visibility(menu, index, visible);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_get_item_rect(L) {
    let menu = luascript_read_userdata(L, MENU);
    let index = LUA.luaL_checkinteger(L, 2);
    const output_location = [0.0, 0.0], output_size = [0.0, 0.0];

    let ret = menu_get_item_rect(menu, index, output_location, output_size);

    if (!ret) {
        LUA.lua_pushnil(L);
        return 1;
    }

    LUA.lua_pushnumber(L, output_location[0]);
    LUA.lua_pushnumber(L, output_location[1]);
    LUA.lua_pushnumber(L, output_size[0]);
    LUA.lua_pushnumber(L, output_size[1]);

    return 4;
}

function script_menu_get_selected_item_name(L) {
    let menu = luascript_read_userdata(L, MENU);

    let ret = menu_get_selected_item_name(menu);

    LUA.lua_pushstring(L, ret);

    return 1;
}

function script_menu_set_text_force_case(L) {
    let menu = luascript_read_userdata(L, MENU);

    let none_or_lowercase_or_uppercase = luascript_helper_optenum(
        L,
        2,
        LUASCRIPT_ENUMS_TextSpriteForceCase,
    );

    menu_set_text_force_case(menu, none_or_lowercase_or_uppercase);

    return 0;
}

function script_menu_has_item(L) {
    let menu = luascript_read_userdata(L, MENU);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = menu_has_item(menu, name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_menu_index_of_item(L) {
    let menu = luascript_read_userdata(L, MENU);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = menu_index_of_item(menu, name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const MENU_FUNCTIONS = [
    ["init", script_menu_init],
    //["destroy", script_menu_destroy],
    ["get_drawable", script_menu_get_drawable],
    ["trasition_in", script_menu_trasition_in],
    ["trasition_out", script_menu_trasition_out],
    ["select_item", script_menu_select_item],
    ["select_index", script_menu_select_index],
    ["select_vertical", script_menu_select_vertical],
    ["select_horizontal", script_menu_select_horizontal],
    ["toggle_choosen", script_menu_toggle_choosen],
    ["get_selected_index", script_menu_get_selected_index],
    ["get_items_count", script_menu_get_items_count],
    ["set_item_text", script_menu_set_item_text],
    ["set_item_visibility", script_menu_set_item_visibility],
    ["get_item_rect", script_menu_get_item_rect],
    ["get_selected_item_name", script_menu_get_selected_item_name],
    ["set_text_force_case", script_menu_set_text_force_case],
    ["has_item", script_menu_has_item],
    ["index_of_item", script_menu_index_of_item],
    [null, null]
];

function script_menu_new(L, menu) {
    return luascript_userdata_new(L, MENU, menu);
}

function script_menu_gc(L) {
    /*
    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, MENU, menu_destroy);
    */
    return luascript_userdata_gc(L, MENU);
}

function script_menu_tostring(L) {
    return luascript_userdata_tostring(L, MENU);
}

function script_menu_register(L) {
    luascript_register(L, MENU, script_menu_gc, script_menu_tostring, MENU_FUNCTIONS);
}
