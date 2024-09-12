#ifndef _jsonparser_h
#define _jsonparser_h

#include <stdbool.h>
#include <stdint.h>

#include "bno.h"
#include "float64.h"



typedef const BNO_Entry* JSONToken;

typedef enum {
    JSONTokenType_Unknown = BNO_Type_EOF,
    JSONTokenType_Null = BNO_Type_Null,
    JSONTokenType_NumberDouble = BNO_Type_NumberDouble,
    JSONTokenType_NumberLong = BNO_Type_NumberLong,
    JSONTokenType_String = BNO_Type_String,
    JSONTokenType_Boolean = BNO_Type_Boolean,
    JSONTokenType_Array = BNO_Type_Array,
    JSONTokenType_Object = BNO_Type_Object
} JSONTokenType;

typedef struct {
    int32_t next_array_index;
    void* ptr;
    const void* ptr_end;
} JSONTokenIterator;

typedef struct {
    const char* property_name;
    int32_t array_index;
    JSONTokenType value_type;
    JSONToken token;
    union {
        const float64* value_number_double;
        const int64_t* value_number_long;
        const uint8_t* value_boolean;
        const char* value_string;
        void* value_pointer;
    };
} JSONTokenValue;


JSONToken json_load_from(const char* src);
JSONToken json_load_from_string(const char* json_sourcecode);
JSONToken json_load_direct(const char* src);

void json_destroy(JSONToken* json);

JSONToken json_read_object(JSONToken json, const char* name);
JSONToken json_read_array(JSONToken json, const char* name);

int32_t json_read_array_length(JSONToken json_array);

float64 json_read_array_item_number_double(JSONToken json_array, int32_t index, float64 default_value);
int64_t json_read_array_item_number_long(JSONToken json_array, int32_t index, int64_t default_value);
bool json_read_array_item_boolean(JSONToken json_array, int32_t index, bool default_value);
const char* json_read_array_item_string(JSONToken json_array, int32_t index, const char* default_value);
uint32_t json_read_array_item_hex(JSONToken json_array, int32_t index, uint32_t default_value);
JSONToken json_read_array_item_object(JSONToken json_array, int32_t index);
JSONToken json_read_array_item_array(JSONToken json_array, int32_t index);

float64 json_read_number_double(JSONToken json, const char* name, float64 default_value);
int64_t json_read_number_long(JSONToken json, const char* name, int64_t default_value);
bool json_read_boolean(JSONToken json, const char* name, bool default_value);
const char* json_read_string(JSONToken json, const char* name, const char* default_value);
uint32_t json_read_hex(JSONToken json, const char* name, uint32_t default_value);

bool json_has_property(JSONToken json, const char* name);
bool json_has_property_boolean(JSONToken json, const char* name);
bool json_has_property_string(JSONToken json, const char* name);
bool json_has_property_number_long(JSONToken json, const char* name);
bool json_has_property_number_double(JSONToken json, const char* name);
bool json_has_property_array(JSONToken json, const char* name);
bool json_has_property_object(JSONToken json, const char* name);
bool json_has_property_hex(JSONToken json, const char* name);
bool json_is_property_null(JSONToken json, const char* name);

bool json_is_array_item_null(JSONToken json_array, int32_t index);
JSONTokenType json_get_value_type(JSONToken json_object_property_or_array_item);
JSONTokenType json_get_array_item_type(JSONToken json_array, int32_t index);

JSONTokenIterator json_iterator_get_object_properties(JSONToken json);
bool json_iterate_object(JSONTokenIterator* iterator, JSONTokenValue* property);
JSONTokenIterator json_iterator_get_array_items(JSONToken json_array);
bool json_iterate_array(JSONTokenIterator* iterator, JSONTokenValue* item);

JSONTokenValue json_get_root_as_token_value(JSONToken json);

/** @brief reads the string item and returns a copy. If not found returns a copy of the default value */
char* json_read_array_item_string2(JSONToken json_array, int32_t index, const char* default_value);
/** @brief reads the string property and returns a copy. If not found returns a copy of the default value */
char* json_read_string2(JSONToken json, const char* name, const char* default_value);

#endif
