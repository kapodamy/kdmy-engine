#ifndef __videoplayer_h
#define __videoplayer_h

#include <stdbool.h>
#include "sprite.h"

typedef struct _VideoPlayer_t {
    int dummy;
} VideoPlayer_t;

typedef VideoPlayer_t* VideoPlayer;

Sprite videoplayer_get_sprite(VideoPlayer videoplayer);
void videoplayer_replay(VideoPlayer videoplayer);
void videoplayer_play(VideoPlayer videoplayer);
void videoplayer_pause(VideoPlayer videoplayer);
void videoplayer_stop(VideoPlayer videoplayer);
void videoplayer_loop_enable(VideoPlayer videoplayer, bool enable);
void videoplayer_fade_audio(VideoPlayer videoplayer, bool in_or_out, float duration);
void videoplayer_set_volume(VideoPlayer videoplayer, float volume);
void videoplayer_set_mute(VideoPlayer videoplayer, bool muted);
void videoplayer_seek(VideoPlayer videoplayer, double timestamp);
bool videoplayer_is_muted(VideoPlayer videoplayer);
bool videoplayer_is_playing(VideoPlayer videoplayer);
double videoplayer_get_duration(VideoPlayer videoplayer);
double videoplayer_get_position(VideoPlayer videoplayer);
bool videoplayer_has_ended(VideoPlayer videoplayer);
bool videoplayer_has_video_track(VideoPlayer videoplayer);
bool videoplayer_has_audio_track(VideoPlayer videoplayer);


#endif