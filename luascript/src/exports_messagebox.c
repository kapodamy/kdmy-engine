#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(void, messagebox_set_buttons_text, (Messagebox messagebox, const char* left_text, const char* right_text), {
    messagebox_set_buttons_text(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(left_text), kdmyEngine_ptrToString(right_text));
});
EM_JS_PRFX(void, messagebox_set_button_single, (Messagebox messagebox, const char* center_text), {
    messagebox_set_button_single(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(center_text));
});
EM_JS_PRFX(void, messagebox_set_buttons_icons, (Messagebox messagebox, const char* left_icon_name, const char* right_icon_name), {
    messagebox_set_buttons_icons(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(left_icon_name), kdmyEngine_ptrToString(right_icon_name));
});
EM_JS_PRFX(void, messagebox_set_button_single_icon, (Messagebox messagebox, const char* center_icon_name), {
    messagebox_set_button_single_icon(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(center_icon_name));
});
EM_JS_PRFX(void, messagebox_set_title, (Messagebox messagebox, const char* text), {
    messagebox_set_title(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(text));
});
EM_JS_PRFX(void, messagebox_set_image_background_color, (Messagebox messagebox, uint32_t color_rgb8), {
    messagebox_set_image_background_color(kdmyEngine_obtain(messagebox), color_rgb8);
});
EM_JS_PRFX(void, messagebox_set_image_background_color_default, (Messagebox messagebox), {
    messagebox_set_image_background_color_default(kdmyEngine_obtain(messagebox));
});
EM_JS_PRFX(void, messagebox_set_message, (Messagebox messagebox, const char* text), {
    messagebox_set_message(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(text));
});
EM_JS_PRFX(void, messagebox_hide_image_background, (Messagebox messagebox, bool hide), {
    messagebox_hide_image_background(kdmyEngine_obtain(messagebox), hide);
});
EM_JS_PRFX(void, messagebox_hide_image, (Messagebox messagebox, bool hide), {
    messagebox_hide_image(kdmyEngine_obtain(messagebox), hide);
});
EM_JS_PRFX(void, messagebox_show_buttons_icons, (Messagebox messagebox, bool show), {
    messagebox_show_buttons_icons(kdmyEngine_obtain(messagebox), show);
});
EM_JS_PRFX(void, messagebox_use_small_size, (Messagebox messagebox, bool small_or_normal), {
    messagebox_use_small_size(kdmyEngine_obtain(messagebox), small_or_normal);
});
EM_JS_PRFX(void, messagebox_set_image_sprite, (Messagebox messagebox, Sprite sprite), {
    messagebox_set_image_sprite(kdmyEngine_obtain(messagebox), sprite);
});
EM_ASYNC_JS_PRFX(void, messagebox_set_image_from_texture, (Messagebox messagebox, const char* filename), {
    await messagebox_set_image_from_texture(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(filename));
});
EM_ASYNC_JS_PRFX(void, messagebox_set_image_from_atlas, (Messagebox messagebox, const char* filename, const char* entry_name, bool is_animation), {
    await messagebox_set_image_from_atlas(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(filename), kdmyEngine_ptrToString(entry_name), is_animation);
});
EM_JS_PRFX(void, messagebox_hide, (Messagebox messagebox, bool animated), {
    messagebox_hide(kdmyEngine_obtain(messagebox), animated);
});
EM_JS_PRFX(void, messagebox_show, (Messagebox messagebox, bool animated), {
    messagebox_show(kdmyEngine_obtain(messagebox), animated);
});
EM_JS_PRFX(void, messagebox_set_z_index, (Messagebox messagebox, float z_index), {
    messagebox_set_z_index(kdmyEngine_obtain(messagebox), z_index);
});
EM_JS_PRFX(Modifier, messagebox_get_modifier, (Messagebox messagebox), {
    const modifier = messagebox_get_modifier(kdmyEngine_obtain(messagebox));
    return kdmyEngine_obtain(modifier);
});
#endif



static int script_messagebox_set_buttons_text(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* left_text = luaL_optstring(L, 2, NULL);
    const char* right_text = luaL_optstring(L, 3, NULL);

    messagebox_set_buttons_text(messagebox, left_text, right_text);

    return 0;
}

