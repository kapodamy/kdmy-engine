#ifndef __sndbridge_h
#define __sndbridge_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"
#include "decoderhandle.h"
#include "sourcehandle.h"


#ifdef _arch_dreamcast
typedef int32_t sndbridge_time_t;
#else
typedef float64 sndbridge_time_t;
#endif

typedef enum {
    STREAMFADING_NONE = 0,
    STREAMFADING_IN = 1,
    STREAMFADING_OUT = 2
} StreamFading;

typedef enum {
    STREAMRESULT_Success = 0,
    STREAMRESULT_DecoderFailed = -2,
    STREAMRESULT_CreateFailed = -3,
    STREAMRESULT_BackendFailed = -4
} StreamResult;


typedef struct Stream Stream;

void sndbridge_init();
void sndbridge_dispose_backend();
StreamResult sndbridge_enqueue1(SourceHandle* source_handle, Stream** created_stream);
StreamResult sndbridge_enqueue2(DecoderHandle* external_decoder, Stream** created_stream);
void sndbridge_set_master_volume(float volume);
void sndbridge_set_master_muted(bool muted);
const char* sndbridge_get_runtime_info();

sndbridge_time_t sndbridge_stream_duration(Stream* stream);
sndbridge_time_t sndbridge_stream_position(Stream* stream);
void sndbridge_stream_seek(Stream* stream, sndbridge_time_t position);
void sndbridge_stream_play(Stream* stream);
void sndbridge_stream_pause(Stream* stream);
void sndbridge_stream_stop(Stream* stream);
void sndbridge_stream_set_volume(Stream* stream, float volume);
void sndbridge_stream_mute(Stream* stream, bool muted);
void sndbridge_stream_do_fade(Stream* stream, bool fade_in_or_out, float seconds);
bool sndbridge_stream_is_active(Stream* stream);
StreamFading sndbridge_stream_active_fade(Stream* stream);
bool sndbridge_stream_has_ended(Stream* stream);
void sndbridge_stream_set_looped(Stream* stream, bool enable);
void sndbridge_stream_set_queueing(Stream* stream, bool enable);
void sndbridge_stream_force_resync(Stream* stream);
void sndbridge_stream_destroy(Stream* stream);

#if DEBUG
void sndbridge_main();
#endif

#endif
