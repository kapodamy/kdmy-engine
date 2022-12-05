#ifndef __luascript_internal_h
#define __luascript_internal_h

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "timer.h"

#include "linkedlist.h"

#include "luascript.h"

#include "camera.h"
#include "character.h"
#include "dialogue.h"
#include "fs.h"
#include "layout.h"
#include "messagebox.h"
#include "modifier.h"
#include "psshader.h"
#include "pvrctx.h"
#include "songplayer.h"
#include "soundplayer.h"
#include "sprite.h"
#include "textsprite.h"


#ifdef JAVASCRIPT
#include <emscripten.h>

#define EM_JS_PRFX(ret, name, params, ...) \
    _EM_JS(ret, name, __js__##name, params, #__VA_ARGS__)

#define EM_ASYNC_JS_PRFX(ret, name, params, ...) \
    _EM_JS(ret, name, __asyncjs__##name, params, "{ return Asyncify.handleAsync(async () => " #__VA_ARGS__ "); }")

#endif

typedef void (*Destructor)(void*);

#define SHARED_ARRAY_CHUNK_SIZE 16


int luascript_create_userdata(Luascript luascript, void* obj, const char* metatable_name, bool allocated);
void luascript_remove_userdata(Luascript luascript, void* obj);

void* luascript_read_userdata(lua_State* L, const char* check_metatable_name);
void* luascript_read_nullable_userdata(lua_State* L, int idx, const char* check_metatable_name);

Luascript luascript_get_instance(lua_State* L);
void luascript_set_instance(Luascript luascript);

int luascript_userdata_new(lua_State* L, const char* check_metatable_name, void* obj);
int luascript_userdata_allocnew(lua_State* L, const char* check_metatable_name, void* obj);
int luascript_userdata_tostring(lua_State* L, const char* check_metatable_name);
int luascript_userdata_gc(lua_State* L, const char* check_metatable_name);
int luascript_userdata_destroy(lua_State* L, const char* check_metatable_name, Destructor destructor);
bool luascript_userdata_is_allocated(lua_State* L, const char* check_metatable_name);

void luascript_register(lua_State* lua, const char* name, const lua_CFunction gc, const lua_CFunction tostring, const luaL_Reg fns[]);

int luascript_parse_align(lua_State* L, const char* align);
int luascript_parse_pvrflag(lua_State* L, const char* pvrflag);
int luascript_parse_forcecase(lua_State* L, const char* forcecase);
int luascript_parse_wordbreak(lua_State* L, const char* wordbreak);
int luascript_parse_interpolator(lua_State* L, const char* interpolator);

const char* luascript_stringify_align(Align align);
const char* luascript_stringify_actiontype(CharacterActionType actiontype);


#define TEXTSPRITE "TextSprite"
int script_textsprite_new(lua_State* L, TextSprite textsprite);
void script_textsprite_register(lua_State* L);

#define SPRITE "Sprite"
int script_sprite_new(lua_State* L, Sprite sprite);
void script_sprite_register(lua_State* L);

#define LAYOUT "Layout"
int script_layout_new(lua_State* L, Layout layout);
void script_layout_register(lua_State* L);

#define CAMERA "Camera"
int script_camera_new(lua_State* L, Camera camera);
void script_camera_register(lua_State* L);

#define SOUNDPLAYER "SoundPlayer"
int script_soundplayer_new(lua_State* L, SoundPlayer soundplayer);
void script_soundplayer_register(lua_State* L);

#define SONGPLAYER "SongPlayer"
int script_songplayer_new(lua_State* L, SongPlayer soundplayer);
void script_songplayer_register(lua_State* L);

#define MODIFIER "Modifier"
int script_modifier_new(lua_State* L, Modifier modifier);
void script_modifier_register(lua_State* L);

#define CHARACTER "Character"
int script_character_new(lua_State* L, Character character);
void script_character_register(lua_State* L);

#define MESSAGEBOX "Messagebox"
int script_messagebox_new(lua_State* L, Messagebox messagebox);
void script_messagebox_register(lua_State* L);

//#define WEEK "Week"
// int script_week_new(lua_State* L);
void script_week_register(lua_State* L);

//#define MATH2D "Math2D"
// int script_math2d_new(lua_State* L);
void script_math2d_register(lua_State* L);

//#define TIMER "Timer"
// int script_timer_new(lua_State* L);
void script_timer_register(lua_State* L);

//#define FILESYSTEM "Filesystem"
// int script_fs_new(lua_State* L);
void script_fs_register(lua_State* L);

#define ANIMSPRITE "AnimSprite"
#define MODELHOLDER "ModelHolder"

//#define MODDING "Modding"
// int script_modding_new(lua_State* L);
void script_modding_register(lua_State* L);

#define DIALOGUE "Dialogue"
int script_dialogue_new(lua_State* L, Dialogue dialogue);
void script_dialogue_register(lua_State* L);

#define PSSHADER "PSShader"
int script_psshader_new(lua_State* L, PSShader psshader);
void script_psshader_register(lua_State* L);

#endif
