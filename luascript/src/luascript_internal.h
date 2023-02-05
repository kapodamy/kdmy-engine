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
#include "tweenlerp.h"
#include "tweenkeyframe.h"
#include "atlas.h"
#include "drawable.h"
#include "animsprite.h"
#include "strum.h"
#include "strums.h"
#include "conductor.h"
#include "countdown.h"
#include "playerstats.h"
#include "streakcounter.h"
#include "roundstats.h"
#include "rankingcounter.h"
#include "missnotefx.h"
#include "healthbar.h"
#include "healthwatcher.h"
#include "songprogressbar.h"
#include "modding.h"
#include "week.h"
#include "engine_settings.h"
#include "menu.h"
#include "menumanifest.h"

#ifdef JAVASCRIPT
#include <emscripten.h>

#define EM_JS_PRFX(ret, name, params, ...) \
    _EM_JS(ret, name, __js__##name, params, #__VA_ARGS__)

#define EM_ASYNC_JS_PRFX(ret, name, params, ...) \
    _EM_JS(ret, name, __asyncjs__##name, params, "{ return Asyncify.handleAsync(async () => " #__VA_ARGS__ "); }")


double kdmy_read_prop_double(void* obj_id, const char* field_name);
float kdmy_read_prop_float(void* obj_id, const char* field_name);
char* kdmy_read_prop_string(void* obj_id, const char* field_name);
int32_t kdmy_read_prop_integer(void* obj_id, const char* field_name);
void* kdmy_read_prop_object(void* obj_id, const char* field_name);
bool kdmy_read_prop_boolean(void* obj_id, const char* field_name);
bool kdmy_read_prop_floatboolean(void* obj_id, const char* field_name);
void kdmy_forget_obtained(void* obj_id);

void* kdmy_create_object();
void* kdmy_create_array(int32_t size);
void* kdmy_read_array_item_object(void* array_id, int32_t index);

void* kdmy_read_window_object(const char* variable_name);

void kdmy_write_prop_double(void* obj_id, const char* field_name, double value);
void kdmy_write_prop_float(void* obj_id, const char* field_name, float value);
void kdmy_write_prop_string(void* obj_id, const char* field_name, char* value);
void kdmy_write_prop_integer(void* obj_id, const char* field_name, int32_t value);
void kdmy_write_prop_object(void* obj_id, const char* field_name, void* value);
void kdmy_write_prop_boolean(void* obj_id, const char* field_name, bool value);

void kdmy_write_in_array_double(void* array_id, int32_t index, double value);
void kdmy_write_in_array_float(void* array_id, int32_t index, float value);
void kdmy_write_in_array_string(void* array_id, int32_t index, char* value);
void kdmy_write_in_array_integer(void* array_id, int32_t index, int32_t value);
void kdmy_write_in_array_object(void* array_id, int32_t index, void* value);
void kdmy_write_in_array_boolean(void* array_id, int32_t index, bool value);

#endif

typedef void (*Destructor)(void**);

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

Align luascript_parse_align(lua_State* L, const char* align);
Align luascript_parse_align2(const char* align);
int luascript_parse_pvrflag(lua_State* L, const char* pvrflag);
int luascript_parse_forcecase(lua_State* L, const char* forcecase);
int luascript_parse_wordbreak(lua_State* L, const char* wordbreak);
int luascript_parse_interpolator(lua_State* L, const char* interpolator);
Blend luascript_parse_blend(lua_State* L, const char* blend);
StrumScriptTarget luascript_parse_strumscripttarget(lua_State* L, const char* strumscripttarget);
StrumScriptOn luascript_parse_strumscripton(lua_State* L, const char* strumscripton);
ScrollDirection luascript_parse_scrolldirection(lua_State* L, const char* scrolldirection);

const char* luascript_stringify_align(Align align);
const char* luascript_stringify_actiontype(CharacterActionType actiontype);

char* luascript_get_string_copy(lua_State* L, int idx, const char* def);
char* luascript_get_string_copy2(const char* def);

void* luascript_parse_and_allocate_modding_value(lua_State* L, int idx, ModdingValueType* output_type, bool throw_error);
int luascript_push_and_deallocate_modding_value(lua_State* L, ModdingValueType type, void* value);

int luascript_pcallk(lua_State* L, int arguments_count, int results_count);

int script_atlas_push_atlas_entry(lua_State* L, AtlasEntry entry);

#define LuaL_add_table_field(L, field_name, lua_function_setter, value) \
    lua_pushstring(L, (field_name));                         \
    lua_function_setter(L, (value));                                    \
    lua_settable(L, -3);


#define TEXTSPRITE "TextSprite"
int script_textsprite_new(lua_State* L, TextSprite textsprite);
void script_textsprite_register(lua_State* L);

#define SPRITE "Sprite"
int script_sprite_new(lua_State* L, Sprite sprite);
void script_sprite_register(lua_State* L);

#define LAYOUT "Layout"
int script_layout_new(lua_State* L, Layout layout);
void script_layout_register(lua_State* L);

#define LAYOUTPLACEHOLDER "LayoutPlaceholder"
int script_layoutplaceholder_new(lua_State* L, LayoutPlaceholder placeholderlayout);
void script_layoutplaceholder_register(lua_State* L);

#define CAMERA "Camera"
int script_camera_new(lua_State* L, Camera camera);
void script_camera_register(lua_State* L);

#define SOUNDPLAYER "SoundPlayer"
int script_soundplayer_new(lua_State* L, SoundPlayer soundplayer);
void script_soundplayer_register(lua_State* L);

#define VIDEOPLAYER "VideoPlayer"
int script_videoplayer_new(lua_State* L, VideoPlayer videoplayer);
void script_videoplayer_register(lua_State* L);

#define SONGPLAYER "SongPlayer"
int script_songplayer_new(lua_State* L, SongPlayer soundplayer);
void script_songplayer_register(lua_State* L);

#define MODIFIER "Modifier"
int script_modifier_new(lua_State* L, Modifier modifier);
void script_modifier_register(lua_State* L);

#define CHARACTER "Character"
int script_character_new(lua_State* L, Character character);
void script_character_register(lua_State* L);

#define MESSAGEBOX "MessageBox"
int script_messagebox_new(lua_State* L, MessageBox messagebox);
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

//#define SCRIPT "Script"
// int script_script_new(lua_State* L);
void script_script_register(lua_State* L);


//#define MODDING "Modding"
// int script_modding_new(lua_State* L);
void script_modding_register(lua_State* L);

//#define MENUMANIFEST "MenuManifest"
// int script_menumanifest_new(lua_State* L);
void script_menumanifest_register(lua_State* L);

#define DIALOGUE "Dialogue"
int script_dialogue_new(lua_State* L, Dialogue dialogue);
void script_dialogue_register(lua_State* L);

#define PSSHADER "PSShader"
int script_psshader_new(lua_State* L, PSShader psshader);
void script_psshader_register(lua_State* L);

#define TWEENLERP "TweenLerp"
int script_tweenlerp_new(lua_State* L, TweenLerp tweenlerp);
void script_tweenlerp_register(lua_State* L);

#define TWEENKEYFRAME "TweenKeyframe"
int script_tweenkeyframe_new(lua_State* L, TweenKeyframe tweenkeyframe);
void script_tweenkeyframe_register(lua_State* L);

#define ATLAS "Atlas"
int script_atlas_new(lua_State* L, Atlas atlas);
void script_atlas_register(lua_State* L);

#define ANIMLIST "AnimList"
int script_animlist_new(lua_State* L, AnimList animlist);
void script_animlist_register(lua_State* L);

#define ANIMLISTITEM "AnimListItem"
int script_animlistitem_new(lua_State* L, AnimListItem animlistitem);
void script_animlistitem_register(lua_State* L);

#define ANIMSPRITE "AnimSprite"
int script_animsprite_new(lua_State* L, AnimSprite animsprite);
void script_animsprite_register(lua_State* L);

#define DRAWABLE "Drawable"
int script_drawable_new(lua_State* L, Drawable drawable);
void script_drawable_register(lua_State* L);

#define MODELHOLDER "ModelHolder"
int script_modelholder_new(lua_State* L, ModelHolder modelholder);
void script_modelholder_register(lua_State* L);

#define STRUM "Strum"
int script_strum_new(lua_State* L, Strum strum);
void script_strum_register(lua_State* L);

#define STRUMS "Strums"
int script_strums_new(lua_State* L, Strums strums);
void script_strums_register(lua_State* L);

#define CONDUCTOR "Conductor"
int script_conductor_new(lua_State* L, Conductor conductor);
void script_conductor_register(lua_State* L);

#define COUNTDOWN "Countdown"
int script_countdown_new(lua_State* L, Countdown countdown);
void script_countdown_register(lua_State* L);

#define PLAYERSTATS "PlayerStats"
int script_playerstats_new(lua_State* L, PlayerStats playerstats);
void script_playerstats_register(lua_State* L);

#define STREAKCOUNTER "StreakCounter"
int script_streakcounter_new(lua_State* L, StreakCounter streakcounter);
void script_streakcounter_register(lua_State* L);

#define ROUNDSTATS "RoundStats"
int script_roundstats_new(lua_State* L, RoundStats roundstats);
void script_roundstats_register(lua_State* L);

#define RANKINGCOUNTER "RankingCounter"
int script_rankingcounter_new(lua_State* L, RankingCounter rankingcounter);
void script_rankingcounter_register(lua_State* L);

#define MISSNOTEFX "MissNoteFX"
int script_missnotefx_new(lua_State* L, MissNoteFX missnotefx);
void script_healthbar_register(lua_State* L);

#define HEALTHBAR "HealthBar"
int script_healthbar_new(lua_State* L, HealthBar healthbar);
void script_healthbar_register(lua_State* L);

#define HEALTHWATCHER "HealthWatcher"
int script_healthwatcher_new(lua_State* L, HealthWatcher healthwatcher);
void script_healthwatcher_register(lua_State* L);

#define SONGPROGRESSBAR "SongProgressbar"
int script_songprogressbar_new(lua_State* L, SongProgressbar songprogressbar);
void script_songprogressbar_register(lua_State* L);

#define ENGINESETTINGS "EngineSettings"
int script_enginesettings_new(lua_State* L, EngineSettings_t* enginesettings);
void script_enginesettings_register(lua_State* L);

#define ENVIRONMENT "Environment"
//int script_environment_new(lua_State* L);
void script_environment_register(lua_State* L);

#define MENU "Menu"
int script_menu_new(lua_State* L, Menu menu);
void script_menu_register(lua_State* L);

#endif
