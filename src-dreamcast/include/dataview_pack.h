#ifndef _dataview_pack_h
#define _dataview_pack_h

#include <stdint.h>


union Packed4bytes {
    int32_t value_int;
    uint32_t value_uint;

    int8_t value_sbyte;
    uint8_t value_byte;

    int16_t value_short;
    uint16_t value_ushort;

    bool value_bool;

    float value_float;
};

#endif
