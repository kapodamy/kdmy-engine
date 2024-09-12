#ifndef _soundplayer_h
#define _soundplayer_h

#include "float64.h"
#include <stdbool.h>
#include <stdint.h>

#include "soundplayer_types.h"


typedef struct SoundPlayer_s* SoundPlayer;


SoundPlayer soundplayer_init(const char* src);
void soundplayer_destroy(SoundPlayer* soundplayer);


void soundplayer_replay(SoundPlayer soundplayer);
void soundplayer_play(SoundPlayer soundplayer);
void soundplayer_pause(SoundPlayer soundplayer);
void soundplayer_stop(SoundPlayer soundplayer);
void soundplayer_loop_enable(SoundPlayer soundplayer, bool enable);
void soundplayer_fade(SoundPlayer soundplayer, bool in_or_out, float duration);

void soundplayer_set_volume(SoundPlayer soundplayer, float volume);
void soundplayer_set_mute(SoundPlayer soundplayer, bool muted);
void soundplayer_seek(SoundPlayer soundplayer, float64 timestamp);
void soundplayer_set_property(SoundPlayer soundplayer, int32_t property_id, float value);

bool soundplayer_is_muted(SoundPlayer soundplayer);
bool soundplayer_is_playing(SoundPlayer soundplayer);
Fading soundplayer_has_fading(SoundPlayer soundplayer);
float64 soundplayer_get_duration(SoundPlayer soundplayer);
float64 soundplayer_get_position(SoundPlayer soundplayer);
bool soundplayer_has_ended(SoundPlayer soundplayer);
void soundplayer_set_queueing(SoundPlayer soundplayer, bool enabled);
void soundplayer_force_resync(SoundPlayer soundplayer);

#endif
