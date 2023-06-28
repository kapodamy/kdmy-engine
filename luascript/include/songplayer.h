#ifndef __songplayer_h
#define __songplayer_h

#include <stdbool.h>

typedef struct _SongInfo_t {
    bool completed;
    double timestamp;
} SongInfo_t;

typedef struct _SongPlayer_t {
    int dummy;
} SongPlayer_t;

typedef SongPlayer_t* SongPlayer;


bool songplayer_changesong(SongPlayer songplayer, const char* src, bool prefer_alternative);
void songplayer_play(SongPlayer songplayer, SongInfo_t* songinfo);
void songplayer_pause(SongPlayer songplayer);
void songplayer_seek(SongPlayer songplayer, double timestamp);
double songplayer_get_duration(SongPlayer songplayer);
bool songplayer_is_completed(SongPlayer songplayer);
double songplayer_get_timestamp(SongPlayer songplayer);
void songplayer_mute_track(SongPlayer songplayer, bool vocals_or_instrumental, bool muted);
void songplayer_mute(SongPlayer songplayer, bool muted);
void songplayer_set_volume_track(SongPlayer songplayer, bool vocals_or_instrumental, float volume);
void songplayer_set_volume(SongPlayer songplayer, float volume);

#endif

