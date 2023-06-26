#include "../ffgraph.h"
#include <malloc.h>
#include <stdio.h>
#include <assert.h>


static void dump_data(uint32_t size, void* data) {
    static int index = 0;
    char filename[256];
    int len = sprintf(filename, "frame_%d.data", index++);
    filename[len] = '\0';

    FILE* file = fopen(filename, "wb");
    if (file) {
        fwrite(data, size, 1, file);
        fflush(file);
        fclose(file);
    }
}

static FILE* audio_raw;
static float samples[1024 * 1024];

int main2(int argc, char** argv) {
    //printf("<%llx>\n", AV_NOPTS_VALUE);
    float buffer[8192 * 2];
    int sizes[] = {
        8192,
        7616,
        6592,
        5568,
        4544,
        3520,
        2496,
        1472,
        448,
        7971,
        5923,
        4899,
        3875,
        2851,
        1827,
        803,
        7938,
        5890,
        4866,
        3842,
        3266,
        3138,
        3010,
        2882,
        2754,
        2626,
        2050,
        1026,
        7940,
        6916,
        5892,
        4868,
        3844,
        2820,
        1796,
        772,
        8080,
        6032,
        5008,
        3984,
        2960,
        1936,
        912,
        7763,
        5715,
        4691,
        3667,
        2643,
        1619,
        595,
        7971,
        5923,
        4899,
        3875,
        2851,
        1827,
        803,
        7938,
        5890,
        4866,
        3842,
        2818,
        1794,
        770,
        7497,
        5449,
        4425,
        3401,
        2377,
        1353,
        329,
        7938,
        5890,
        5314,
        5186,
        5058,
        4930,
        4802,
        4226,
        3202,
        2178,
        1154,
        130,
        7497,
        5449,
        4425,
        3401,
        2377,
        1353,
        329,
        7938,
        5890,
        4866,
        3842,
        2818,
        1794,
        770,
        7968,
        -1};

    (void)argc;
    (void)argv;

    printf("version: %s\n", ffgraph_get_runtime_info());
    FileHandle_t* audio_filehandle = filehandle_init("./a.webm");
    FileHandle_t* video_filehandle = filehandle_init("./a.webm");

    FFGraph* ffgraph = ffgraph_init(video_filehandle, audio_filehandle);

    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    audio_raw = fopen("./audio.data", "wb");
    for (size_t i = 0; sizes[i] >= 0; i++) {
        int readed = ffgraph_read_audio_samples(ffgraph, buffer, sizes[i]);
        fwrite(buffer, sizeof(float), readed * info.audio_channels, audio_raw);
    }
    fclose(audio_raw);

    return 0;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("version: %s\n", av_version_info());

    audio_raw = fopen("./audio.data", "wb");
    FileHandle_t* audio_filehandle = filehandle_init3("./a.webm");
    FileHandle_t* video_filehandle = filehandle_init3("./a.webm");

    assert (audio_filehandle != NULL && video_filehandle != NULL);

    FFGraph* ffgraph = ffgraph_init(video_filehandle, audio_filehandle);

    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    int frame_video_stop = 40;
    int samples_audio_stop = 106;
    int iters = 0;
    bool test_seek = true;
L_read_streams:

    while (!ffgraph->video->has_ended) {
        void* frame;
        int32_t frame_size;
        double seconds = ffgraph_read_video_frame(ffgraph, &frame, &frame_size);
        if (seconds < 0 || iters++ >= frame_video_stop) {
            printf("break: seconds < 0\n");
            break;
        }
        dump_data(frame_size, frame);
    }

    iters = 0;
    while (!ffgraph->audio->has_ended) {
        int readed = ffgraph_read_audio_samples(ffgraph, samples, sizeof(samples) / 8);
        if (readed < 0 || iters++ >= samples_audio_stop) {
            printf("break: readed < 0\n");
            break;
        }
        fwrite(samples, sizeof(float), readed * info.audio_channels, audio_raw);
    }

    if (test_seek) {
        samples_audio_stop = frame_video_stop = INT32_MAX;
        test_seek = false;

        ffgraph_seek(ffgraph, 0.458);
        fflush(audio_raw);
        fseek(audio_raw, 0, SEEK_SET);

        goto L_read_streams;
    }

    ffgraph_destroy(ffgraph);
    fclose(audio_raw);
    filehandle_destroy(video_filehandle);
    filehandle_destroy(audio_filehandle);

    return 0;
}
