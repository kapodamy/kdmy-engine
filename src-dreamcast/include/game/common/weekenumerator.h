#ifndef _weekenumerator_h
#define _weekenumerator_h

#include <stdbool.h>
#include <stdint.h>

#include "bno.h"


//
//  forward reference
//

typedef const BNO_Entry* JSONToken;


typedef struct {
    bool hide_if_locked;
    const char* unlock_directive;
    const char* manifest;
    const char* name;
} WeekInfoUnlockableCharacter;
typedef struct {
    const char* name;
    const char* unlock_directive;
} WeekInfoCustomDifficult;
typedef struct {
    const char* name;
    const char* freeplay_host_icon_model;
    const char* freeplay_host_icon_name;
    const char* freeplay_locked_host_icon_model;
    const char* freeplay_locked_host_icon_name;
    const char* freeplay_background;
    bool freeplay_only;
    const char* freeplay_unlock_directive;
    bool freeplay_hide_if_week_locked;
    bool freeplay_hide_if_locked;
    const char* freeplay_gameplaymanifest;
    int32_t freeplay_song_index_in_gameplaymanifest;
    const char* freeplay_song_filename;
    const char* freeplay_description;
    float freeplay_seek_time;
} WeekInfoSong;
typedef struct {
    WeekInfoUnlockableCharacter* boyfriend_models;
    int32_t boyfriend_models_size;
    WeekInfoUnlockableCharacter* girlfriend_models;
    int32_t girlfriend_models_size;
} WeekInfoUnlockables;


typedef struct WeekInfo_s {
    const char* name;
    const char* display_name;
    const char* description;
    const char* week_title_model;
    const char* week_title_model_animation_name;
    bool host_flip_sprite;
    bool host_enable_beat;
    bool host_hide_if_week_locked;
    const char* week_host_character_manifest;
    const char* week_host_model;
    const char* week_host_model_idle_animation_name;
    const char* week_host_model_choosen_animation_name;
    WeekInfoSong* songs;
    int32_t songs_count;
    uint32_t selector_background_color;
    bool selector_background_color_override;
    bool has_difficulty_easy;
    bool has_difficulty_normal;
    bool has_difficulty_hard;
    const char* custom_difficults_model;
    WeekInfoCustomDifficult* custom_difficults;
    int32_t custom_difficults_size;
    const char* default_difficulty;
    const char* unlock_directive;
    const char* emit_directive;
    const char* warning_message;
    const char* sensible_content_message;
    bool disallow_custom_boyfriend;
    bool disallow_custom_girlfriend;
    WeekInfoUnlockables unlockables;
    const char* custom_selector_layout;
    const char* custom_folder;
    const char* custom_folder_gameplay;
    bool has_greetings;
    const char* songs_default_freeplay_host_icon_model;
} WeekInfo;

typedef struct {
    int32_t size;
    WeekInfo* array;
} WeekArray;


extern WeekArray weeks_array;
extern const WeekInfo* custom_style_from_week;

void weekenumerator_enumerate();
char* weekenumerator_get_host_model(const WeekInfo* weekinfo);
char* weekenumerator_get_title_texture(const WeekInfo* weekinfo);
char* weekenumerator_get_week_folder(const WeekInfo* weekinfo);
char* weekenumerator_get_custom_folder(const WeekInfo* weekinfo);
char* weekenumerator_get_gameplay_manifest(const WeekInfo* weekinfo);
char* weekenumerator_get_greetings(const WeekInfo* weekinfo);
char* weekenumerator_get_asset(const WeekInfo* weekinfo, const char* relative_path);
char* weekenumerator_parse_path(JSONToken json, const char* json_property_name);

#endif