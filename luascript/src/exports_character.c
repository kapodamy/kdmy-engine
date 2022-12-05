#include "luascript_internal.h"

#include "animsprite.h"
#include "character.h"
#include "modifier.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(void, character_use_alternate_sing_animations, (Character character, bool enable), {
    character_use_alternate_sing_animations(kdmyEngine_obtain(character), enable);
});
EM_JS_PRFX(void, character_set_draw_location, (Character character, float x, float y), {
    character_set_draw_location(kdmyEngine_obtain(character), x, y);
});
EM_JS_PRFX(void, character_set_draw_align, (Character character, Align align_vertical, Align align_horizontal), {
    character_set_draw_align(kdmyEngine_obtain(character), align_vertical, align_horizontal);
});
EM_JS_PRFX(void, character_update_reference_size, (Character character, float width, float height), {
    character_update_reference_size(kdmyEngine_obtain(character), width, height);
});
EM_JS_PRFX(void, character_enable_reference_size, (Character character, bool enable), {
    character_enable_reference_size(kdmyEngine_obtain(character), enable);
});
EM_JS_PRFX(void, character_set_offset, (Character character, float offset_x, float offset_y), {
    character_set_offset(kdmyEngine_obtain(character), offset_x, offset_y);
});
EM_JS_PRFX(int32_t, character_state_add, (Character character, ModelHolder modelholder, const char* state_name), {
    return character_state_add(kdmyEngine_obtain(character), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(state_name));
});
EM_JS_PRFX(int32_t, character_state_toggle, (Character character, const char* state_name), {
    return character_state_toggle(kdmyEngine_obtain(character), kdmyEngine_ptrToString(state_name));
});
EM_JS_PRFX(bool, character_play_hey, (Character character), {
    return character_play_hey(kdmyEngine_obtain(character));
});
EM_JS_PRFX(bool, character_play_idle, (Character character), {
    return character_play_idle(kdmyEngine_obtain(character));
});
EM_JS_PRFX(bool, character_play_sing, (Character character, const char* direction, bool prefer_sustain), {
    return character_play_sing(kdmyEngine_obtain(character), kdmyEngine_ptrToString(direction), prefer_sustain);
});
EM_JS_PRFX(bool, character_play_miss, (Character character, const char* direction, bool keep_in_hold), {
    return character_play_miss(kdmyEngine_obtain(character), kdmyEngine_ptrToString(direction), keep_in_hold);
});
EM_JS_PRFX(bool, character_play_extra, (Character character, const char* extra_animation_name, bool prefer_sustain), {
    return character_play_extra(kdmyEngine_obtain(character), kdmyEngine_ptrToString(extra_animation_name), prefer_sustain);
});
EM_JS_PRFX(void, character_set_idle_speed, (Character character, float speed), {
    character_set_idle_speed(kdmyEngine_obtain(character), speed);
});
EM_JS_PRFX(void, character_set_scale, (Character character, float scale_factor), {
    character_set_scale(kdmyEngine_obtain(character), scale_factor);
});
EM_JS_PRFX(void, character_reset, (Character character), {
    character_reset(kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, character_enable_continuous_idle, (Character character, bool enable), {
    character_enable_continuous_idle(kdmyEngine_obtain(character), enable);
});
EM_JS_PRFX(bool, character_is_idle_active, (Character character), {
    return character_is_idle_active(kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, character_enable_flip_correction, (Character character, bool enable), {
    return character_enable_flip_correction(kdmyEngine_obtain(character), enable);
});
EM_JS_PRFX(void, character_flip_orientation, (Character character, bool enable), {
    return character_flip_orientation(kdmyEngine_obtain(character), enable);
});
EM_JS_PRFX(void, character_face_as_opponent, (Character character, bool face_as_opponent), {
    character_face_as_opponent(kdmyEngine_obtain(character), face_as_opponent);
});
EM_JS_PRFX(void, character_set_z_index, (Character character, float z), {
    character_set_z_index(kdmyEngine_obtain(character), z);
});
EM_JS_PRFX(void, character_set_z_offset, (Character character, float z_offset), {
    character_set_z_offset(kdmyEngine_obtain(character), z_offset);
});
EM_JS_PRFX(void, character_animation_set, (Character character, const AnimSprite animsprite), {
    character_animation_set(kdmyEngine_obtain(character), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, character_animation_restart, (Character character), {
    character_animation_restart(kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, character_animation_end, (Character character), {
    character_animation_end(kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, character_set_color_offset, (Character character, float r, float g, float b, float a), {
    character_set_color_offset(kdmyEngine_obtain(character), r, g, b, a);
});
EM_JS_PRFX(void, character_set_color_offset_to_default, (Character character), {
    character_set_color_offset_to_default(kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, character_set_alpha, (Character character, float alpha), {
    character_set_alpha(kdmyEngine_obtain(character), alpha);
});
EM_JS_PRFX(void, character_set_visible, (Character character, bool visible), {
    character_set_visible(kdmyEngine_obtain(character), visible);
});
EM_JS_PRFX(Modifier, character_get_modifier, (Character character), {
    const modifier = character_get_modifier(kdmyEngine_obtain(character));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(bool, character_has_direction, (Character character, const char* name, bool is_extra), {
    return character_has_direction(kdmyEngine_obtain(character), kdmyEngine_ptrToString(name), is_extra);
});
EM_JS_PRFX(int32_t, character_get_play_calls, (Character character), {
    return character_get_play_calls(kdmyEngine_obtain(character));
});
EM_JS_PRFX(CharacterActionType, character_get_current_action, (Character character), {
    return character_get_current_action(kdmyEngine_obtain(character));
});
#endif



static int script_character_use_alternate_sing_animations(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enable = lua_toboolean(L, 2);

    character_use_alternate_sing_animations(character, enable);

    return 0;
}

static int script_character_set_draw_location(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    character_set_draw_location(character, x, y);

    return 0;
}

static int script_character_set_draw_align(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* str1 = luaL_optstring(L, 2, NULL);
    const char* str2 = luaL_optstring(L, 3, NULL);

    Align align_vertical = luascript_parse_align(L, str1);
    Align align_horizontal = luascript_parse_align(L, str2);

    character_set_draw_align(character, align_vertical, align_horizontal);

    return 0;
}

static int script_character_update_reference_size(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float width = (float)luaL_optnumber(L, 2, NAN);
    float height = (float)luaL_optnumber(L, 3, NAN);

    character_update_reference_size(character, width, height);

    return 0;
}

static int script_character_enable_reference_size(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enable = lua_toboolean(L, 2);

    character_enable_reference_size(character, enable);

    return 0;
}

static int script_character_set_offset(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float offset_x = (float)luaL_checknumber(L, 2);
    float offset_y = (float)luaL_checknumber(L, 3);

    character_set_offset(character, offset_x, offset_y);

    return 0;
}

static int script_character_state_add(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    ModelHolder modelholder = (ModelHolder)luaL_testudata(L, 2, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 3, NULL);

    bool ret = character_state_add(character, modelholder, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_state_toggle(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = character_state_toggle(character, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_play_hey(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool ret = character_play_hey(character);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_play_idle(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    int32_t ret = character_play_idle(character);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_character_play_sing(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* direction = luaL_optstring(L, 2, NULL);
    bool prefer_sustain = lua_toboolean(L, 3);

    bool ret = character_play_sing(character, direction, prefer_sustain);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_play_miss(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* direction = luaL_optstring(L, 2, NULL);
    bool keep_in_hold = lua_toboolean(L, 3);

    bool ret = character_play_miss(character, direction, keep_in_hold);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_character_play_extra(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* extra_animation_name = luaL_optstring(L, 2, NULL);
    bool prefer_sustain = lua_toboolean(L, 3);

    bool ret = character_play_extra(character, extra_animation_name, prefer_sustain);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_set_idle_speed(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float speed = (float)luaL_checknumber(L, 2);

    character_set_idle_speed(character, speed);

    return 0;
}

static int script_character_set_scale(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float scale_factor = (float)luaL_checknumber(L, 2);

    character_set_scale(character, scale_factor);

    return 0;
}

static int script_character_reset(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    character_reset(character);

    return 0;
}

static int script_character_enable_continuous_idle(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enable = lua_toboolean(L, 2);

    character_enable_continuous_idle(character, enable);

    return 0;
}

static int script_character_is_idle_active(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool ret = character_is_idle_active(character);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_enable_flip_correction(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enable = lua_toboolean(L, 2);

    character_enable_flip_correction(character, enable);

    return 0;
}

static int script_character_flip_orientation(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enable = lua_toboolean(L, 2);

    character_flip_orientation(character, enable);

    return 0;
}

static int script_character_face_as_opponent(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool face_as_opponent = lua_toboolean(L, 2);

    character_face_as_opponent(character, face_as_opponent);

    return 0;
}

static int script_character_set_z_index(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float z = (float)luaL_checknumber(L, 2);

    character_set_z_index(character, z);

    return 0;
}

static int script_character_set_z_offset(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float z_offset = (float)luaL_checknumber(L, 2);

    character_set_z_offset(character, z_offset);

    return 0;
}

static int script_character_animation_set(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    AnimSprite animsprite = (AnimSprite)luaL_testudata(L, 2, ANIMSPRITE);

    character_animation_set(character, animsprite);

    return 0;
}

static int script_character_animation_restart(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    character_animation_restart(character);

    return 0;
}

static int script_character_animation_end(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    character_animation_end(character);

    return 0;
}

static int script_character_set_color_offset(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float r = (float)luaL_optnumber(L, 2, NAN);
    float g = (float)luaL_optnumber(L, 3, NAN);
    float b = (float)luaL_optnumber(L, 4, NAN);
    float a = (float)luaL_optnumber(L, 5, NAN);

    character_set_color_offset(character, r, g, b, a);

    return 0;
}

static int script_character_set_color_offset_to_default(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    character_set_color_offset_to_default(character);

    return 0;
}

static int script_character_set_alpha(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float alpha = (float)luaL_checknumber(L, 2);

    character_set_alpha(character, alpha);

    return 0;
}

static int script_character_set_visible(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool visible = lua_toboolean(L, 2);

    character_set_visible(character, visible);

    return 0;
}

static int script_character_get_modifier(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    Modifier modifier = character_get_modifier(character);

    return script_modifier_new(L, modifier);
}

static int script_character_has_direction(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    const char* name = luaL_optstring(L, 2, NULL);
    bool is_extra = lua_toboolean(L, 3);

    bool ret = character_has_direction(character, name, is_extra);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_character_get_play_calls(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    int32_t ret = character_get_play_calls(character);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_character_get_current_action(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    CharacterActionType ret = character_get_current_action(character);

    lua_pushstring(L, luascript_stringify_actiontype(ret));
    return 1;
}



////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg CHARACTER_FUNCTIONS[] = {
    {"use_alternate_sing_animations", script_character_use_alternate_sing_animations},
    {"set_draw_location", script_character_set_draw_location},
    {"set_draw_align", script_character_set_draw_align},
    {"update_reference_size", script_character_update_reference_size},
    {"enable_reference_size", script_character_enable_reference_size},
    {"set_offset", script_character_set_offset},
    {"state_add", script_character_state_add},
    {"state_toggle", script_character_state_toggle},
    {"play_hey", script_character_play_hey},
    {"play_idle", script_character_play_idle},
    {"play_sing", script_character_play_sing},
    {"play_miss", script_character_play_miss},
    {"play_extra", script_character_play_extra},
    {"set_idle_speed", script_character_set_idle_speed},
    {"set_scale", script_character_set_scale},
    {"reset", script_character_reset},
    {"enable_continuous_idle", script_character_enable_continuous_idle},
    {"is_idle_active", script_character_is_idle_active},
    {"enable_flip_correction", script_character_enable_flip_correction},
    {"flip_orientation", script_character_flip_orientation},
    {"face_as_opponent", script_character_face_as_opponent},
    {"set_z_index", script_character_set_z_index},
    {"set_z_offset", script_character_set_z_offset},
    {"animation_set", script_character_animation_set},
    {"animation_restart", script_character_animation_restart},
    {"animation_end", script_character_animation_end},
    {"set_color_offset", script_character_set_color_offset},
    {"set_color_offset_to_default", script_character_set_color_offset_to_default},
    {"set_alpha", script_character_set_alpha},
    {"set_visible", script_character_set_visible},
    {"get_modifier", script_character_get_modifier},
    {"has_direction", script_character_has_direction},
    {"get_play_calls", script_character_get_play_calls},
    {"get_current_action", script_character_get_current_action},
    {NULL, NULL}
};

int script_character_new(lua_State* L, Character character) {
    return luascript_userdata_new(L, CHARACTER, character);
}

static int script_character_gc(lua_State* L) {
    return luascript_userdata_gc(L, CHARACTER);
}

static int script_character_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, CHARACTER);
}


void script_character_register(lua_State* L) {
    luascript_register(L, CHARACTER, script_character_gc, script_character_tostring, CHARACTER_FUNCTIONS);
}
