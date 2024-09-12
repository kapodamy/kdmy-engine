/*
PL_MPEG - MPEG1 Video decoder

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2019 Dominic Szablewski

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.




-- Synopsis

// Define `PL_MPEG_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define PL_MPEG_IMPLEMENTATION
#include "plmpeg.h"

// This function gets called for each decoded video frame
void my_video_callback(plm_t *plm, plm_frame_t *frame, void *user) {
	// Do something with frame->y.data, frame->cr.data, frame->cb.data
}

// Load a .mpg (MPEG Program Stream) file
plm_t *plm = plm_create_with_filename("some-file.mpg");

// Install the video decode callbacks
plm_set_video_decode_callback(plm, my_video_callback, my_data);


// Decode
do {
	plm_decode(plm, time_since_last_call);
} while (!plm_has_ended(plm));

// All done
plm_destroy(plm);



-- Documentation

This library provides several interfaces to decode MPEG video data.
A high-level API combines the video decoder in an easy to use wrapper.

Lower-level APIs for accessing video decoder, 
as well as providing different data sources are also available.

Interfaces are written in an object oriented style, meaning you create object 
instances via various different constructor functions (plm_*create()),
do some work on them and later dispose them via plm_*destroy().

plm_* ......... the high-level interface decoder
plm_buffer_* .. the data source used by all interfaces
plm_video_* ... the MPEG1 Video ("mpeg1") decoder


With the high-level interface you have two options to decode video:

 1. Use plm_decode() and just hand over the delta time since the last call.
    It will decode everything needed and call your callbacks (specified through
    plm_set_video_decode_callback()) any number of times.

 2. Use plm_decode_video() to decode exactly one
    frame of video data at a time. How you handle the synchronization 
    of both streams is up to you.

If you only want to decode video through these functions, you should
disable the other stream (plm_set_video_enabled(FALSE))

Video data is decoded into a struct with all 3 planes (Y, Cr, Cb) stored in
separate buffers. You can either convert this to RGB on the CPU (slow) via the
plm_frame_to_rgb() function or do it on the GPU with the following matrix:

mat4 bt601 = mat4(
	1.16438,  0.00000,  1.59603, -0.87079,
	1.16438, -0.39176, -0.81297,  0.52959,
	1.16438,  2.01723,  0.00000, -1.08139,
	0, 0, 0, 1
);
gl_FragColor = vec4(y, cb, cr, 1.0) * bt601;


Data can be supplied to the high level interface decoder
in three different ways:

 1. Using plm_create_from_filename() or with a file handle with 
    plm_create_from_file().

 2. Using plm_create_with_memory() and supplying a pointer to memory that
    contains the whole file.

 3. Using plm_create_with_buffer(), supplying your own plm_buffer_t instance and
    periodically writing to this buffer.

When using your own plm_buffer_t instance, you can fill this buffer using 
plm_buffer_write(). You can either monitor plm_buffer_get_remaining() and push 
data when appropriate, or install a callback on the buffer with 
plm_buffer_set_load_callback() that gets called whenever the buffer needs more 
data.

A buffer created with plm_buffer_create_with_capacity() is treated as a ring
buffer, meaning that data that has already been read, will be discarded. In
contrast, a buffer created with plm_buffer_create_for_appending() will keep all
data written to it in memory. This enables seeking in the already loaded data.


There should be no need to use the lower level plm_video_*
functions, if all you want to do is read/decode an MPEG-PS file.
However, if you get raw mpeg1video data from a different
source, these functions can be used to decode the raw data directly.


This library uses malloc(), realloc() and free() to manage memory. Typically 
all allocation happens up-front when creating the interface. However, the
default buffer size may be too small for certain inputs. In these cases plmpeg
will realloc() the buffer with a larger size whenever needed. You can configure
the default buffer size by defining PLM_BUFFER_DEFAULT_SIZE *before* 
including this library.


See below for detailed the API documentation.

*/


#ifndef PL_MPEG_H
#define PL_MPEG_H

#include <stdint.h>
#include <stdio.h>

#include "float64.h"


// -----------------------------------------------------------------------------
// Public Data Types


// Object types for the various interfaces

typedef struct plm_buffer_t plm_buffer_t;
typedef struct plm_video_t plm_video_t;


// Decoded Video Plane 
// The byte length of the data is width * height. Note that different planes
// have different sizes: the Luma plane (Y) is double the size of each of 
// the two Chroma planes (Cr, Cb) - i.e. 4 times the byte length.
// Also note that the size of the plane does *not* denote the size of the 
// displayed frame. The sizes of planes are always rounded up to the nearest
// macroblock (16px).

typedef struct {
	unsigned int width;
	unsigned int height;
	uint8_t *data;
} plm_plane_t;


// Decoded Video Frame
// width and height denote the desired display size of the frame. This may be
// different from the internal size of the 3 planes.

typedef struct {
	float64 time;
	unsigned int width;
	unsigned int height;
	plm_plane_t y;
	plm_plane_t cr;
	plm_plane_t cb;
} plm_frame_t;


// Callback function for plm_buffer when it needs more data
typedef void(*plm_buffer_load_callback)(plm_buffer_t *self, void *user);



// -----------------------------------------------------------------------------
// plm_buffer public API
// Provides the data source for all other plm_* interfaces


// The default size for buffers created from files or by the high-level API

#ifndef PLM_BUFFER_DEFAULT_SIZE
#define PLM_BUFFER_DEFAULT_SIZE (128 * 1024)
#endif


// Create a buffer instance with a filename. Returns NULL if the file could not
// be opened.

plm_buffer_t *plm_buffer_create_with_filename(const char *filename);


