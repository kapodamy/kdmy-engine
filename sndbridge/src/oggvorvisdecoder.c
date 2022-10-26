#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "oggdecoders.h"


static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    return (size_t)file_read((FileHandle_t*)datasource, ptr, size * nmemb);
}
static int seek_func(void* datasource, ogg_int64_t offset, int whence) {
    return (int)file_seek((FileHandle_t*)datasource, (_off64_t)offset, whence);
}
static long tell_func(void* datasource) {
    return (long)file_tell((FileHandle_t*)datasource);
}



static ov_callbacks callbacks = {
    .read_func = read_func,
    .seek_func = seek_func,
    .close_func = NULL,
    .tell_func = tell_func };


OggVorbisDecoder oggvorbisdecoder_init(FileHandle_t* file_hnd) {
    OggVorbisDecoder oggvorbisdecoder = malloc(sizeof(OggVorbisDecoder_t));
    oggvorbisdecoder->current_section = 0;
    assert(oggvorbisdecoder);

    //printf("oggvorbisdecoder_init() sizeof(OggVorbis_File) is %zu\n", sizeof(OggVorbis_File));
    //printf("oggvorbisdecoder_init() sizeof(vorbis_info) is %zu\n", sizeof(vorbis_info));

    if (ov_open_callbacks((void*)file_hnd, &oggvorbisdecoder->vf, NULL, 0, callbacks) == 0) {
        vorbis_info* vi = ov_info(&oggvorbisdecoder->vf, -1);
        oggvorbisdecoder->rate = vi->rate;
        oggvorbisdecoder->channels = vi->channels;

        oggvorbisdecoder->duration = ov_time_total(&oggvorbisdecoder->vf, -1) * 1000.0;

        oggvorbisdecoder->file_hnd = file_hnd;

        return oggvorbisdecoder;
    }

    free(oggvorbisdecoder);
    return NULL;
}

void oggvorbisdecoder_destroy(OggVorbisDecoder oggvorbisdecoder) {
    ov_clear(&oggvorbisdecoder->vf);
    free(oggvorbisdecoder);
}


int32_t oggvorbisdecoder_read(OggVorbisDecoder oggvorbisdecoder, uint8_t* buffer, int32_t buffer_size) {
    return (int32_t)ov_read(
        &oggvorbisdecoder->vf, (char*)buffer, (int)buffer_size, 0, 2, 1, &oggvorbisdecoder->current_section
    );
}

void oggvorbisdecoder_get_info(OggVorbisDecoder oggvorbisdecoder, int32_t* rate, int32_t* channels, double* duration) {
    *rate = (int32_t)oggvorbisdecoder->rate;
    *channels = oggvorbisdecoder->channels;
    *duration = oggvorbisdecoder->duration;
}

bool oggvorbisdecoder_seek(OggVorbisDecoder oggvorbisdecoder, double timestamp) {
    return ov_pcm_seek(&oggvorbisdecoder->vf, (ogg_int64_t)((timestamp / 1000.0) * oggvorbisdecoder->rate));
}
