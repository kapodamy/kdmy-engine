#ifndef _vertexprops_h
#define _vertexprops_h

#include <stdbool.h>
#include <stdint.h>

#include "nbool.h"
#include "float64.h"
#include "pvrcontext_types.h"
#include "vertexprops_types.h"
#include "xmlparser.h"


#define SPRITE_PROP_X 0
#define SPRITE_PROP_Y 1
#define SPRITE_PROP_WIDTH 2
#define SPRITE_PROP_HEIGHT 3
#define SPRITE_PROP_ROTATE 4
#define SPRITE_PROP_SCALE_X 5
#define SPRITE_PROP_SCALE_Y 6
#define SPRITE_PROP_SKEW_X 7
#define SPRITE_PROP_SKEW_Y 8
#define SPRITE_PROP_TRANSLATE_X 9
#define SPRITE_PROP_TRANSLATE_Y 10
#define SPRITE_PROP_ALPHA 11
#define SPRITE_PROP_Z 12
#define SPRITE_PROP_VERTEX_COLOR_R 13
#define SPRITE_PROP_VERTEX_COLOR_G 14
#define SPRITE_PROP_VERTEX_COLOR_B 15
#define SPRITE_PROP_VERTEX_COLOR_OFFSET_R 16
#define SPRITE_PROP_VERTEX_COLOR_OFFSET_G 17
#define SPRITE_PROP_VERTEX_COLOR_OFFSET_B 18
#define SPRITE_PROP_VERTEX_COLOR_OFFSET_A 19
#define SPRITE_PROP_FLIP_X 20
#define SPRITE_PROP_FLIP_Y 21
#define SPRITE_PROP_ROTATE_PIVOT_ENABLED 22
#define SPRITE_PROP_ROTATE_PIVOT_U 23
#define SPRITE_PROP_ROTATE_PIVOT_V 24
#define SPRITE_PROP_SCALE_DIRECTION_X 25
#define SPRITE_PROP_SCALE_DIRECTION_Y 26
#define SPRITE_PROP_TRANSLATE_ROTATION 27
#define SPRITE_PROP_SCALE_SIZE 28

#define SPRITE_PROP_ANIMATIONLOOP 29 // reserved for layouts
#define SPRITE_PROP_FRAMEINDEX 30    // reserved for macroexecutors

#define TEXTSPRITE_PROP_FONT_SIZE 31
#define TEXTSPRITE_PROP_ALIGN_H 32
#define TEXTSPRITE_PROP_ALIGN_V 33
#define TEXTSPRITE_PROP_ALIGN_PARAGRAPH 34
#define TEXTSPRITE_PROP_FORCE_CASE 35
#define TEXTSPRITE_PROP_FONT_COLOR 36
#define TEXTSPRITE_PROP_MAX_LINES 37
#define TEXTSPRITE_PROP_MAX_WIDTH 38
#define TEXTSPRITE_PROP_MAX_HEIGHT 39
#define TEXTSPRITE_PROP_BORDER_ENABLE 40
#define TEXTSPRITE_PROP_BORDER_SIZE 41
#define TEXTSPRITE_PROP_BORDER_COLOR_R 42
#define TEXTSPRITE_PROP_BORDER_COLOR_G 43
#define TEXTSPRITE_PROP_BORDER_COLOR_B 44
#define TEXTSPRITE_PROP_BORDER_COLOR_A 45
#define TEXTSPRITE_PROP_PARAGRAPH_SEPARATION 46

#define SPRITE_PROP_Z_OFFSET 47
#define SPRITE_PROP_ANTIALIASING 48

#define MEDIA_PROP_VOLUME 49
#define MEDIA_PROP_SEEK 50
#define MEDIA_PROP_PLAYBACK 51

#define FONT_PROP_WORDBREAK 52
#define SPRITE_PROP_SCALE_TRANSLATION 53
#define SPRITE_PROP_FLIP_CORRECTION 54

