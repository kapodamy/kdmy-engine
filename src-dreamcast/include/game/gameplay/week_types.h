#ifndef _week_types_h
#define _week_types_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"
#include "game/common/weekenumerator.h"
#include "game/ddrkeymon.h"
#include "game/gameplay/helpers/chart.h"
#include "vertexprops_types.h"


//
//  forward references
//

typedef struct Luascript_s* WeekScript;
typedef struct LinkedList_s* LinkedList;
typedef struct Conductor_s* Conductor;
typedef struct Character_s* Character;
typedef struct PlayerStats_s* PlayerStats;
typedef struct Strums_s* Strums;
typedef struct NotePool_s* NotePool;
typedef struct FontHolder_s* FontHolder;
typedef struct AnimList_s* AnimList;
typedef struct GameplayManifest_s* GameplayManifest;
typedef struct RankingCounter_s* RankingCounter;
typedef struct StreakCounter_s* StreakCounter;
typedef struct Healthbar_s* Healthbar;
typedef struct RoundStats_s* RoundStats;
typedef struct TextSprite_s* TextSprite;
typedef struct Countdown_s* Countdown;
typedef struct WeekGameOver_s* WeekGameOver;
typedef struct WeekPause_s* WeekPause;
typedef struct WeekResult_s* WeekResult;
typedef struct MessageBox_s* MessageBox;
typedef struct Camera_s* Camera;
typedef struct MissNoteFX_s* MissNoteFX;
typedef struct SongProgressbar_s* SongProgressbar;
typedef struct AutoUICosmetics_s* AutoUICosmetics;
typedef struct SongPlayer_s* SongPlayer;
typedef struct HealthWatcher_s* HealthWatcher;
typedef struct Dialogue_s* Dialogue;
typedef struct Layout_s* Layout;
typedef struct Sprite_s* Sprite;
typedef struct LuascriptValue_s* LuascriptValueCollection;
typedef struct Drawable_s* Drawable;
typedef struct WeekResult_Stats_s WeekResult_Stats;


typedef enum {
    CharacterType_ACTOR,
    CharacterType_BOT,
    CharacterType_PLAYER
} CharacterType;

typedef struct {
    char* name;
    float64 value;
    bool completed_week;
    bool completed_round;
    bool create;
} ModifiedDirective;

typedef struct {
    bool halt_flag;
    LinkedList /*ModifiedDirective*/ directives;
    bool force_end_flag;
    bool force_end_round_or_week;
    bool force_end_loose_or_win;
    bool no_week_end_result_screen;
} ScriptContext;

typedef struct {
    Conductor conductor;
    Character character;
    PlayerStats playerstats;
    CharacterType type;
    Strums strums;
    DDRKeymon ddrkeymon;
    DDRKeysFIFO* ddrkeys_fifo;
    Gamepad controller;
    NotePool notepool;

    bool is_vertical;
    bool is_opponent;
    bool can_die;
    bool can_recover;
} PlayerStruct;

typedef struct {
    bool girlfriend_cry;
    bool ask_ready;
    bool do_countdown;
    bool camera_bumping;

    float bpm;
    float64 speed;

    float original_bpm;
    float64 original_speed;

    char* camera_name_opponent;
    char* camera_name_player;

    bool layout_rollback;
    bool show_credits;
    bool no_healthbar;
} Settings;

