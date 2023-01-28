#ifndef _sndbridge_h
#define _sndbridge_h

#include "filehandle.h"
#include "externaldecoder.h"
#include <stdbool.h>
#include <stdint.h>

#define StreamID_INVALID -1
#define StreamID_DECODER_FAILED -2
#define StreamID_BACKEND_FAILED -3

#define StreamFading_NONE 0
#define StreamFading_IN 1
#define StreamFading_OUT 2

typedef int32_t StreamID;
typedef int32_t StreamFading;

extern StreamID sndbridge_queue_ogg(FileHandle_t* ogg_filehandle);
extern StreamID sndbridge_queue(ExternalDecoder* external_decoder);
extern void sndbridge_dispose(StreamID stream_id);

extern double sndbridge_duration(StreamID stream_id);
extern double sndbridge_position(StreamID stream_id);

extern void sndbridge_seek(StreamID stream_id, double milliseconds);
extern void sndbridge_play(StreamID stream_id);
extern void sndbridge_pause(StreamID stream_id);
extern void sndbridge_stop(StreamID stream_id);

extern void sndbridge_set_volume(StreamID stream_id, float volume);
extern void sndbridge_mute(StreamID stream_id, bool muted);
extern void sndbridge_do_fade(StreamID stream_id, bool fade_in_or_out, float milliseconds);
extern bool sndbridge_is_active(StreamID stream_id);
extern StreamFading sndbridge_has_fade_active(StreamID stream_id);
extern bool sndbridge_has_ended(StreamID stream_id);
extern void sndbridge_loop(StreamID stream_id, bool enable);
extern void sndbridge_set_master_volume(float volume);
extern const char* sndbridge_get_runtime_info();

extern void sndbridge_set_master_volume(float volume);

#endif
