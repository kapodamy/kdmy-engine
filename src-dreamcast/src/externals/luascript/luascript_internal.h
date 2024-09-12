#ifndef _luascript_internal_h
#define _luascript_internal_h

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lua/lauxlib.h>
#include <lua/lua.h>

#include "game/modding_types.h"
#include "nbool.h"



#define LUASCRIPT_SHARED_ARRAY_CHUNK_SIZE 16


//
//  forward references
//

typedef struct AtlasEntry_s AtlasEntry;
typedef struct AnimListItem_s AnimListItem;
typedef struct Modifier_s Modifier;
typedef struct LayoutPlaceholder_s LayoutPlaceholder;
typedef struct EngineSettings_s EngineSettings;
typedef struct TextSprite_s* TextSprite;
typedef struct Sprite_s* Sprite;
typedef struct Camera_s* Camera;
typedef struct VideoPlayer_s* VideoPlayer;
typedef struct SongPlayer_s* SongPlayer;
typedef struct Character_s* Character;
typedef struct Dialogue_s* Dialogue;
typedef struct PSShader_s* PSShader;
typedef struct TweenLerp_s* TweenLerp;
typedef struct TweenKeyframe_s* TweenKeyframe;
typedef struct Atlas_s* Atlas;
typedef struct AnimList_s* AnimList;
typedef struct AnimSprite_s* AnimSprite;
typedef struct Drawable_s* Drawable;
typedef struct ModelHolder_s* ModelHolder;
typedef struct Strum_s* Strum;
typedef struct Strums_s* Strums;
typedef struct Conductor_s* Conductor;
typedef struct Countdown_s* Countdown;
typedef struct PlayerStats_s* PlayerStats;
typedef struct StreakCounter_s* StreakCounter;
typedef struct RoundStats_s* RoundStats;
typedef struct RankingCounter_s* RankingCounter;
typedef struct MissNoteFX_s* MissNoteFX;
typedef struct Healthbar_s* Healthbar;
typedef struct HealthWatcher_s* HealthWatcher;
typedef struct SongProgressbar_s* SongProgressbar;
typedef struct Layout_s* Layout;
typedef struct SoundPlayer_s* SoundPlayer;
typedef struct MessageBox_s* MessageBox;
typedef struct Menu_s* Menu;

typedef struct Luascript_s* Luascript;
typedef struct LuascriptEnums_s* LuascriptEnums;
typedef void (*Destructor)(void**);

typedef struct LuascriptObject_s {
    int lua_ref;
    void* obj_ptr;
    bool was_allocated_by_lua;
} LuascriptObject;

struct Luascript_s {
    void* L;
    void* context;
    char* working_folder;
    int32_t id;
    bool is_week;
    LuascriptObject* shared_array;
    size_t shared_size;
};


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

void* luascript_parse_and_allocate_modding_value(lua_State* L, int idx, ModdingValueType* output_type, bool throw_error);
int luascript_push_modding_value(lua_State* L, bool deallocate, ModdingValueType type, void* value);

void luascript_change_working_folder(lua_State* L);
void luascript_restore_working_folder(lua_State* L);

int luascript_pcallk(lua_State* L, int arguments_count, int results_count);

int script_atlas_push_atlas_entry(lua_State* L, const AtlasEntry* entry);

const char* luascript_helper_enums_stringify(const LuascriptEnums source, const int32_t value);
int32_t luascript_helper_optenum(lua_State* L, int idx, const LuascriptEnums source);
void luascript_helper_pushenum(lua_State* L, const LuascriptEnums source, const int32_t value);

void luascript_helper_add_field_integer(lua_State* L, const char* name, const lua_Integer integer);
void luascript_helper_add_field_string(lua_State* L, const char* name, const char* string);
void luascript_helper_add_field_number(lua_State* L, const char* name, const lua_Number number);
void luascript_helper_add_field_boolean(lua_State* L, const char* name, const bool boolean);
void luascript_helper_add_field_null(lua_State* L, const char* name);
void luascript_helper_add_field_table(lua_State* L, const char* name, const int32_t narr, const int32_t nrec);
void luascript_helper_add_field_enum(lua_State* L, const char* name, const LuascriptEnums source, const int32_t value);
void luascript_helper_add_field_array_item_as_table(lua_State* L, const int32_t narr, const int32_t nrec, const int32_t index_in_table);

