#ifndef __luascript_internal_h
#define __luascript_internal_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "timer.h"

#include "linkedlist.h"

#include "luascript.h"

#include "soundplayer.h"
#include "songplayer.h"
#include "layout.h"
#include "camera.h"
#include "sprite.h"
#include "textsprite.h"
#include "character.h"
#include "modifier.h"
#include "messagebox.h"
#include "pvrctx.h"
#include "fs.h"
#include "dialogue.h"
#include "psshader.h"

static const char* ALIGN_NONE_STRING = "NONE";
static const char* ALIGN_BOTH_STRING = "BOTH";
static const char* ALIGN_CENTER_STRING = "CENTER";
static const char* ALIGN_START_STRING = "START";
static const char* ALIGN_END_STRING = "END";

#define luaL_checkboolean(L, n)        lua_toboolean(L, n)
#define luaL_checkfloat(L, arg)        (float)luaL_checknumber(L, arg)
#define luaL_optionalfloat(L, idx)     (float)luaL_optnumber(L, idx, NAN)


#define READ_USERDATA(L, T, V, N)   if (lua_isnil(L, 1)) return luaL_error(L, "%s was null (nil in lua).", N); \
                                    T V = (T)(*((void**)luaL_checkudata(L, 1, N))); \
                                    check_shared_userdata(L, V);
#define READ_USERDATA_UNCHECKED(L, T, V, N)   if (lua_isnil(L, 1)) luaL_error(L, "%s was null (nil in lua).", N); \
                                    T V = (T)(*((void**)luaL_checkudata(L, 1, N)));

#define LUASCRIPT_SET(L, value_ptr)     lua_pushlightuserdata(L, (void *)luascript_drop_shared); \
                                        lua_pushlightuserdata(L, (void *)value_ptr); \
                                        lua_settable(L, LUA_REGISTRYINDEX);

#define LUASCRIPT_GET(L)    lua_pushlightuserdata(L, (void *)luascript_drop_shared); \
                            lua_gettable(L, LUA_REGISTRYINDEX); \
                            Luascript luascript = lua_touserdata(L, -1); \
                            lua_pop(L, 1);

#define LUA_THROW_ERROR(L, message)  lua_pushliteral(L, message); \
                                    return lua_error(L);

#define NULLIFY_USERDATA(L)  { \
                                    void** ptr = (void**)lua_touserdata(L, 1); \
                                    if (ptr) *ptr = NULL; \
                                };

#define LUA_TOUSERDATA_OR_NULL(L, IDX, TYPE, NAME) ( \
                                                        lua_isnil(L, IDX) ? \
                                                        NULL \
                                                        : \
                                                        ((TYPE)*((void**)luaL_checkudata(L, IDX, NAME))) \
                                                    )

void _luascript_declare_item(lua_State* lua, void* parent_ptr, void* object_ptr, bool is_shared);
void _luascript_suppress_item(lua_State* lua, void* object_ptr, bool is_shared);
int _luascript_has_item(lua_State* lua, void* object_ptr, bool is_shared);
void* _luascript_get_parent(lua_State* lua, void* object_ptr, bool is_shared);
void _luascript_register(lua_State* lua, const char* name, lua_CFunction gc, lua_CFunction tostring, const luaL_Reg fns[]);
int _parse_align(lua_State* L, const char* align);
const char* _get_align(Align align);
int _parse_pvrflag(lua_State* L, const char* pvrflag);

#ifdef JAVASCRIPT
#include <emscripten.h>
#define EM_JS_PRFX(ret, name, params, ...) _EM_JS(ret, name, __js__##name, params, #__VA_ARGS__)

#define EM_ASYNC_JS_PRFX(ret, name, params, ...) _EM_JS(ret, name, __asyncjs__##name, params,          \
  "{ return Asyncify.handleAsync(async () => " #__VA_ARGS__ "); }")

#endif

static inline int NEW_USERDATA(lua_State* L, const char* type, void* parent_ptr, void* object_ptr, bool shared) {
    if (object_ptr == NULL) {
        lua_pushnil(L);
        return 1;
    }

    void** ptr = (void**)lua_newuserdata(L, sizeof(void**));
    *ptr = object_ptr;

    luaL_getmetatable(L, type);
    lua_setmetatable(L, -2);

    _luascript_declare_item(L, parent_ptr, object_ptr, shared);
    return 1;
}

static inline void check_shared_userdata(lua_State* L, void* ptr) {
    if (ptr && (_luascript_has_item(L, ptr, true) || _luascript_has_item(L, ptr, false))) return;
    luaL_error(L, "object destroyed");
}

#define TEXTSPRITE "TextSprite"
int script_textsprite_new(lua_State* L, Layout layout, TextSprite textsprite);
void register_textsprite(lua_State* L);

#define SPRITE "Sprite"
int script_sprite_new(lua_State* L, Layout layout, Sprite sprite);
void register_sprite(lua_State* L);

#define LAYOUT "Layout"
int script_layout_new(lua_State* L, Layout layout);
void register_layout(lua_State* L);

#define CAMERA "Camera"
int script_camera_new(lua_State* L, Layout layout, Camera camera);
void register_camera(lua_State* L);

#define SOUNDPLAYER "SoundPlayer"
int script_soundplayer_new(lua_State* L, Layout layout, SoundPlayer soundplayer);
void register_soundplayer(lua_State* L);

#define SONGPLAYER "SongPlayer"
int script_songplayer_new(lua_State* L, SongPlayer soundplayer);
void register_songplayer(lua_State* L);

#define MODIFIER "Modifier"
int script_modifier_new(lua_State* L, void* vertex, Modifier modifier);
void register_modifier(lua_State* L);

#define CHARACTER "Character"
int script_character_new(lua_State* L, Character character);
void register_character(lua_State* L);

#define MESSAGEBOX "Messagebox"
int script_messagebox_new(lua_State* L, Messagebox messagebox);
void register_messagebox(lua_State* L);

//#define WEEK "Week"
//int script_week_new(lua_State* L);
void register_week(lua_State* L);

//#define MATH2D "Math2D"
//int script_math2d_new(lua_State* L);
void register_math2d(lua_State* L);

//#define TIMER "Timer"
//int script_timer_new(lua_State* L);
void register_timer(lua_State* L);

//#define FILESYSTEM "Filesystem"
//int script_fs_new(lua_State* L);
void register_fs(lua_State* L);

#define ANIMSPRITE "AnimSprite"
#define MODELHOLDER "ModelHolder"

//#define MODDING "Modding"
// int script_modding_new(lua_State* L);
void register_modding(lua_State* L);

#define DIALOGUE "Dialogue"
int script_dialogue_new(lua_State* L, Dialogue dialogue);
void register_dialogue(lua_State* L);

#define PSSHADER "PSShader"
int script_psshader_new(lua_State* L, void* vertex, PSShader psshader);
void register_psshader(lua_State* L);

#endif

