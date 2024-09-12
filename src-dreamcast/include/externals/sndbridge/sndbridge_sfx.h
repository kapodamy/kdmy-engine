#ifndef __sndbridge_sfx_h
#define __sndbridge_sfx_h

#include "sndbridge.h"

typedef struct _StreamSFX StreamSFX;

sndbridge_time_t sndbridge_sfx_duration(StreamSFX* streamsfx);
sndbridge_time_t sndbridge_sfx_position(StreamSFX* streamsfx);

void sndbridge_sfx_seek(StreamSFX* streamsfx, sndbridge_time_t position);
void sndbridge_sfx_play(StreamSFX* streamsfx);
void sndbridge_sfx_pause(StreamSFX* streamsfx);
void sndbridge_sfx_stop(StreamSFX* streamsfx);
void sndbridge_sfx_set_volume(StreamSFX* streamsfx, float volume);
void sndbridge_sfx_mute(StreamSFX* streamsfx, bool muted);
bool sndbridge_sfx_is_active(StreamSFX* streamsfx);
void sndbridge_sfx_do_fade(StreamSFX* streamsfx, bool fade_in_or_out, float seconds);
StreamFading sndbridge_sfx_active_fade(StreamSFX* streamsfx);
bool sndbridge_sfx_has_ended(StreamSFX* streamsfx);
void sndbridge_sfx_set_looped(StreamSFX* streamsfx, bool enable);
void sndbridge_sfx_set_queueing(StreamSFX* streamsfx, bool enable);
void sndbridge_sfx_force_resync(StreamSFX* streamsfx);

void sndbridge_sfx_destroy(StreamSFX* streamsfx);
StreamSFX* sndbridge_sfx_init(SourceHandle* hnd);

void sndbride_sfx_startup();
void sndbride_sfx_shutdown();

bool sndbridge_can_use_sfx_backend(SourceHandle* hnd);

#ifdef DEBUG
void sndbridge_sfx_main();
#endif

#endif