lua_Integer luascript_helper_get_field_integer(lua_State* L, const int32_t idx, const char* name, const lua_Integer def_value);
char* luascript_helper_get_field_string(lua_State* L, const int32_t idx, const char* name, const char* def_value);
int32_t luascript_helper_get_field_align(lua_State* L, const int32_t idx, const char* name, const int32_t def_value);
lua_Number luascript_helper_get_field_number(lua_State* L, const int32_t idx, const char* name, const lua_Number def_value);
bool luascript_helper_get_field_boolean(lua_State* L, const int32_t idx, const char* name, const bool def_value);
bool luascript_helper_get_field_table(lua_State* L, const int32_t idx, const char* name);
bool luascript_helper_get_array_item_as_table(lua_State* L, const int32_t idx, const int32_t index_in_table);
bool luascript_helper_has_table_field(lua_State* L, const int32_t idx, const char* name, const int desired_type);

nbool luascript_helper_optnbool(lua_State* L, int idx);
void luascript_helper_pushnbool(lua_State* L, nbool value);

#define luascript_helper_add_table_field(L, field_name, setter, value) \
    {                                                                  \
        lua_pushstring(L, field_name);                                 \
        setter(L, value);                                              \
        lua_settable(L, -3);                                           \
    }


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
int script_layoutplaceholder_new(lua_State* L, LayoutPlaceholder* placeholderlayout);
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
int script_modifier_new(lua_State* L, Modifier* modifier);
void script_modifier_register(lua_State* L);

#define CHARACTER "Character"
int script_character_new(lua_State* L, Character character);
void script_character_register(lua_State* L);

#define MESSAGEBOX "MessageBox"
int script_messagebox_new(lua_State* L, MessageBox messagebox);
void script_messagebox_register(lua_State* L);

// #define WEEK "Week"
//  int script_week_new(lua_State* L);
void script_week_register(lua_State* L);

// #define MATH2D "Math2D"
//  int script_math2d_new(lua_State* L);
void script_math2d_register(lua_State* L);

// #define TIMER "Timer"
//  int script_timer_new(lua_State* L);
void script_timer_register(lua_State* L);

// #define FILESYSTEM "Filesystem"
//  int script_fs_new(lua_State* L);
void script_fs_register(lua_State* L);

// #define SCRIPT "Script"
//  int script_script_new(lua_State* L);
void script_script_register(lua_State* L);


// #define MODDING "Modding"
//  int script_modding_new(lua_State* L);
void script_modding_register(lua_State* L);

// #define MENUMANIFEST "MenuManifest"
//  int script_menumanifest_new(lua_State* L);
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
int script_animlistitem_new(lua_State* L, const AnimListItem* animlistitem);
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

#define HEALTHBAR "Healthbar"
int script_healthbar_new(lua_State* L, Healthbar healthbar);
void script_healthbar_register(lua_State* L);

#define HEALTHWATCHER "HealthWatcher"
int script_healthwatcher_new(lua_State* L, HealthWatcher healthwatcher);
void script_healthwatcher_register(lua_State* L);

#define SONGPROGRESSBAR "SongProgressbar"
int script_songprogressbar_new(lua_State* L, SongProgressbar songprogressbar);
void script_songprogressbar_register(lua_State* L);

#define ENGINESETTINGS "EngineSettings"
int script_enginesettings_new(lua_State* L, EngineSettings* enginesettings);
void script_enginesettings_register(lua_State* L);

#define ENVIRONMENT "Environment"
// int script_environment_new(lua_State* L);
void script_environment_register(lua_State* L);

#define MENU "Menu"
int script_menu_new(lua_State* L, Menu menu);
void script_menu_register(lua_State* L);

#define JSON "JSON"
// int script_json_new(lua_State* L);
void script_json_register(lua_State* L);

#define PRELOADCACHE "PreloadCache"
// int script_preloadcache_new(lua_State* L);
void script_preloadcache_register(lua_State* L);

#endif