// Create a buffer instance with a file handle. Pass TRUE to close_when_done
// to let plmpeg call fclose() on the handle when plm_destroy() is called.

plm_buffer_t *plm_buffer_create_with_file(FILE *fh, int close_when_done);


// Create a buffer instance with a pointer to memory as source. This assumes
// the whole file is in memory. The bytes are not copied. Pass 1 to 
// free_when_done to let plmpeg call free() on the pointer when plm_destroy() 
// is called.

plm_buffer_t *plm_buffer_create_with_memory(uint8_t *bytes, size_t length, int free_when_done);


// Create an empty buffer with an initial capacity. The buffer will grow
// as needed. Data that has already been read, will be discarded.

plm_buffer_t *plm_buffer_create_with_capacity(size_t capacity);


// Create an empty buffer with an initial capacity. The buffer will grow
// as needed. Decoded data will *not* be discarded. This can be used when
// loading a file over the network, without needing to throttle the download. 
// It also allows for seeking in the already loaded data.

plm_buffer_t *plm_buffer_create_for_appending(size_t initial_capacity);


// Destroy a buffer instance and free all data

void plm_buffer_destroy(plm_buffer_t *self);


// Copy data into the buffer. If the data to be written is larger than the 
// available space, the buffer will realloc() with a larger capacity. 
// Returns the number of bytes written. This will always be the same as the
// passed in length, except when the buffer was created _with_memory() for
// which _write() is forbidden.

size_t plm_buffer_write(plm_buffer_t *self, uint8_t *bytes, size_t length);


// Mark the current byte length as the end of this buffer and signal that no 
// more data is expected to be written to it. This function should be called
// just after the last plm_buffer_write().
// For _with_capacity buffers, this is cleared on a plm_buffer_rewind().

void plm_buffer_signal_end(plm_buffer_t *self);


// Set a callback that is called whenever the buffer needs more data

void plm_buffer_set_load_callback(plm_buffer_t *self, plm_buffer_load_callback fp, void *user);


// Rewind the buffer back to the beginning. When loading from a file handle,
// this also seeks to the beginning of the file.

void plm_buffer_rewind(plm_buffer_t *self);


// Get the total size. For files, this returns the file size. For all other 
// types it returns the number of bytes currently in the buffer.

size_t plm_buffer_get_size(plm_buffer_t *self);


// Get the number of remaining (yet unread) bytes in the buffer. This can be
// useful to throttle writing.

size_t plm_buffer_get_remaining(plm_buffer_t *self);


// Get whether the read position of the buffer is at the end and no more data 
// is expected.

int plm_buffer_has_ended(plm_buffer_t *self);


// Get the amount of free space in the buffer, useful before calling plm_buffer_write()

size_t plm_buffer_get_available_space(plm_buffer_t *self);

// -----------------------------------------------------------------------------
// plm_video public API
// Decode MPEG1 Video ("mpeg1") data into raw YCrCb frames


// Create a video decoder with a plm_buffer as source.
plm_video_t *plm_video_create_with_buffer(plm_buffer_t *buffer, int destroy_when_done);


// Destroy a video decoder and free all data.
void plm_video_destroy(plm_video_t *self);


// Search the video sequence header in the buffer necessary to
// known the dimensions and framerate.
int plm_video_has_header(plm_video_t *self);


// Get the framerate in frames per second.

float64 plm_video_get_framerate(plm_video_t *self);


// Get the display width/height.

int plm_video_get_width(plm_video_t *self);
int plm_video_get_height(plm_video_t *self);


// Set "no delay" mode. When enabled, the decoder assumes that the video does
// *not* contain any B-Frames. This is useful for reducing lag when streaming.
// The default is FALSE.
void plm_video_set_no_delay(plm_video_t *self, int no_delay);


// Get the current internal time in seconds.
float64 plm_video_get_time(plm_video_t *self);


// Set the current internal time in seconds. This is only useful when you
// manipulate the underlying video buffer and want to enforce a correct
// timestamps.
void plm_video_set_time(plm_video_t *self, float64 time);


// Rewind the internal buffer. See plm_buffer_rewind().
void plm_video_rewind(plm_video_t *self);


// Get whether the file has ended. This will be cleared on rewind.
int plm_video_has_ended(plm_video_t *self);


// Decode and return one frame of video and advance the internal time by 
// 1/framerate seconds. The returned frame_t is valid until the next call of
// plm_video_decode() or until the video decoder is destroyed.
plm_frame_t *plm_video_decode(plm_video_t *self);

// Get the amount of bytes required to allocate each YCrCb planes
void plm_video_get_planes_byte_size(plm_video_t *self, size_t* luma_plane_size, size_t* chroma_plane_size);

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define PLM_UNUSED(expr) (void)(expr)


enum plm_buffer_mode {
	PLM_BUFFER_MODE_FILE,
	PLM_BUFFER_MODE_FIXED_MEM,
	PLM_BUFFER_MODE_RING,
	PLM_BUFFER_MODE_APPEND
};

struct plm_buffer_t {
	size_t bit_index;
	size_t capacity;
	size_t length;
	size_t total_size;
	int discard_read_bytes;
	int has_ended;
	int free_when_done;
	int close_when_done;
	FILE *fh;
	plm_buffer_load_callback load_callback;
	void *load_callback_user_data;
	uint8_t *bytes;
	enum plm_buffer_mode mode;
};

typedef struct {
	short index;
	short value;
} plm_vlc_t;

typedef struct {
	short index;
	unsigned short value;
} plm_vlc_uint_t;

#endif // PL_MPEG_H

