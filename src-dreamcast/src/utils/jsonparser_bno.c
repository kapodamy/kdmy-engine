#include "jsonparser.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include "fs.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"
#include "vertexprops.h"


#define GET_VALUE_PTR(item, c_type) (*((c_type*)item->data))
#define GET_STRING_VALUE_PTR(item) ((const char*)item->data)

//
// the type stored in the BNO is 64bit floating-point
//
#if __SIZEOF_FLOAT__ == __SIZEOF_DOUBLE__
typedef long double BNO_Double;
#else
typedef double BNO_Double;
#endif

typedef struct __attribute__((__packed__)) {
    const BNO_Type type;
    const char* name;
} BNO_TypeName;

static const BNO_TypeName BNO_TYPES_NAMES[] = {
    {BNO_Type_Null, "Null"},
    {BNO_Type_NumberDouble, "Number(double)"},
    {BNO_Type_NumberLong, "Number(long)"},
    {BNO_Type_String, "String"},
    {BNO_Type_Boolean, "Boolean"},
    {BNO_Type_Array, "Array"},
    {BNO_Type_Object, "Object"},
    {BNO_Type_EOF, NULL},
};

typedef struct {
    uint32_t value_length;
    uint32_t accumulated_length;
    JSONTokenIterator iterator;
    bool (*iterator_fn_ptr)(JSONTokenIterator*, JSONTokenValue*);
} BNOJsonStack;


static bool value_integrity_check(const BNO_Entry* entry, const uint8_t* root_end) {
    const uint8_t* end = entry->data + entry->value_length;
    if (end > root_end) {
        // truncated BNO
        return false;
    }

    uint32_t expected_value_length;
    switch (entry->type) {
        case BNO_Type_Null:
            expected_value_length = 0;
            break;
        case BNO_Type_NumberDouble:
            expected_value_length = sizeof(BNO_Double);
            break;
        case BNO_Type_NumberLong:
            expected_value_length = sizeof(int64_t);
            break;
        case BNO_Type_String:
            if (entry->value_length < 1 || entry->data[entry->value_length - 1] != '\0') {
                // missing null-terminator
                return false;
            }
            return true;
        case BNO_Type_Boolean:
            expected_value_length = sizeof(uint8_t);
            break;
        default:
            // unknown type
            return false;
    }

    if (expected_value_length != entry->value_length) {
        // unexpected value_length
        return false;
    }

    return true;
}

static bool integrity_check(const BNO_Entry* root, size_t length) {
    size_t stack_length = 32;
    ssize_t stack_index = 0;
    BNOJsonStack* stack = malloc_for_array(BNOJsonStack, stack_length);

    BNOJsonStack* current;
    const BNO_Entry* current_entry = root;
    JSONTokenValue token_value;
    bool success = false;
    uint8_t* root_end = (uint8_t*)root + length;

L_pick_iterators:
    current = stack + stack_index;
    *current = (BNOJsonStack){
        .value_length = current_entry->value_length,
        .accumulated_length = 0
    };

    if (current_entry->type == BNO_Type_Object) {
        current->iterator = json_iterator_get_object_properties(current_entry),
        current->iterator_fn_ptr = json_iterate_object;
    } else if (current_entry->type == BNO_Type_Array) {
        current->iterator = json_iterator_get_array_items(current_entry);
        current->iterator_fn_ptr = json_iterate_array;
    } else {
        // the whole json is a single value
        success = value_integrity_check(current_entry, root_end);
        goto L_return;
    }

    if (((uint8_t*)current_entry - (uint8_t*)root) > length) {
        logger_error("json: BNO integrity check failed, out-of-bounds offset: %u", (size_t)((uint8_t*)current_entry - (uint8_t*)root));
        goto L_return;
    }

    current->value_length = current_entry->value_length;
    current->accumulated_length = 0;
    stack_index++;

    while (stack_index > 0) {
        BNOJsonStack* current = stack + (stack_index - 1);
        const BNO_EntryName* entry_name = (BNO_EntryName*)current->iterator.ptr;

        if (!current->iterator_fn_ptr(&current->iterator, &token_value)) {
            if (current->accumulated_length != current->value_length) {
                // invalid value length
                goto L_return;
            }

            stack_index--;
            continue;
        }

        if (token_value.property_name) {
            current->accumulated_length += sizeof(BNO_EntryName) + entry_name->name_length;

            if (entry_name->name_length < 1 || entry_name->name[entry_name->name_length - 1] != '\0') {
                // missing null-terminator
                goto L_return;
            }
        }

        const BNO_Entry* value = token_value.token;
        current->accumulated_length += sizeof(BNO_Entry) + value->value_length;

        switch (token_value.value_type) {
            case BNO_Type_Object:
            case BNO_Type_Array:
                if ((stack_index + 1) > stack_length) {
                    // deep json tree, increase stack length
                    stack_length += 32;
                    stack = realloc_for_array(stack, stack_length, BNOJsonStack);
                }

                current_entry = token_value.token;
                goto L_pick_iterators;
            default:
                break;
        }


        if (!value_integrity_check(value, root_end)) {
            goto L_return;
        }
    }

    success = true;

L_return:
    free_chk(stack);
    return success;
}

