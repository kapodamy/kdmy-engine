#include "luascript_internal.h"

#include "game/funkin/character.h"
#include "luascript_enums.h"
#include "math2d.h"


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

    Align align_vertical = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);
    Align align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);

    character_set_draw_align(character, align_vertical, align_horizontal);

    return 0;
}

static int script_character_update_reference_size(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    float width = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float height = (float)luaL_optnumber(L, 3, FLOAT_NaN);

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

    lua_pushinteger(L, (lua_Integer)ret);
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

    lua_pushboolean(L, ret);
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

    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    float a = (float)luaL_optnumber(L, 5, FLOAT_NaN);

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

    Modifier* modifier = character_get_modifier(character);

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

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_character_get_commited_animations_count(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    int32_t ret = character_get_commited_animations_count(character);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_character_get_current_action(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    CharacterActionType ret = character_get_current_action(character);

    luascript_helper_pushenum(L, LUASCRIPT_ENUMS_CharacterActionType, ret);
    return 1;
}

static int script_character_freeze_animation(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    bool enabled = lua_toboolean(L, 2);

    character_freeze_animation(character, enabled);

    return 0;
}

static int script_character_trailing_enabled(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);
    bool enabled = (bool)lua_toboolean(L, 2);

    character_trailing_enabled(character, enabled);

    return 0;
}

static int script_character_trailing_set_params(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);
    int32_t length = (int32_t)luaL_optinteger(L, 2, -1);
    float trail_delay = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float trail_alpha = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    nbool darken_colors = luascript_helper_optnbool(L, 5);

    character_trailing_set_params(character, length, trail_delay, trail_alpha, darken_colors);

    return 0;
}

static int script_character_trailing_set_offsetcolor(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);
    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);

    character_trailing_set_offsetcolor(character, r, g, b);

    return 0;
}

static int script_character_schedule_idle(lua_State* L) {
    Character character = luascript_read_userdata(L, CHARACTER);

    character_schedule_idle(character);

    return 0;
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
    {"get_commited_animations_count", script_character_get_commited_animations_count},
    {"get_current_action", script_character_get_current_action},
    {"freeze_animation", script_character_freeze_animation},
    {"trailing_enabled", script_character_trailing_enabled},
    {"trailing_set_params", script_character_trailing_set_params},
    {"trailing_set_offsetcolor", script_character_trailing_set_offsetcolor},
    {"schedule_idle", script_character_schedule_idle},
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
