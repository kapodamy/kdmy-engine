#include <errno.h>

#ifdef SNDBRINDE_TREMOR
#include <vorbis/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif

#include "externals/sndbridge/oggutil.h"
#include "logger.h"
#include "malloc_utils.h"


#ifndef _arch_dreamcast
#error "this OGGVorbisDecoder is only designed for pcm16le, which is only available in the dreamcast"
#endif


typedef struct {
    DecoderHandle handle;

    OggVorbis_File vf;

    int current_section;
    SourceHandle* src_hnd;
    float64 duration_seconds;
    uint32_t channels, rate;
    int64_t loop_start, loop_length;
} OGGVorbisDecoder;


static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    SourceHandle* src_hnd = (SourceHandle*)datasource;
    int32_t ret = src_hnd->read(src_hnd, ptr, (int32_t)(size * nmemb));

#ifdef SNDBRINDE_TREMOR
    if (ret < 0) {
        ret = 0;
        if (errno == 0) errno = EINVAL;
    } else {
        errno = 0;
    }
#endif

    return (size_t)ret;
}
static int seek_func(void* datasource, ogg_int64_t offset, int whence) {
    SourceHandle* src_hnd = (SourceHandle*)datasource;
    int32_t ret = src_hnd->seek(src_hnd, (int64_t)offset, whence);

    return (int)ret;
}
static long tell_func(void* datasource) {
    SourceHandle* src_hnd = (SourceHandle*)datasource;
    return (long)src_hnd->tell(src_hnd);
}

#ifdef SNDBRINDE_TREMOR
static int close_func(void* datasource) {
    (void)datasource;
    return 0;
}
#endif


static int32_t oggvorbisdecoder_readBytes(DecoderHandle* decoder, void* buffer, uint32_t buffer_size) {
    OGGVorbisDecoder* oggvorbisdecoder = (OGGVorbisDecoder*)decoder;

#ifdef SNDBRINDE_TREMOR
    long readed = ov_read(&oggvorbisdecoder->vf, buffer, (int)buffer_size, &oggvorbisdecoder->current_section);
#else
    // request sample format pcm16sle (pcm 16-bits signed litte-endian)
    long readed = ov_read(&oggvorbisdecoder->vf, buffer, (int)buffer_size, 0, 2, 1, &oggvorbisdecoder->current_section);
#endif

    return readed;
}
static SampleFormat oggvorbisdecoder_getInfo(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, float64* duration) {
    OGGVorbisDecoder* oggvorbisdecoder = (OGGVorbisDecoder*)decoder;

    *rate = oggvorbisdecoder->rate;
    *channels = oggvorbisdecoder->channels;
    *duration = oggvorbisdecoder->duration_seconds;

    return SampleFormat_PCM_S16LE;
}
static bool oggvorbisdecoder_seek(DecoderHandle* decoder, float64 seconds) {
    OGGVorbisDecoder* oggvorbisdecoder = (OGGVorbisDecoder*)decoder;
    return ov_pcm_seek(&oggvorbisdecoder->vf, (ogg_int64_t)(seconds * oggvorbisdecoder->rate)) != 0;
}
static void oggvorbisdecoder_getLoopPoints(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length) {
    OGGVorbisDecoder* oggvorbisdecoder = (OGGVorbisDecoder*)decoder;

    *loop_start = oggvorbisdecoder->loop_start;
    *loop_length = oggvorbisdecoder->loop_length;
}
static void oggvorbisdecoder_destroy(DecoderHandle* decoder) {
    OGGVorbisDecoder* oggvorbisdecoder = (OGGVorbisDecoder*)decoder;
    ov_clear(&oggvorbisdecoder->vf);
    free_chk(decoder);
}

DecoderHandle* oggvorbisdecoder_init(SourceHandle* src_hnd) {
    OGGVorbisDecoder* oggvorbisdecoder = malloc_chk(sizeof(OGGVorbisDecoder));
    if (!oggvorbisdecoder) {
        logger_error("oggvorbisdecoder_init() out-of-memory");
        return NULL;
    }

    oggvorbisdecoder->current_section = 0;
    oggvorbisdecoder->src_hnd = src_hnd;

    ov_callbacks callbacks = (ov_callbacks){
        .read_func = read_func,
        .seek_func = seek_func,
        .tell_func = tell_func,
        .close_func = NULL
    };
#ifdef SNDBRINDE_TREMOR
    // dumb tremor crashes if there no close callback, stub it
    callbacks.close_func = close_func;
#endif

    int ret = ov_open_callbacks(oggvorbisdecoder->src_hnd, &oggvorbisdecoder->vf, NULL, 0, callbacks);
    if (ret != 0) {
        free_chk(oggvorbisdecoder);
        return NULL;
    }

    vorbis_info* vi = ov_info(&oggvorbisdecoder->vf, -1);
    oggvorbisdecoder->rate = (uint32_t)vi->rate;
    oggvorbisdecoder->channels = (uint32_t)vi->channels;

    oggvorbisdecoder->duration_seconds = ov_time_total(&oggvorbisdecoder->vf, -1);

    oggvorbisdecoder->loop_start = 0;
    oggvorbisdecoder->loop_length = 0;

    // find loop points
    vorbis_comment* comments_info = ov_comment(&oggvorbisdecoder->vf, -1);
    oggutil_find_loop_points(
        comments_info->comments,
        comments_info->user_comments,
        comments_info->comment_lengths,
        &oggvorbisdecoder->loop_start,
        &oggvorbisdecoder->loop_length
    );

    oggvorbisdecoder->handle.readBytes = oggvorbisdecoder_readBytes;
    oggvorbisdecoder->handle.getInfo = oggvorbisdecoder_getInfo;
    oggvorbisdecoder->handle.seek = oggvorbisdecoder_seek;
    oggvorbisdecoder->handle.getLoopPoints = oggvorbisdecoder_getLoopPoints;
    oggvorbisdecoder->handle.destroy = oggvorbisdecoder_destroy;

    if (vi->channels > 2) {
        logger_error("oggvorbisdecoder_init() only mono and stereo channels are supported");
        oggvorbisdecoder_destroy((DecoderHandle*)oggvorbisdecoder);
        return NULL;
    }

#ifdef SNDBRINDE_TREMOR
    // dumb tremor returns the duration in milliseconds
    oggvorbisdecoder->duration_seconds /= 1000.0;
#endif

    return (DecoderHandle*)oggvorbisdecoder;
}
