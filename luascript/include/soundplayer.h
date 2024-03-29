#ifndef __soundplayer_h
#define __soundplayer_h

#include <stdbool.h>

typedef struct _SoundPlayer_t {
    int dummy;
} SoundPlayer_t;

typedef SoundPlayer_t *SoundPlayer;

typedef int32_t Fadding;


SoundPlayer soundplayer_init(const char* src);
void soundplayer_destroy(SoundPlayer* soundplayer);
void soundplayer_play(SoundPlayer soundplayer);
void soundplayer_pause(SoundPlayer soundplayer);
void soundplayer_stop(SoundPlayer soundplayer);
void soundplayer_loop_enable(SoundPlayer soundplayer, bool enable);
void soundplayer_fade(SoundPlayer soundplayer, bool in_or_out, float duration);
void soundplayer_set_volume(SoundPlayer soundplayer, float volume);
void soundplayer_set_mute(SoundPlayer soundplayer, bool muted);
Fadding soundplayer_has_fading(SoundPlayer soundplayer);
bool soundplayer_is_muted(SoundPlayer soundplayer);
bool soundplayer_is_playing(SoundPlayer soundplayer);
double soundplayer_get_duration(SoundPlayer soundplayer);
double soundplayer_get_position(SoundPlayer soundplayer);
void soundplayer_seek(SoundPlayer soundplayer, double timestamp);
bool soundplayer_has_ended(SoundPlayer soundplayer);
#endif

