#ifndef __soundplayer_h
#define __soundplayer_h

#include <stdbool.h>

typedef struct _SoundPlayer_t {
    int dummy;
} SoundPlayer_t;

typedef SoundPlayer_t *SoundPlayer;


void soundplayer_play(SoundPlayer soundplayer);
void soundplayer_pause(SoundPlayer soundplayer);
void soundplayer_stop(SoundPlayer soundplayer);
void soundplayer_loop_enable(SoundPlayer soundplayer, bool enable);
void soundplayer_fade(SoundPlayer soundplayer, bool in_or_out, float duration);
void soundplayer_set_volume(SoundPlayer soundplayer, float volume);
void soundplayer_set_mute(SoundPlayer soundplayer, bool muted);
bool soundplayer_is_muted(SoundPlayer soundplayer);
bool soundplayer_is_playing(SoundPlayer soundplayer);
double soundplayer_get_duration(SoundPlayer soundplayer);
double soundplayer_get_position(SoundPlayer soundplayer);
void soundplayer_seek(SoundPlayer soundplayer, double timestamp);

#endif

