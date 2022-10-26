#ifndef _sndbridge_h
#define _sndbridge_h

#include <stdint.h>
#include <stdbool.h>
#include "filehandle.h"

#define StreamID_INVALID -1
#define StreamID_DECODER_FAILED -2
#define StreamID_BACKEND_FAILED -3

typedef int32_t StreamID;

extern StreamID sndbridge_queue_ogg(FileHandle_t* ogg_filehandle);
extern void sndbridge_dispose(StreamID stream);

extern double sndbridge_duration(StreamID stream);
extern double sndbridge_position(StreamID stream);
extern void sndbridge_seek(StreamID stream, double milliseconds);
extern void sndbridge_play(StreamID stream);
extern void sndbridge_pause(StreamID stream);
extern void sndbridge_stop(StreamID stream);
extern void sndbridge_set_volume(StreamID stream, float volume);
extern void sndbridge_mute(StreamID stream, bool muted);
extern void sndbridge_do_fade(StreamID, bool fade_in_or_out, float milliseconds);

extern void sndbridge_set_master_volume(float volume);

#endif
