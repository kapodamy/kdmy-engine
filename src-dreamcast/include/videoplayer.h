#ifndef _videoplayer_h
#define _videoplayer_h

#include <stdint.h>
#include <stdbool.h>

#include "soundplayer_types.h"
#include "float64.h"

//
//  forward reference
//

typedef struct Sprite_s* Sprite;


typedef struct VideoPlayer_s* VideoPlayer;


VideoPlayer videoplayer_init(const char* src);
void videoplayer_destroy(VideoPlayer* videoplayer);
Sprite videoplayer_get_sprite(VideoPlayer videoplayer);
void videoplayer_replay(VideoPlayer videoplayer);
void videoplayer_play(VideoPlayer videoplayer);
void videoplayer_pause(VideoPlayer videoplayer);
void videoplayer_stop(VideoPlayer videoplayer);
void videoplayer_loop_enable(VideoPlayer videoplayer, bool enable);
void videoplayer_fade_audio(VideoPlayer videoplayer, bool in_or_out, float duration);

void videoplayer_set_volume(VideoPlayer videoplayer, float volume);
void videoplayer_set_mute(VideoPlayer videoplayer, bool muted);
void videoplayer_seek(VideoPlayer videoplayer, float64 timestamp);
void videoplayer_set_property(VideoPlayer videoplayer, int32_t property_id, float value);

bool videoplayer_is_muted(VideoPlayer videoplayer);
bool videoplayer_is_playing(VideoPlayer videoplayer);
Fading videoplayer_has_fading_audio(VideoPlayer videoplayer);
float64 videoplayer_get_duration(VideoPlayer videoplayer);
float64 videoplayer_get_position(VideoPlayer videoplayer);
bool videoplayer_has_ended(VideoPlayer videoplayer);

bool videoplayer_has_video_track(VideoPlayer videoplayer);
bool videoplayer_has_audio_track(VideoPlayer videoplayer);
void videoplayer_poll_streams(VideoPlayer videoplayer);

#endif