static int script_messagebox_set_button_single(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* center_text = luaL_optstring(L, 2, NULL);

    messagebox_set_button_single(messagebox, center_text);

    return 0;
}

static int script_messagebox_set_buttons_icons(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* left_icon_name = luaL_optstring(L, 2, NULL);
    const char* right_icon_name = luaL_optstring(L, 3, NULL);

    messagebox_set_buttons_icons(messagebox, left_icon_name, right_icon_name);

    return 0;
}

static int script_messagebox_set_button_single_icon(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* center_icon_name = luaL_optstring(L, 2, NULL);

    messagebox_set_button_single_icon(messagebox, center_icon_name);

    return 0;
}

static int script_messagebox_set_title(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* text = luaL_optstring(L, 2, NULL);

    messagebox_set_title(messagebox, text);

    return 0;
}

static int script_messagebox_set_image_background_color(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    int64_t number = luaL_checkinteger(L, 2);
    int32_t color_rgb8 = (int32_t)number;

    messagebox_set_image_background_color(messagebox, color_rgb8);

    return 0;
}

static int script_messagebox_set_image_background_color_default(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    messagebox_set_image_background_color_default(messagebox);

    return 0;
}

static int script_messagebox_set_message(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* text = luaL_optstring(L, 2, NULL);

    messagebox_set_message(messagebox, text);

    return 0;
}

static int script_messagebox_hide_image_background(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool hide = luaL_checkboolean(L, 2);

    messagebox_hide_image_background(messagebox, hide);

    return 0;
}

static int script_messagebox_hide_image(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool hide = luaL_checkboolean(L, 2);

    messagebox_hide_image(messagebox, hide);

    return 0;
}

static int script_messagebox_show_buttons_icons(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool show = luaL_checkboolean(L, 2);

    messagebox_show_buttons_icons(messagebox, show);

    return 0;
}

static int script_messagebox_use_small_size(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool small_or_normal = luaL_checkboolean(L, 2);

    messagebox_use_small_size(messagebox, small_or_normal);

    return 0;
}

static int script_messagebox_set_image_from_texture(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* filename = luaL_optstring(L, 2, NULL);

    messagebox_set_image_from_texture(messagebox, filename);

    return 0;
}

static int script_messagebox_set_image_from_atlas(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    const char* filename = luaL_optstring(L, 2, NULL);
    const char* entry_name = luaL_optstring(L, 3, NULL);
    bool is_animation = luaL_optstring(L, 4, NULL);

    messagebox_set_image_from_atlas(messagebox, filename, entry_name, is_animation);

    return 0;
}

static int script_messagebox_hide(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool animated = luaL_checkboolean(L, 2);

    messagebox_hide(messagebox, animated);

    return 0;
}

static int script_messagebox_show(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    bool animated = luaL_checkboolean(L, 2);

    messagebox_show(messagebox, animated);

    return 0;
}

static int script_messagebox_set_z_index(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    float z_index = luaL_checkfloat(L, 2);

    messagebox_set_z_index(messagebox, z_index);

    return 0;
}

static int script_messagebox_get_modifier(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);

    Modifier modifier = messagebox_get_modifier(messagebox);

    return script_modifier_new(L, messagebox, modifier);
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

int script_messagebox_new(lua_State* L, Messagebox messagebox) {
    return NEW_USERDATA(L, MESSAGEBOX, NULL, messagebox, true);
}

static int script_messagebox_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, Messagebox, messagebox, MESSAGEBOX);
    _luascript_suppress_item(L, messagebox, true);
    return 0;
}

static int script_messagebox_tostring(lua_State* L) {
    READ_USERDATA(L, Messagebox, messagebox, MESSAGEBOX);
    lua_pushstring(L, "[MessageBox]");
    return 1;
}


inline void register_messagebox(lua_State* L) {
    _luascript_register(
        L,
        MESSAGEBOX,
        script_messagebox_gc,
        script_messagebox_tostring,
        MESSAGEBOX_FUNCTIONS
    );
}

