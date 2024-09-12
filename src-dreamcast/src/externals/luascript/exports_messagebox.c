#include "luascript_internal.h"

#include "game/common/messagebox.h"


static int script_messagebox_set_buttons_text(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* left_text = luaL_optstring(L, 2, NULL);
    const char* right_text = luaL_optstring(L, 3, NULL);

    messagebox_set_buttons_text(messagebox, left_text, right_text);

    return 0;
}

static int script_messagebox_set_button_single(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* center_text = luaL_optstring(L, 2, NULL);

    messagebox_set_button_single(messagebox, center_text);

    return 0;
}

static int script_messagebox_set_buttons_icons(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* left_icon_name = luaL_optstring(L, 2, NULL);
    const char* right_icon_name = luaL_optstring(L, 3, NULL);

    messagebox_set_buttons_icons(messagebox, left_icon_name, right_icon_name);

    return 0;
}

static int script_messagebox_set_button_single_icon(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* center_icon_name = luaL_optstring(L, 2, NULL);

    messagebox_set_button_single_icon(messagebox, center_icon_name);

    return 0;
}

static int script_messagebox_set_title(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* text = luaL_optstring(L, 2, NULL);

    messagebox_set_title(messagebox, text);

    return 0;
}

static int script_messagebox_set_image_background_color(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    int64_t number = luaL_checkinteger(L, 2);
    int32_t color_rgb8 = (int32_t)number;

    messagebox_set_image_background_color(messagebox, color_rgb8);

    return 0;
}

static int script_messagebox_set_image_background_color_default(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    messagebox_set_image_background_color_default(messagebox);

    return 0;
}

static int script_messagebox_set_message(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* text = luaL_optstring(L, 2, NULL);

    messagebox_set_message(messagebox, text);

    return 0;
}

static int script_messagebox_hide_image_background(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool hide = lua_toboolean(L, 2);

    messagebox_hide_image_background(messagebox, hide);

    return 0;
}

static int script_messagebox_hide_image(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool hide = lua_toboolean(L, 2);

    messagebox_hide_image(messagebox, hide);

    return 0;
}

static int script_messagebox_show_buttons_icons(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool show = lua_toboolean(L, 2);

    messagebox_show_buttons_icons(messagebox, show);

    return 0;
}

static int script_messagebox_use_small_size(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool small_or_normal = lua_toboolean(L, 2);

    messagebox_use_small_size(messagebox, small_or_normal);

    return 0;
}

static int script_messagebox_set_image_from_texture(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    luascript_change_working_folder(L);
    const char* filename = luaL_optstring(L, 2, NULL);
    luascript_restore_working_folder(L);

    messagebox_set_image_from_texture(messagebox, filename);

    return 0;
}

static int script_messagebox_set_image_from_atlas(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    const char* filename = luaL_optstring(L, 2, NULL);
    const char* entry_name = luaL_optstring(L, 3, NULL);
    bool is_animation = luaL_optstring(L, 4, NULL);

    luascript_change_working_folder(L);
    messagebox_set_image_from_atlas(messagebox, filename, entry_name, is_animation);
    luascript_restore_working_folder(L);

    return 0;
}

static int script_messagebox_hide(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool animated = lua_toboolean(L, 2);

    messagebox_hide(messagebox, animated);

    return 0;
}

static int script_messagebox_show(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    bool animated = lua_toboolean(L, 2);

    messagebox_show(messagebox, animated);

    return 0;
}

static int script_messagebox_set_z_index(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    float z_index = (float)luaL_checknumber(L, 2);

    messagebox_set_z_index(messagebox, z_index);

    return 0;
}

static int script_messagebox_get_modifier(lua_State* L) {
    MessageBox messagebox = luascript_read_userdata(L, MESSAGEBOX);

    Modifier* modifier = messagebox_get_modifier(messagebox);

    return script_modifier_new(L, modifier);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg MESSAGEBOX_FUNCTIONS[] = {
    {"set_buttons_text", script_messagebox_set_buttons_text},
    {"set_button_single", script_messagebox_set_button_single},
    {"set_buttons_icons", script_messagebox_set_buttons_icons},
    {"set_button_single_icon", script_messagebox_set_button_single_icon},
    {"set_title", script_messagebox_set_title},
    {"set_image_background_color", script_messagebox_set_image_background_color},
    {"set_image_background_color_default", script_messagebox_set_image_background_color_default},
    {"set_message", script_messagebox_set_message},
    {"hide_image_background", script_messagebox_hide_image_background},
    {"hide_image", script_messagebox_hide_image},
    {"show_buttons_icons", script_messagebox_show_buttons_icons},
    {"use_small_size", script_messagebox_use_small_size},
    {"set_image_from_texture", script_messagebox_set_image_from_texture},
    {"set_image_from_atlas", script_messagebox_set_image_from_atlas},
    {"hide", script_messagebox_hide},
    {"show", script_messagebox_show},
    {"set_z_index", script_messagebox_set_z_index},
    {"get_modifier", script_messagebox_get_modifier},
    {NULL, NULL}
};

int script_messagebox_new(lua_State* L, MessageBox messagebox) {
    return luascript_userdata_new(L, MESSAGEBOX, messagebox);
}

static int script_messagebox_gc(lua_State* L) {
    return luascript_userdata_gc(L, MESSAGEBOX);
}

static int script_messagebox_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, MESSAGEBOX);
}

void script_messagebox_register(lua_State* L) {
    luascript_register(L, MESSAGEBOX, script_messagebox_gc, script_messagebox_tostring, MESSAGEBOX_FUNCTIONS);
}
