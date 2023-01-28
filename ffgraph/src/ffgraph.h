#ifndef _ffgraph_decoder_h
#define _ffgraph_decoder_h

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "filehandle.h"
#include "externaldecoder.h"

struct BufferNode_t;

typedef struct BufferNode_t {
    uint8_t* data;
    int data_size;
    int64_t seconds;
    struct BufferNode_t* next_node;
} BufferNode;


typedef struct {
    BufferNode* head;
    BufferNode* tail;
} Buffering;

struct FFGraphConversor_t;

typedef bool (*ConverterCallback)(struct FFGraphConversor_t* conv_ctx, const AVCodecContext* codec_ctx, const AVFrame* av_frame);
typedef void (*DestroyCallback)(struct FFGraphConversor_t* conv_ctx);
typedef void (*FlushCallback)(struct FFGraphConversor_t* conv_ctx);
typedef void (*BufferSeekCallback)(struct FFGraphConversor_t* conv_ctx, double seconds);

typedef struct FFGraphConversor_t {
    ConverterCallback convert_cb;
    DestroyCallback destroy_cb;
    FlushCallback flush_cb;
    BufferSeekCallback bufferseek_cb;
    void* priv_data;
    const AVStream* av_stream;
} FFGraphConversor;


typedef struct {
    AVFormatContext* fmt_ctx;
    AVPacket* packet;

    FFGraphConversor ffgraphconv;

    AVIOContext* iohandle;

    AVCodecContext* codec_ctx;
    const AVCodec* av_codec;
    AVFrame* av_frame;
    int stream_idx;
    bool has_ended;
} FFGraphFormat;


FFGraphFormat* ffgraphfmt_init(const FileHandle_t* filehandle, const enum AVMediaType required_type);
void ffgraphfmt_destroy(FFGraphFormat* ffgraphdec);
bool ffgraphfmt_read(FFGraphFormat* ffgraphfmt);
void ffgraphfmt_seek(FFGraphFormat* ffgraphfmt, double seconds);
double ffgraphfmt_get_stream_duration(FFGraphFormat* ffgraphfmt);

bool videoconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* conv_ctx);
void videoconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_width, int* out_height);
double videoconverter_read(FFGraphConversor* ffgraphconv, double seconds, void** frame_out, int32_t* frame_size_out);

bool audioconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* ffgraphconv);
void audioconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_channels, int* out_rate);
int32_t audioconverter_read(FFGraphConversor* ffgraphconv, void* samples_out, int32_t max_samples_per_channel);

AVIOContext* iohandler_init(const FileHandle_t* filehandle);
void iohandler_destroy(AVIOContext** iohandle);

/*
////////////
// UNUSED //
////////////
Buffering* buffering_init();
void buffering_destroy(Buffering** buffering);
void buffering_add(Buffering* buffering, void* data, int data_size, int64_t seconds);
bool buffering_get(Buffering* buffering, void** data, int* data_size, int64_t* seconds);
void buffering_free_data(void* data);
void buffering_clear(Buffering* buffering);
*/

static inline double calculate_seconds(const AVStream* av_stream, const AVFrame* av_frame) {
    int64_t start_pts = av_stream->start_time;
    if (start_pts == AV_NOPTS_VALUE) start_pts = 0;

    double time_base = av_stream->time_base.num / (double)av_stream->time_base.den;
    double seconds = (av_frame->pts - start_pts) * time_base;

    return seconds;
}


typedef struct {
    FFGraphFormat* video;
    FFGraphFormat* audio;
} FFGraph;

typedef struct {
    int32_t audio_has_stream;
    int32_t video_has_stream;

    int32_t audio_channels;
    int32_t audio_sample_rate;

    int32_t video_width;
    int32_t video_height;

    double video_seconds_duration;
    double audio_seconds_duration;
} FFGraphInfo;


extern FFGraph* ffgraph_init(FileHandle_t* video_filehandle, FileHandle_t* audio_filehandle);
extern void ffgraph_destroy(FFGraph* ffgraph);
extern void ffgraph_get_streams_info(FFGraph* ffgraph, FFGraphInfo* output_info);
extern int32_t ffgraph_read_audio_samples(FFGraph* ffgraph, void* out_samples, int32_t max_samples_per_channel);
extern double ffgraph_read_video_frame(FFGraph* ffgraph, void** out_frame, int32_t* out_frame_size);
extern double ffgraph_read_video_frame2(FFGraph* ffgraph, uint8_t* buffer, int32_t buffer_size);
extern void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds);
extern void ffgraph_seek2(FFGraph* ffgraph, double time_in_seconds, bool audio_or_video);

extern ExternalDecoder* ffgraph_sndbridge_create_helper(FFGraph* ffgraph, bool allow_seek, bool allow_destroy);
extern void ffgraph_sndbridge_destroy_helper(ExternalDecoder* ffgraph_sndbridge);

extern const char* ffgraph_get_runtime_info();

#endif