static void print_unexpected_type_error(const char* prop_name, int32_t item_index, const BNO_Entry* entry, BNO_Type expected) {
    const char* entry_type_name = NULL;
    const char* expected_type_name = NULL;

    for (size_t i = 0; BNO_TYPES_NAMES[i].name; i++) {
        if (entry->type == BNO_TYPES_NAMES[i].type)
            entry_type_name = BNO_TYPES_NAMES[i].name;
        else if (expected == BNO_TYPES_NAMES[i].type)
            expected_type_name = BNO_TYPES_NAMES[i].name;
    }

    if (prop_name)
        logger_warn("json: expected %s at '%s', found %s.", expected_type_name, entry_type_name, prop_name);
    else
        logger_warn("json: expected %s at index %i, found %s.", expected_type_name, (ssize_t)item_index, entry_type_name);
}


static const BNO_Entry* get_object_prop(const BNO_Entry* entry, const char* prop_name) {
    if (!prop_name) return NULL;

    if (entry->type != BNO_Type_Object) {
        // not a object
        return NULL;
    }

    // find property
    size_t prop_name_length = strlen(prop_name) + 1;
    const uint8_t* ptr = entry->data;
    const uint8_t* ptr_end = ptr + entry->value_length;

    while (ptr < ptr_end) {
        const BNO_EntryName* entry_name = (const BNO_EntryName*)ptr;
        ptr += sizeof(BNO_EntryName) + entry_name->name_length;

        if (prop_name_length == entry_name->name_length && memcmp(entry_name->name, prop_name, prop_name_length) == 0) {
            return (const BNO_Entry*)ptr;
        }

        const BNO_Entry* property = (const BNO_Entry*)ptr;

        ptr += sizeof(BNO_Entry) + property->value_length;
    }

    return NULL;
}

static const BNO_Entry* get_object_prop2(const BNO_Entry* entry, const char* prop_name, BNO_Type prop_type) {
    if (!entry) return NULL;

    const BNO_Entry* prop = get_object_prop(entry, prop_name);
    if (!prop) return NULL;

    if (prop->type != prop_type) {
        if (prop_type == BNO_Type_String && prop->type == BNO_Type_Null) {
            return NULL;
        }
        print_unexpected_type_error(prop_name, -1, prop, prop_type);
        return NULL;
    }

    return prop;
}

static bool has_object_prop(const BNO_Entry* entry, const char* prop_name, BNO_Type prop_type) {
    if (!entry) return NULL;

    JSONToken prop = get_object_prop(entry, prop_name);

    if (prop && prop->type == prop_type) {
        return true;
    }

    return false;
}

static bool has_object_prop2(const BNO_Entry* entry, const char* prop_name, BNO_Type prop_type1, BNO_Type prop_type2) {
    if (!entry) return NULL;

    JSONToken prop = get_object_prop(entry, prop_name);

    if (prop && (prop->type == prop_type1 || prop->type == prop_type2)) {
        return true;
    }

    return false;
}


static const BNO_Entry* get_array_item(const BNO_Entry* entry, int32_t index) {
    if (!entry || index < 0) {
        return NULL;
    }

    if (entry->type != BNO_Type_Array) {
        // not an array
        return NULL;
    }

    // seek to item
    const uint8_t* ptr = entry->data;
    const uint8_t* ptr_end = ptr + entry->value_length;

    for (int32_t i = 0; ptr < ptr_end; i++) {
        const BNO_Entry* item = (BNO_Entry*)ptr;

        if (i == index) {
            return (JSONToken)ptr;
        }

        ptr += sizeof(BNO_Entry) + item->value_length;
    }

    return NULL;
}