typedef struct {
    float healthbar_x;
    float healthbar_y;
    float healthbar_z;
    float healthbar_length;
    float healthbar_dimmen;
    float healthbar_border;
    float healthbar_iconoverlap;
    float healthbar_warnheight;
    float healthbar_lockheight;
    bool healthbar_is_vertical;
    bool healthbar_nowarns;
    bool roundstats_hide;
    float roundstats_x;
    float roundstats_y;
    float roundstats_z;
    float roundstats_fontsize;
    float roundstats_fontbordersize;
    uint32_t roundstats_fontcolor;
    float streakcounter_comboheight;
    float streakcounter_numbergap;
    float streakcounter_delay;
    bool rankingcounter_percentonly;
    float songinfo_x;
    float songinfo_y;
    float songinfo_z;
    float songinfo_maxwidth;
    float songinfo_maxheight;
    Align songinfo_alignvertical;
    Align songinfo_alignhorinzontal;
    float songinfo_fontsize;
    float songinfo_fontbordersize;
    uint32_t songinfo_fontcolor;
    float countdown_height;
    float songprogressbar_x;
    float songprogressbar_y;
    float songprogressbar_z;
    float songprogressbar_width;
    float songprogressbar_height;
    Align songprogressbar_align;
    float songprogressbar_bordersize;
    float songprogressbar_fontsize;
    float songprogressbar_fontbordersize;
    bool songprogressbar_isvertical;
    bool songprogressbar_showtime;
    uint32_t songprogressbar_colorrgba8_text;
    uint32_t songprogressbar_colorrgba8_background;
    uint32_t songprogressbar_colorrgba8_barback;
    uint32_t songprogressbar_colorrgba8_barfront;
} UIParams;

typedef struct {
    float x;
    float y;
    float z;
    float length;
    float gap;
    bool keep_marker_scale;
    float marker_dimmen;
    float invdimmen;
    float sustain_alpha;
    bool is_vertical;
} LayoutStrum;

typedef struct {
    float x;
    float y;
    float z;
    float scale;
    Align align_vertical;
    Align align_horizontal;
    float reference_width;
    float reference_height;
    int32_t placeholder_id;
} LayoutCharacter;

typedef struct {
    LayoutStrum* layout_strums;
    int32_t layout_strums_size;

    LayoutCharacter layout_girlfriend;
    LayoutCharacter* layout_characters;
    int32_t layout_characters_size;

    FontHolder font;
    float ui_layout_width;
    float ui_layout_height;
    AnimList animlist;
    GameplayManifest gameplaymanifest;

    UIParams ui;

    bool alt_tracks;
    const char* difficult;
    const char* default_boyfriend;
    const char* default_girlfriend;
    bool single_song;

    WeekInfo* weekinfo;
} InitParams;

typedef struct {
    char* opponent_icon_model;
    char* player_icon_model;

    bool has_player_color;
    uint32_t player_color_rgb8;

    bool has_opponent_color;
    uint32_t opponent_color_rgb8;
} HealthBarParams;

typedef struct RoundContext_s {
    RankingCounter rankingcounter;
    StreakCounter streakcounter;
    Healthbar healthbar;
    RoundStats roundstats;
    TextSprite songinfo;
    Countdown countdown;
    WeekGameOver weekgameover;
    WeekPause weekpause;
    WeekResult weekresult;
    MessageBox messagebox;
    Camera ui_camera;
    MissNoteFX missnotefx;
    SongProgressbar songprogressbar;
    AutoUICosmetics autouicosmetics;

    Layout layout;

    SongPlayer songplayer;
    WeekScript script;
    int32_t playerstats_index;
    HealthWatcher healthwatcher;
    Settings settings;

    Character girlfriend;
    PlayerStruct* players;
    int32_t players_size;

    int32_t song_index;
    const char* song_difficult;
    float64 round_duration;

    ChartEventEntry* events;
    int32_t events_size;
    int32_t events_peek_index;

    HealthBarParams healthbarparams;

    ScriptContext scriptcontext;
    Dialogue dialogue;

    bool girlfriend_from_default;
    bool healthbar_from_default;
    bool stage_from_default;
    bool script_from_default;
    bool dialogue_from_default;
    uint32_t weekgameover_from_version;
    bool ui_from_default;
    bool pause_menu_from_default;

    bool players_from_default;
    bool distributions_from_default;

    Layout ui_layout;
    Sprite screen_background;

    bool has_directive_changes;
    int32_t resolution_changes;

    InitParams initparams;
}* RoundContext;


#endif
