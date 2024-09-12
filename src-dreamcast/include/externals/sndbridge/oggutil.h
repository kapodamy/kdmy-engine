#ifndef __oggutil_h
#define __oggutil_h

#include "decoderhandle.h"
#include "sourcehandle.h"

DecoderHandle* oggutil_init_ogg_decoder(SourceHandle* src_hnd);
void oggutil_find_loop_points(int count, char** comments, int* lengths, int64_t* loop_start, int64_t* loop_length);

DecoderHandle* oggvorbisdecoder_init(SourceHandle* src_hnd);

#endif
