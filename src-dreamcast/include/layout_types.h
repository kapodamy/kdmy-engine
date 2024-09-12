#ifndef _layout_types_h
#define _layout_types_h

#include <stdbool.h>

#include "pvrcontext_types.h"
#include "vertexprops_types.h"
#include "float64.h"


#define LYT_T(FIELD_SUFFIX, VALUE) ((LayoutAttachedValue){.as_##FIELD_SUFFIX = VALUE})


//
//  forward reference
//

typedef struct AnimSprite_s* AnimSprite;
typedef struct Layout_s* Layout;
typedef struct Drawable_s* Drawable;


typedef struct {
    float x;
    float y;
    float z;
} LayoutParallax;

typedef struct LayoutPlaceholder_s {
    PVRVertex type;
    int32_t group_id;
    char* name;
    Align align_vertical;
    Align align_horizontal;
    float x;
    float y;
    float z;
    float height;
    float width;
    LayoutParallax parallax;
    bool static_camera;
    Drawable vertex;
} LayoutPlaceholder;

typedef struct {
    char* name;
    bool enable_offset_zoom;
    bool has_duration;
    bool duration_in_beats;
    float duration;
    bool move_only;
    bool has_from;
    bool is_empty;
    bool has_parallax_offset_only;
    bool move_offset_only;
    bool has_offset_from;
    bool has_offset_to;
    float to_offset_x;
    float to_offset_y;
    float to_offset_z;
    float from_offset_x;
    float from_offset_y;
    float from_offset_z;
    AnimSprite animation;
    float from_x, from_y, from_z;
    float to_x, to_y, to_z;
    float offset_x, offset_y, offset_z;
} CameraPlaceholder;

typedef enum {
    AttachedValueType_NOTFOUND = 0x00,
    AttachedValueType_STRING = 0x01,
    AttachedValueType_FLOAT = 0x02,
    AttachedValueType_INTEGER = 0x04,
    AttachedValueType_HEX = 0x08, // unsigned integer
    AttachedValueType_BOOLEAN = 0x10
} AttachedValueType;

typedef union {
    float64 as_double;
    int64_t as_long;
    uint32_t as_unsigned;
    const char* as_string;
    bool as_boolean;
} LayoutAttachedValue;

#endif