#define LAYOUT_PROP_GROUP_VIEWPORT_X 55
#define LAYOUT_PROP_GROUP_VIEWPORT_Y 56
#define LAYOUT_PROP_GROUP_VIEWPORT_WIDTH 57
#define LAYOUT_PROP_GROUP_VIEWPORT_HEIGHT 58

#define TEXTSPRITE_PROP_BACKGROUND_ENABLED 59
#define TEXTSPRITE_PROP_BACKGROUND_SIZE 60
#define TEXTSPRITE_PROP_BACKGROUND_OFFSET_X 61
#define TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y 62
#define TEXTSPRITE_PROP_BACKGROUND_COLOR_R 63
#define TEXTSPRITE_PROP_BACKGROUND_COLOR_G 64
#define TEXTSPRITE_PROP_BACKGROUND_COLOR_B 65
#define TEXTSPRITE_PROP_BACKGROUND_COLOR_A 66

#define CAMERA_PROP_OFFSET_X 67
#define CAMERA_PROP_OFFSET_Y 68
#define CAMERA_PROP_OFFSET_Z 69

#define SPRITE_PROP_ALPHA2 70

#define TEXTSPRITE_PROP_BORDER_OFFSET_X 71
#define TEXTSPRITE_PROP_BORDER_OFFSET_Y 72

#define CAMERA_PROP_OFFSET_ZOOM 73

#define TEXTSPRITE_PROP_STRING 74 // warning: string pointer. DO NOT USE IN MACROEXECUTOR

#define PLAYBACK_NONE 0
#define PLAYBACK_PLAY 1
#define PLAYBACK_PAUSE 2
#define PLAYBACK_STOP 3
#define PLAYBACK_MUTE 4
#define PLAYBACK_UNMUTE 5


int32_t vertexprops_parse_sprite_property(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_sprite_property2(const char* property);

int32_t vertexprops_parse_textsprite_property(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_textsprite_property2(const char* property);
int32_t vertexprops_parse_textsprite_forcecase(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_textsprite_forcecase2(const char* value);

int32_t vertexprops_parse_media_property(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_media_property2(const char* property);

int32_t vertexprops_parse_layout_property(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_layout_property2(const char* property);

int32_t vertexprops_parse_camera_property(XmlNode node, const char* name, bool warn);
int32_t vertexprops_parse_camera_property2(const char* property);

Align vertexprops_parse_align(XmlNode node, const char* name, bool warn_missing, bool reject_bothnone);
Align vertexprops_parse_align2(const char* value);

int32_t vertexprops_parse_playback(XmlNode node, const char* name, bool warn_missing);
int32_t vertexprops_parse_playback2(const char* value);

nbool vertexprops_parse_boolean(XmlNode node, const char* attr_name, nbool def_value);
nbool vertexprops_parse_boolean2(const char* value, nbool def_value);

int64_t vertexprops_parse_integer(XmlNode node, const char* attr_name, int64_t def_value);
int64_t vertexprops_parse_integer2(const char* value, int64_t def_value);

PVRFlag vertexprops_parse_flag(XmlNode node, const char* attr_name, PVRFlag def_value);
PVRFlag vertexprops_parse_flag2(const char* value, PVRFlag def_value);

uint32_t vertexprops_parse_unsigned_integer(const char* string, uint32_t default_value);

bool vertexprops_parse_hex(const char* string, uint32_t* output_value, bool only_if_prefixed);
uint32_t vertexprops_parse_hex2(const char* string, uint32_t default_value, bool only_if_prefixed);

int32_t vertexprops_parse_wordbreak(XmlNode node, const char* name, bool warn_missing);
int32_t vertexprops_parse_wordbreak2(const char* value);
float vertexprops_parse_float(XmlNode node, const char* name, float def_value);
float vertexprops_parse_float2(const char* value, float def_value);
bool vertexprops_is_property_enumerable(int32_t property_id);
bool vertexprops_is_property_boolean(int32_t property_id);
float64 vertexprops_parse_double(XmlNode node, const char* name, float64 def_value);
float64 vertexprops_parse_double2(const char* value, float64 def_value);
Blend vertexprops_parse_blending(const char* value);

bool vertexprops_is_integer(const char* value);

#endif
