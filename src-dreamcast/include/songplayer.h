#ifndef _songplayer_h
#define _songplayer_h

#include <stdbool.h>

#include "float64.h"


#define SONGPLAYER_TRACKS_SEPARATOR '|'


typedef struct {
    float64 timestamp;
    bool completed;
} SongPlayerInfo;

typedef struct SongPlayer_s* SongPlayer;

SongPlayer songplayer_init(const char* src, bool prefer_alternative);
SongPlayer songplayer_init2(bool is_not_splitted, const char* path_voices, const char* path_instrumental);
void songplayer_destroy(SongPlayer* songplayer);
void songplayer_play(SongPlayer songplayer, SongPlayerInfo* songinfo);
void songplayer_pause(SongPlayer songplayer);
void songplayer_seek(SongPlayer songplayer, float64 timestamp);
void songplayer_poll(SongPlayer songplayer, SongPlayerInfo* songinfo);
float64 songplayer_get_duration(SongPlayer songplayer);
bool songplayer_changesong(SongPlayer songplayer, const char* src, bool prefer_alternative);
bool songplayer_is_completed(SongPlayer songplayer);
float64 songplayer_get_timestamp(SongPlayer songplayer);
void songplayer_mute_track(SongPlayer songplayer, bool vocals_or_instrumental, bool muted);
void songplayer_mute(SongPlayer songplayer, bool muted);
void songplayer_set_volume(SongPlayer songplayer, float volume);
void songplayer_set_volume_track(SongPlayer songplayer, bool vocals_or_instrumental, float volume);

bool songplayer_helper_get_tracks(const char* src, bool prefer_alternative, char** path_voices, char** path_instrumental);
char* songplayer_helper_get_tracks_full_path(const char* src);

#endif
