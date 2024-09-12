#ifndef _bno_h
#define _bno_h

#include <stdint.h>

#define BNO__NAME_MAKE(s) (uint32_t)(s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24))
#define BNO_SIGNATURE BNO__NAME_MAKE("bno\x20")
#define BNO_CONTENT_JSON BNO__NAME_MAKE("json")
#define BNO_CONTENT_XML BNO__NAME_MAKE("xml\x20")

#define BNO_MAX_ATTRIBUTES 127
#define BNO_MAX_NODE_SIZE INT32_MAX

typedef enum __attribute__((__packed__)) {
    BNO_Type_EOF = 0,
    BNO_Type_Null = 1,
    BNO_Type_NumberDouble = 2,
    BNO_Type_NumberLong = 3,
    BNO_Type_String = 4,
    BNO_Type_Boolean = 5,
    BNO_Type_Array = 6,
    BNO_Type_Object = 7
} BNO_Type;

typedef struct __attribute__((__packed__)) {
    uint32_t signature;
    uint32_t content;
    const uint8_t data[];
} BNO_Header;

typedef struct __attribute__((__packed__)) {
    uint8_t name_length;
    const char name[];
} BNO_EntryName;

typedef struct __attribute__((__packed__)) {
    BNO_Type type;
    uint32_t value_length;
    const uint8_t data[];
    // uint8_t value[value_length];
} BNO_Entry;

typedef struct __attribute__((__packed__)) {
    BNO_Type type;
    uint8_t name_length;
    uint8_t attributes_count;
    uint16_t attributes_length;
    uint32_t value_length;
    const uint8_t data[];
    // char* text[name_length];
    // BNO_Attribute attributes[attributes_length];
    // uint8_t value[value_length];
} BNO_Node;

typedef struct __attribute__((__packed__)) {
    uint8_t name_length;
    uint8_t value_length;
    const char data[];
} BNO_Attribute;

#endif
