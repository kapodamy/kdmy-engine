#ifndef _dataview_h
#define _dataview_h

#include <assert.h>
#include <stdint.h>

#include "dataview_pack.h"
#include "float64.h"


typedef struct {
    union {
        uint8_t* buffer_u8;
        int8_t* buffer_i8;
        uint16_t* buffer_u16;
        int16_t* buffer_i16;
        uint32_t* buffer_u32;
        int32_t* buffer_i32;
        uint64_t* buffer_u64;
        int64_t* buffer_i64;
        float* buffer_f32;
        float64* buffer_d64;
        union Packed4bytes* buffer_pack4;
    };
    uint8_t* end;
} DataView;


static inline void dataview_assert(DataView* dataview) {
    assert(dataview->buffer_u8 <= dataview->end);
}


#define dataview_from(buffer_pointer, offset, length) ((DataView){.buffer_u8 = (uint8_t*)buffer_pointer + offset, .end = (uint8_t*)buffer_pointer + offset + length})

#define dataview_read_u8(dataview) (*dataview.buffer_u8++)
#define dataview_read_i8(dataview) (*dataview.buffer_i8++)
#define dataview_read_u16(dataview) (*dataview.buffer_u16++)
#define dataview_read_i16(dataview) (*dataview.buffer_i16++)
#define dataview_read_u32(dataview) (*dataview.buffer_u32++)
#define dataview_read_i32(dataview) (*dataview.buffer_i32++)
#define dataview_read_u64(dataview) (*dataview.buffer_u64++)
#define dataview_read_i64(dataview) (*dataview.buffer_i64++)
#define dataview_read_f32(dataview) (*dataview.buffer_f32++)
#define dataview_read_f64(dataview) (*dataview.buffer_d64++)
#define dataview_read_pack4(dataview) (*dataview.buffer_pack4++)

#define dataview_skip(dataview, bytes) \
    { dataview.buffer_u8 += bytes; }

#define dataview_pointer(dataview) (dataview.buffer_u8)

#define dataview_assert(dataview) assert(dataview.buffer_u8 <= dataview.end)

#define dataview_write_u8(dataview, value) *dataview.buffer_u8++ = value
#define dataview_write_i8(dataview, value) *dataview.buffer_i8++ = value
#define dataview_write_u16(dataview, value) *dataview.buffer_u16++ = value
#define dataview_write_i16(dataview, value) *dataview.buffer_i16++ = value
#define dataview_write_u32(dataview, value) *dataview.buffer_u32++ = value
#define dataview_write_i32(dataview, value) *dataview.buffer_i32++ = value
#define dataview_write_u64(dataview, value) *dataview.buffer_u64++ = value
#define dataview_write_i64(dataview, value) *dataview.buffer_i64++ = value
#define dataview_write_f32(dataview, value) *dataview.buffer_f32++ = value
#define dataview_write_f64(dataview, value) *dataview.buffer_d64++ = value
#define dataview_write_pack4(dataview, value) *dataview.buffer_pack4++ = value

#endif
