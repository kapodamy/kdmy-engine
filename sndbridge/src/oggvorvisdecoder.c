#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oggdecoders.h"

#define STR_STARTWITH(str, substr) strncmp(substr, str, strlen(substr)) == 0

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    return (size_t)filehandle_read((FileHandle_t*)datasource, ptr, size * nmemb);
}
static int seek_func(void* datasource, ogg_int64_t offset, int whence) {
    return (int)filehandle_seek((FileHandle_t*)datasource, (_off64_t)offset, whence);
}
static long tell_func(void* datasource) {
    return (long)filehandle_tell((FileHandle_t*)datasource);
}


static ov_callbacks callbacks = {
    .read_func = read_func,
    .seek_func = seek_func,
    .close_func = NULL,
    .tell_func = tell_func
};


OggVorbisDecoder oggvorbisdecoder_init(FileHandle_t* file_hnd) {
    OggVorbisDecoder oggvorbisdecoder = malloc(sizeof(OggVorbisDecoder_t));
    oggvorbisdecoder->current_section = 0;
    assert(oggvorbisdecoder);

    // printf("oggvorbisdecoder_init() sizeof(OggVorbis_File) is %zu\n", sizeof(OggVorbis_File));
    // printf("oggvorbisdecoder_init() sizeof(vorbis_info) is %zu\n", sizeof(vorbis_info));

    if (ov_open_callbacks((void*)file_hnd, &oggvorbisdecoder->vf, NULL, 0, callbacks) == 0) {
        vorbis_info* vi = ov_info(&oggvorbisdecoder->vf, -1);
        oggvorbisdecoder->rate = vi->rate;
        oggvorbisdecoder->channels = vi->channels;

        oggvorbisdecoder->duration = ov_time_total(&oggvorbisdecoder->vf, -1) * 1000.0;

        oggvorbisdecoder->file_hnd = file_hnd;

       oggvorbisdecoder->loop_start = -1;
       oggvorbisdecoder->loop_length = -1;

        // find loop points
        vorbis_comment* comments_info = ov_comment(&oggvorbisdecoder->vf, -1);
        for (int i = 0; i < comments_info->comments; i++) {
            //int length = comments_info->comment_lengths[i];
            char* comment = comments_info->user_comments[i];

            if (STR_STARTWITH(comment, OGGDECODERS_LOOPSTART)) {
                oggvorbisdecoder->loop_start = strtoll(comment + strlen(OGGDECODERS_LOOPSTART), NULL, 10);
            }

            if (STR_STARTWITH(comment, OGGDECODERS_LOOPLENGTH)) {
                oggvorbisdecoder->loop_length = strtoll(comment + strlen(OGGDECODERS_LOOPLENGTH), NULL, 10);
            }
        }

        return oggvorbisdecoder;
    }

    free(oggvorbisdecoder);
    return NULL;
}

void oggvorbisdecoder_destroy(OggVorbisDecoder oggvorbisdecoder) {
    ov_clear(&oggvorbisdecoder->vf);
    free(oggvorbisdecoder);
}


int32_t oggvorbisdecoder_read(OggVorbisDecoder oggvorbisdecoder, float* buffer, int32_t samples_per_channel) {
    const int32_t channels = oggvorbisdecoder->channels;
    float** pcm_channels = NULL;

    int32_t readed = ov_read_float(
        &oggvorbisdecoder->vf, &pcm_channels, samples_per_channel, &oggvorbisdecoder->current_section
    );

    int offset = 0;
    for (int i = 0; i < readed; i++) {
        for (int j = 0; j < channels; j++) {
            buffer[offset++] = pcm_channels[j][i];
        }
    }

    return readed;
}

void oggvorbisdecoder_get_info(OggVorbisDecoder oggvorbisdecoder, int32_t* rate, int32_t* channels, double* duration) {
    *rate = (int32_t)oggvorbisdecoder->rate;
    *channels = oggvorbisdecoder->channels;
    *duration = oggvorbisdecoder->duration;
}

bool oggvorbisdecoder_seek(OggVorbisDecoder oggvorbisdecoder, double timestamp) {
    return ov_pcm_seek(&oggvorbisdecoder->vf, (ogg_int64_t)((timestamp / 1000.0) * oggvorbisdecoder->rate));
}

void oggvorbisdecoder_get_loop_points(OggVorbisDecoder oggvorbisdecoder, int64_t* loop_start, int64_t* loop_length) {
    *loop_start = oggvorbisdecoder->loop_start;
    *loop_length = oggvorbisdecoder->loop_length;
}