static const BNO_Entry* get_array_item2(const BNO_Entry* entry, int32_t index, BNO_Type item_type) {
    JSONToken item = get_array_item(entry, index);
    if (!item) return NULL;

    if (item->type != item_type) {
        print_unexpected_type_error(NULL, index, item, item_type);
        return NULL;
    }

    return item;
}


static inline float64 get_fp64_value(const BNO_Entry* entry) {
    BNO_Double value = GET_VALUE_PTR(entry, BNO_Double);
    return (float64)value;
}


static const BNO_Entry* json_internal_load_from_buffer(ArrayBuffer arraybuffer) {
    BNO_Header* bno = (BNO_Header*)arraybuffer->data;

    if (arraybuffer->length < sizeof(BNO_Header) || bno->signature != BNO_SIGNATURE) {
        logger_error("json_internal_load_from_buffer() failed, expected BNO file.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

    if (bno->content != BNO_CONTENT_JSON) {
        logger_error("json_internal_load_from_buffer() failed, the bno contents is not JSON.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

    const BNO_Entry* root = (const BNO_Entry*)bno->data;

    // do integrity check, to avoid out-of-bounds access
    if (!integrity_check(root, arraybuffer->length)) {
        logger_error("json_internal_load_from_buffer() failed, malformed JSON contents.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

#ifdef DEBUG
    const uint8_t* ptr = (const uint8_t*)root - sizeof(BNO_Header) - sizeof(struct ArrayBuffer_s);
    assert((void*)ptr == (void*)arraybuffer);
#endif

    return (const BNO_Entry*)root;
}



JSONToken json_load_from(const char* src) {
    ArrayBuffer arraybuffer = fs_readarraybuffer(src);

    if (!arraybuffer) {
        return NULL;
    }

    return json_internal_load_from_buffer(arraybuffer);
}

JSONToken json_load_from_string(const char* json_sourcecode) {
    if (!json_sourcecode) return NULL;

    logger_error("json_load_from_string() failed, parse JSON from string is not available on the current platform.");
    return NULL;
}

JSONToken json_load_direct(const char* src) {
    FILE* file = fopen(src, "rb");
    if (!file) {
        logger_error("json_load_direct() failed to open the file %s", src);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 1) {
        fclose(file);
        return NULL;
    }

    ArrayBuffer arraybuffer = arraybuffer_init((size_t)size);
    assert(arraybuffer);

    fread((uint8_t*)arraybuffer->data, sizeof(uint8_t), arraybuffer->length, file);
    fclose(file);

    return json_internal_load_from_buffer(arraybuffer);
}

void json_destroy(JSONToken* json) {
    if (!*json) return;

    uint8_t* ptr = (uint8_t*)*json;
    ptr -= sizeof(BNO_Header);
    ptr -= sizeof(struct ArrayBuffer_s);

    ArrayBuffer arraybuffer = (ArrayBuffer)ptr;
    arraybuffer_destroy(&arraybuffer);

    *json = NULL;
}


JSONToken json_read_object(JSONToken json, const char* name) {
    return get_object_prop2(json, name, BNO_Type_Object);
}

JSONToken json_read_array(JSONToken json, const char* name) {
    return get_object_prop2(json, name, BNO_Type_Array);
}

int32_t json_read_array_length(JSONToken json_array) {
    if (!json_array) {
        return -1;
    }

    if (json_array->type != BNO_Type_Array) {
        // not an array
        return -1;
    }

    // count items on the array
    int32_t count = 0;
    const uint8_t* ptr = json_array->data;
    const uint8_t* ptr_end = ptr + json_array->value_length;

    while (ptr < ptr_end) {
        const BNO_Entry* item = (BNO_Entry*)ptr;
        ptr += sizeof(BNO_Entry) + item->value_length;
        count++;
    }

    return count;
}

float64 json_read_array_item_number_double(JSONToken json_array, int32_t index, float64 default_value) {
    const BNO_Entry* item = get_array_item(json_array, index);
    if (!item) return default_value;

    if (item->type == BNO_Type_NumberLong) {
        return GET_VALUE_PTR(item, int64_t);
    } else if (item->type == BNO_Type_NumberDouble) {
        return get_fp64_value(item);
    }

    logger_warn("json: expected number in %i.", (ssize_t)index);
    return default_value;
}

int64_t json_read_array_item_number_long(JSONToken json_array, int32_t index, int64_t default_value) {
    const BNO_Entry* item = get_array_item2(json_array, index, BNO_Type_NumberLong);
    if (!item) return default_value;

    return GET_VALUE_PTR(item, int64_t);
}

bool json_read_array_item_boolean(JSONToken json_array, int32_t index, bool default_value) {
    const BNO_Entry* item = get_array_item2(json_array, index, BNO_Type_Boolean);
    if (!item) return default_value;

    return GET_VALUE_PTR(item, uint8_t) > 0 ? true : false;
}

const char* json_read_array_item_string(JSONToken json_array, int32_t index, const char* default_value) {
    const BNO_Entry* item = get_array_item2(json_array, index, BNO_Type_String);
    if (!item) return default_value;

    return GET_STRING_VALUE_PTR(item);
}

uint32_t json_read_array_item_hex(JSONToken json_array, int32_t index, uint32_t default_value) {
    const BNO_Entry* item = get_array_item(json_array, index);
    if (!item) return default_value;

    if (item->type == BNO_Type_NumberLong) {
        return (uint32_t)GET_VALUE_PTR(item, int64_t);
    } else if (item->type == BNO_Type_String) {
        const char* value = GET_STRING_VALUE_PTR(item);
        uint32_t val;
        if (vertexprops_parse_hex(value, &val, false)) {
            return val;
        }
    }

    logger_warn("json: expected hexadecimal number in %i.", (ssize_t)index);
    return default_value;
}

JSONToken json_read_array_item_object(JSONToken json_array, int32_t index) {
    return get_array_item2(json_array, index, BNO_Type_Object);
}

JSONToken json_read_array_item_array(JSONToken json_array, int32_t index) {
    return get_array_item2(json_array, index, BNO_Type_Array);
}


float64 json_read_number_double(JSONToken json, const char* name, float64 default_value) {
    const BNO_Entry* prop = get_object_prop(json, name);
    if (!prop) return default_value;

    if (prop->type == BNO_Type_NumberLong) {
        return GET_VALUE_PTR(prop, int64_t);
    } else if (prop->type == BNO_Type_NumberDouble) {
        return get_fp64_value(prop);
    }

    logger_warn("json: expected number in %s.", name);
    return default_value;
}

int64_t json_read_number_long(JSONToken json, const char* name, int64_t default_value) {
    const BNO_Entry* prop = get_object_prop2(json, name, BNO_Type_NumberLong);
    if (!prop) return default_value;

    return GET_VALUE_PTR(prop, int64_t);
}

bool json_read_boolean(JSONToken json, const char* name, bool default_value) {
    const BNO_Entry* prop = get_object_prop2(json, name, BNO_Type_Boolean);
    if (!prop) return default_value;

    return GET_VALUE_PTR(prop, uint8_t) > 0 ? true : false;
}

const char* json_read_string(JSONToken json, const char* name, const char* default_value) {
    const BNO_Entry* prop = get_object_prop2(json, name, BNO_Type_String);
    if (!prop) return default_value;

    return GET_STRING_VALUE_PTR(prop);
}

uint32_t json_read_hex(JSONToken json, const char* name, uint32_t default_value) {
    const BNO_Entry* prop = get_object_prop(json, name);
    if (!prop) return default_value;

    if (prop->type == BNO_Type_NumberLong) {
        return (uint32_t)GET_VALUE_PTR(prop, int64_t);
    } else if (prop->type == BNO_Type_String) {
        const char* value = GET_STRING_VALUE_PTR(prop);
        uint32_t val;
        if (vertexprops_parse_hex(value, &val, false)) {
            return val;
        }
    }

    logger_warn("json: expected hexadecimal number in %s.", name);
    return default_value;
}


bool json_has_property(JSONToken json, const char* name) {
    return get_object_prop(json, name) != NULL;
}


bool json_has_property_boolean(JSONToken json, const char* name) {
    return has_object_prop(json, name, BNO_Type_Boolean);
}

bool json_has_property_string(JSONToken json, const char* name) {
    return has_object_prop2(json, name, BNO_Type_String, BNO_Type_Null);
}

bool json_has_property_number_long(JSONToken json, const char* name) {
    return has_object_prop(json, name, BNO_Type_NumberLong);
}

bool json_has_property_number_double(JSONToken json, const char* name) {
    return has_object_prop2(json, name, BNO_Type_NumberDouble, BNO_Type_NumberLong);
}

bool json_has_property_array(JSONToken json, const char* name) {
    return has_object_prop(json, name, BNO_Type_Array);
}

bool json_has_property_object(JSONToken json, const char* name) {
    return has_object_prop(json, name, BNO_Type_Object);
}

bool json_has_property_hex(JSONToken json, const char* name) {
    return has_object_prop2(json, name, BNO_Type_String, BNO_Type_NumberLong);
}


bool json_is_property_null(JSONToken json, const char* name) {
    return has_object_prop(json, name, BNO_Type_Null);
}

bool json_is_array_item_null(JSONToken json_array, int32_t index) {
    const BNO_Entry* item = get_array_item(json_array, index);

    return item && item->type == BNO_Type_Null;
}

JSONTokenType json_get_value_type(JSONToken json_object_property_or_array_item) {
    if (json_object_property_or_array_item) {
        return (JSONTokenType)json_object_property_or_array_item->type;
    }

    return JSONTokenType_Unknown;
}

JSONTokenType json_get_array_item_type(JSONToken json_array, int32_t index) {
    const BNO_Entry* item = get_array_item(json_array, index);

    return json_get_value_type(item);
}


JSONTokenIterator json_iterator_get_object_properties(JSONToken json) {
    if (!json || json->type != BNO_Type_Object) {
        return (JSONTokenIterator){
            .next_array_index = -1,
            .ptr = NULL,
            .ptr_end = NULL
        };
    }

    return (JSONTokenIterator){
        .next_array_index = -1,
        .ptr = (void*)json->data,
        .ptr_end = json->data + json->value_length
    };
}

bool json_iterate_object(JSONTokenIterator* iterator, JSONTokenValue* property) {
    assert(iterator->next_array_index < 0);

    if (iterator->ptr >= iterator->ptr_end) {
#if DEBUG
        assert(iterator->ptr == iterator->ptr_end);
#endif
        memset(property, 0x00, sizeof(JSONTokenValue));
        return false;
    }

    const uint8_t* ptr = iterator->ptr;
    const BNO_EntryName* entry_name = (BNO_EntryName*)ptr;

    ptr = (const uint8_t*)entry_name->name + entry_name->name_length;
    const BNO_Entry* entry = (BNO_Entry*)ptr;

    property->array_index = -1;
    property->property_name = entry_name->name;
    property->value_type = (JSONTokenType)entry->type;
    property->value_pointer = (void*)entry->data;
    property->token = entry;

    iterator->ptr = (void*)(entry->data + entry->value_length);
    return true;
}

JSONTokenIterator json_iterator_get_array_items(JSONToken json_array) {
    if (!json_array || json_array->type != BNO_Type_Array) {
        return (JSONTokenIterator){
            .next_array_index = 0,
            .ptr = NULL,
            .ptr_end = NULL
        };
    }

    return (JSONTokenIterator){
        .next_array_index = 0,
        .ptr = (void*)json_array->data,
        .ptr_end = json_array->data + json_array->value_length
    };
}

bool json_iterate_array(JSONTokenIterator* iterator, JSONTokenValue* item) {
    assert(iterator->next_array_index >= 0);

    if (iterator->ptr >= iterator->ptr_end) {
#if DEBUG
        assert(iterator->ptr <= iterator->ptr_end);
#endif
        memset(item, 0x00, sizeof(JSONTokenValue));
        return false;
    }

    const BNO_Entry* entry = (BNO_Entry*)iterator->ptr;

    item->property_name = NULL;
    item->array_index = iterator->next_array_index++;
    item->value_type = (JSONTokenType)entry->type;
    item->value_pointer = (void*)entry->data;
    item->token = entry;

    iterator->ptr = (void*)(entry->data + entry->value_length);
    return true;
}


JSONTokenValue json_get_root_as_token_value(JSONToken json) {
    if (!json) {
        return (JSONTokenValue){
            .array_index = -1,
            .property_name = NULL,
            .value_type = JSONTokenType_Unknown,
            .value_pointer = NULL,
            .token = NULL
        };
    }

    return (JSONTokenValue){
        .array_index = -1,
        .property_name = NULL,
        .value_type = (JSONTokenType)json->type,
        .value_pointer = (void*)json->data,
        .token = json
    };
}


char* json_read_array_item_string2(JSONToken json_array, int32_t index, const char* default_value) {
    const char* str = json_read_array_item_string(json_array, index, default_value);
    return string_duplicate(str);
}

char* json_read_string2(JSONToken json, const char* name, const char* default_value) {
    const char* str = json_read_string(json, name, default_value);
    return string_duplicate(str);
}
