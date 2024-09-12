#ifndef _gameplaymanifest_h
#define _gameplaymanifest_h


#include "float64.h"
#include "game/funkin/strums.h"


typedef enum {
    GameplayManifestRefer_NONE,
    GameplayManifestRefer_GIRLFRIEND,
    GameplayManifestRefer_BOYFRIEND
} GameplayManifestRefer;

typedef struct {
    char* name;
    char* model;
} GameplayManifestCharacterState;

typedef struct {
    char* manifest;
    GameplayManifestCharacterState* states;
    int32_t states_size;
    GameplayManifestRefer refer;
    int32_t controller;
    int32_t layout_strums_id;
    int32_t distribution_index;
    bool can_die;
    bool can_recover;
    bool is_opponent;
} GameplayManifestPlayer;

typedef struct {
    GameplayManifestRefer refer;
    char* manifest;
    GameplayManifestCharacterState* states;
    int32_t states_size;
} GameplayManifestGirlfriend;

typedef struct {
    char* icon_model;
    char* bar_model;
    uint32_t bar_color;
} GameplayManifestHealthBarStateEntry;

typedef struct {
    char* name;
    GameplayManifestHealthBarStateEntry opponent;
    GameplayManifestHealthBarStateEntry player;
    GameplayManifestHealthBarStateEntry background;
} GameplayManifestHealthBarState;

typedef struct {
    bool has_disable;
    bool disable;
    bool has_use_alt_icons;
    bool use_alt_icons;
    bool has_opponent_color;
    uint32_t opponent_color_rgb8;
    bool has_player_color;
    uint32_t player_color_rgb8;
    GameplayManifestHealthBarState* states;
    int32_t states_size;
    bool has_warnings_model;
    char* warnings_model;
} GameplayManifestHealthBar;

typedef struct {
    char* name;
    char* file;
    char* chart;
    char* stage;
    bool has_stage;
    char* script;
    bool has_script;
    float64 duration;
    char* selected_state_name;
    bool has_selected_state_name;
    char* ui_layout;
    bool has_ui_layout;
    char* pause_menu;
    bool has_pause_menu;
    char** selected_state_name_per_player;
    int32_t selected_state_name_per_player_size;
    GameplayManifestPlayer* players;
    int32_t players_size;
    Distribution* distributions;
    int32_t distributions_size;
    bool has_players;
    bool has_distributions;
    GameplayManifestHealthBar* healthbar;
    GameplayManifestGirlfriend* girlfriend;
    bool has_girlfriend;
    char* dialogue_params;
    char* dialog_text;
    bool dialog_ignore_on_freeplay;
    bool disable_resource_cache_between_songs;
} GameplayManifestSong;

typedef struct {
    Distribution* distributions;
    int32_t distributions_size;
    GameplayManifestGirlfriend* girlfriend;
    GameplayManifestHealthBar* healthbar;
    char* stage;
    char* script;
    GameplayManifestPlayer* players;
    int32_t players_size;
    char* ui_layout;
    char* pause_menu;
    char* dialogue_params;
} GameplayManifestDefault;

typedef struct GameplayManifest_s {
    GameplayManifestDefault* default_;
    GameplayManifestSong* songs;
    int32_t songs_size;
}* GameplayManifest;

GameplayManifest gameplaymanifest_init(const char* src);
void gameplaymanifest_destroy(GameplayManifest* gameplaymanifest);


#endif
