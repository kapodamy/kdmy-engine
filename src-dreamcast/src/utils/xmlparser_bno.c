#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>

#include "bno.h"
#include "fs.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"
#include "xmlparser.h"


#define STR_PUTC(chr, str, counter) \
    {                               \
        if (str) *str++ = chr;      \
        counter++;                  \
    }
#define STR_PUTS(str, dst, counter)         \
    {                                       \
        size_t tmp##_length = strlen(str);  \
        if (dst) {                          \
            memcpy(dst, str, tmp##_length); \
            dst += tmp##_length;            \
        }                                   \
        counter += tmp##_length;            \
    }
#define STR_PUTSL(str, str_len, dst, counter) \
    {                                         \
        if (dst) {                            \
            memcpy(dst, str, str_len);        \
            dst += str_len;                   \
        }                                     \
        counter += str_len;                   \
    }
#define STR_CHECK_END(str, full_str_len) (str[(full_str_len) - 1]) != '\0'
#define MEM_CHECK(ptr, start, max_length, jump_label)               \
    {                                                               \
        if ((((uintptr_t)ptr) - ((uintptr_t)start)) > max_length) { \
            goto jump_label;                                        \
        }                                                           \
    }

typedef struct {
    const BNO_Node* node;
    const char* node_name;
    int32_t child_index;
    int32_t child_count;
    bool last_node_was_text;
} BNOStack;

typedef struct {
    int32_t child_index;
    int32_t child_count;
    const BNO_Node* node;
} BNOStack2;

typedef struct {
    uint64_t accumulated_size;
    int32_t child_index;
    int32_t child_count;
    const BNO_Node* node;
} BNOStack3;


static size_t outer_xml_escape_string(const char* str, char* out_string) {
    size_t written = 0, i = 0, last_i = 0;

    for (; str[i] != '\0'; i++) {
        const char* escaped = NULL;

        switch (str[i]) {
            case '"':
                escaped = "&quot;";
                break;
            case '\'':
                escaped = "&apos;";
                break;
            case '<':
                escaped = "&lt;";
                break;
            case '>':
                escaped = "&gt;";
                break;
            case '&':
                escaped = "&amp;";
                break;
            default:
                continue;
        }

        if (escaped) {
            if (last_i < i) {
                size_t len = i - last_i;
                STR_PUTSL(str + last_i, len, out_string, written);
                last_i = i;
            }
            STR_PUTS(escaped, out_string, written);
        }
    }

    if (last_i < i) {
        size_t len = i - last_i;
        STR_PUTSL(str + last_i, len, out_string, written);
    }

    return written;
}

static size_t outer_xml_write_header(const BNO_Node* node, char* out_string) {
    size_t bytes_written = 0;

    STR_PUTC('<', out_string, bytes_written);

    size_t ret = outer_xml_escape_string(xmlparser_get_tag_name(node), out_string);
    bytes_written += ret;
    if (out_string) out_string += ret;

    // write attributes
    for (uint8_t i = 0; i < node->attributes_count; i++) {
        XmlAttribute attr = xmlparser_get_attribute(node, i);

        STR_PUTC(' ', out_string, bytes_written);
        STR_PUTS(attr.name, out_string, bytes_written);
        STR_PUTS("=\"", out_string, bytes_written);
        STR_PUTS(attr.value, out_string, bytes_written);
        STR_PUTC('"', out_string, bytes_written);
    }

    STR_PUTC('>', out_string, bytes_written);

    return bytes_written;
}

static size_t outer_xml_write_padding(ssize_t level, char* out_string) {
    size_t bytes_written = 0;

    STR_PUTS("\r\n", out_string, bytes_written);

    while (level--)
        STR_PUTS("    ", out_string, bytes_written);

    return bytes_written;
}

static size_t outer_xml_stringify(const BNO_Node* root, char* out_string) {
    size_t ret, bytes_written = 0;

    // STR_PUTS("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n", out_string, bytes_written);

    size_t stack_length = 16;
    ssize_t stack_index = 0;
    BNOStack* stack = malloc_for_array(BNOStack, stack_length);

    ret = outer_xml_write_header(root, out_string);
    bytes_written += ret;
    if (out_string) out_string += ret;

    // add root to stack
    stack[stack_index] = (BNOStack){
        .child_index = 0,
        .child_count = (int32_t)xmlparser_get_children_count(root),
        .node = root,
        .node_name = xmlparser_get_tag_name(root),
        .last_node_was_text = xmlparser_is_node_text(root)
    };
    stack_index++;

    XmlNode node_parent;
    BNOStack* parent;

L_Process:
    while (stack_index > 0) {
        parent = stack + (stack_index - 1);
        node_parent = parent->node;

        for (; parent->child_index < parent->child_count; parent->child_index++) {
            XmlNode node = xmlparser_get_children_at(node_parent, parent->child_index);

            if (xmlparser_is_node_text(node)) {
                parent->last_node_was_text = true;
                const char* str = xmlparser_get_text(node);
                STR_PUTS(str, out_string, bytes_written);
            } else {
                ssize_t new_stack_index = stack_index + 1;
                if (new_stack_index >= stack_length) {
                    // this never should happen (XML is too big)
                    stack_length += 16;
                    stack = realloc_for_array(stack, stack_length, BNOStack);
                }

                // obligatory
                parent->child_index++;

                // write node header
                if (!parent->last_node_was_text) {
                    ret = outer_xml_write_padding(stack_index, out_string);
                    bytes_written += ret;
                    if (out_string) out_string += ret;
                }

                ret = outer_xml_write_header(node, out_string);
                bytes_written += ret;
                if (out_string) out_string += ret;

                parent->last_node_was_text = false;

                // change parent and write current node childrens
                stack[stack_index] = (BNOStack){
                    .child_index = 0,
                    .child_count = xmlparser_get_children_count(node),
                    .node = node,
                    .node_name = xmlparser_get_tag_name(node),
                    .last_node_was_text = false
                };

                stack_index = new_stack_index;
                parent = stack + stack_index;

                goto L_Process;
            }
        }

        // close current node and return to parent
        if (parent->node_name) {
            if (parent->child_count > 0) {
                if (!parent->last_node_was_text) {
                    ret = outer_xml_write_padding(stack_index - 1, out_string);
                    bytes_written += ret;
                    if (out_string) out_string += ret;
                }
                STR_PUTS("</", out_string, bytes_written);
                STR_PUTS(parent->node_name, out_string, bytes_written);
                STR_PUTS(">", out_string, bytes_written);
            } else {
                bytes_written--;
                if (out_string) out_string--;
                STR_PUTS(" />", out_string, bytes_written);
            }
        }

        stack_index--;
    }

    free_chk(stack);
    return bytes_written;
}


static size_t concat_text_nodes(const BNO_Node* root, char* out_string) {
    size_t bytes_written = 0;

    size_t stack_length = 16;
    ssize_t stack_index = 0;
    BNOStack2* stack = malloc_for_array(BNOStack2, stack_length);

    // add root to stack
    stack[stack_index] = (BNOStack2){
        .child_index = 0,
        .child_count = (int32_t)xmlparser_get_children_count(root),
        .node = root
    };
    stack_index++;

    XmlNode node_parent;
    BNOStack2* parent;

L_Process:
    while (stack_index > 0) {
        parent = stack + (stack_index - 1);
        node_parent = parent->node;

        for (; parent->child_index < parent->child_count; parent->child_index++) {
            XmlNode node = xmlparser_get_children_at(node_parent, parent->child_index);

            if (xmlparser_is_node_text(node)) {
                const char* str = xmlparser_get_text(node);
                STR_PUTS(str, out_string, bytes_written);
            } else {
                ssize_t new_stack_index = stack_index + 1;
                if (new_stack_index >= stack_length) {
                    // this never should happen (XML is too big)
                    stack_length += 16;
                    stack = realloc_for_array(stack, stack_length, BNOStack2);
                }

                // obligatory
                parent->child_index++;

                // change parent and write current node childrens
                stack[stack_index] = (BNOStack2){
                    .child_index = 0,
                    .child_count = xmlparser_get_children_count(node),
                    .node = node
                };

                stack_index = new_stack_index;
                parent = stack + stack_index;

                goto L_Process;
            }
        }

        // return to parent
        stack_index--;
    }

    free_chk(stack);
    return bytes_written;
}


static BNO_Attribute* seek_to_attribute(const BNO_Node* bno_node, int32_t index) {
    if (!bno_node) return NULL;
    if (bno_node->type != BNO_Type_Object) return 0;
    if (index < 0 || index >= bno_node->attributes_count) return NULL;

    const uint8_t* ptr = bno_node->data + bno_node->name_length;

    for (; index > 0; index--) {
        BNO_Attribute* attr = (BNO_Attribute*)ptr;
        ptr += (int32_t)sizeof(BNO_Attribute) + attr->name_length + attr->value_length;
    }
    return (BNO_Attribute*)ptr;
}

static BNO_Attribute* seek_to_attribute_by_name(const BNO_Node* bno_node, const char* attribute_name) {
    if (!bno_node) return NULL;
    if (bno_node->attributes_count < 1) return NULL;
    if (!attribute_name || attribute_name[0] == '\0') return NULL;

    const uint8_t* ptr = bno_node->data + bno_node->name_length;
    size_t name_length = strlen(attribute_name) + 1;

    assert(name_length <= UINT8_MAX);

    for (uint8_t i = 0; i < bno_node->attributes_count; i++) {
        BNO_Attribute* attr = (BNO_Attribute*)ptr;

        if (name_length == attr->name_length && memcmp(attr->data, attribute_name, name_length) == 0) {
            return attr;
        }

        ptr += (int32_t)sizeof(BNO_Attribute) + attr->name_length + attr->value_length;
    }

    return NULL;
}


static bool integrity_check(const BNO_Node* root, size_t length) {
    size_t stack_length = 16;
    ssize_t stack_index = 0;
    BNOStack3* stack = malloc_for_array(BNOStack3, stack_length);

    length -= sizeof(BNO_Header);

    // add root to stack
    stack[stack_index] = (BNOStack3){
        .accumulated_size = 0,
        .child_index = 0,
        .child_count = (int32_t)xmlparser_get_children_count(root),
        .node = root
    };
    stack_index++;

    XmlNode node_parent;
    BNOStack3* parent;
    const BNO_Node* asserted_offset = NULL;

L_Process:
    while (stack_index > 0) {
        parent = stack + (stack_index - 1);
        node_parent = parent->node;

        for (; parent->child_index < parent->child_count; parent->child_index++) {
            XmlNode node = xmlparser_get_children_at(node_parent, parent->child_index);
            asserted_offset = node;

            // avoid out-of-bounds reading
            MEM_CHECK(node, root, length, L_failed);

            if (node->type == BNO_Type_String) {
                if (node->attributes_count > 0 || node->attributes_length > 0 || node->name_length > 0) {
                    // such fields must be zero
                    goto L_failed;
                }
                if (node->value_length > BNO_MAX_NODE_SIZE) {
                    // >2GiB
                    goto L_failed;
                }
                MEM_CHECK(node + node->value_length, root, length, L_failed);
                if (STR_CHECK_END(node->data, node->value_length)) {
                    // missing null terminator in string value
                    goto L_failed;
                }

                parent->accumulated_size += sizeof(BNO_Node) + node->value_length;
                continue;
            } else if (node->type == BNO_Type_EOF) {
                break;
            } else if (node->type != BNO_Type_Object) {
                // unknown node type
                goto L_failed;
            }

            size_t node_length = node->name_length + node->attributes_length + node->value_length;
            MEM_CHECK(node + node_length, root, length, L_failed);

            // check valid node fields
            if (node->attributes_count > BNO_MAX_ATTRIBUTES) {
                // overflows "attributes_length" field
                goto L_failed;
            }
            if (node->name_length < 2) {
                // missing or empty node name
                goto L_failed;
            }
            if (STR_CHECK_END(node->data, node->name_length)) {
                // missing null terminator in node name
                goto L_failed;
            }

            // check attributes length
            uint64_t attributes_length = 0;
            const uint8_t* attr_ptr = node->data + node->name_length;
            for (uint8_t i = 0; i < node->attributes_count; i++) {
                const BNO_Attribute* attr = (const BNO_Attribute*)attr_ptr;
                attr_ptr += sizeof(BNO_Attribute);

                if (STR_CHECK_END(attr_ptr, attr->name_length)) {
                    // missing null terminator in attribute name
                    goto L_failed;
                }
                attr_ptr += attr->name_length;
                if (STR_CHECK_END(attr_ptr, attr->value_length)) {
                    // missing null terminator in attribute value
                    goto L_failed;
                }
                attr_ptr += attr->value_length;

                attributes_length += sizeof(BNO_Attribute) + attr->name_length + attr->value_length;
            }

            if (attributes_length != node->attributes_length) {
                // invalid "attributes_length" field
                goto L_failed;
            }

            // accumulate node header size
            parent->accumulated_size += sizeof(BNO_Node) + node_length;

            ssize_t new_stack_index = stack_index + 1;
            if (new_stack_index >= stack_length) {
                // this never should happen (XML is too big)
                stack_length += 16;
                stack = realloc_for_array(stack, stack_length, BNOStack3);
            }

            // obligatory
            parent->child_index++;

            // change parent and write current node childrens
            stack[stack_index] = (BNOStack3){
                .accumulated_size = 0,
                .child_index = 0,
                .child_count = xmlparser_get_children_count(node),
                .node = node
            };

            stack_index = new_stack_index;
            goto L_Process;
        }

        // return to parent
        stack_index--;

        // check accumulated node size
        if (parent->accumulated_size != parent->node->value_length) {
            // invalid "value_length" field
            goto L_failed;
        }
    }

    free_chk(stack);
    return true;

L_failed:
    logger_error(
        "integrity_check() malformed bno file at offset %u.",
        sizeof(BNO_Header) + ((size_t)asserted_offset - (size_t)root)
    );

    free_chk(stack);
    return false;
}


XmlParser xmlparser_init(const char* src) {
    ArrayBuffer arraybuffer = fs_readarraybuffer(src);
    if (!arraybuffer) return NULL;

    if (arraybuffer->length < 1) {
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

    const BNO_Header* root = (const BNO_Header*)arraybuffer->data;

    if (root->signature != BNO_SIGNATURE) {
        logger_error("xmlparser_init() failed, expected BNO file.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }
    if (root->content != BNO_CONTENT_XML) {
        logger_error("xmlparser_init() failed, the bno contents is not XML.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

    // do integrity check, to avoid out-of-bounds access
    if (!integrity_check((const BNO_Node*)root->data, arraybuffer->length)) {
        logger_error("xmlparser_init() failed, malformed XML contents.");
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

#ifdef DEBUG
    const uint8_t* ptr = (const uint8_t*)root - sizeof(struct ArrayBuffer_s);
    assert((void*)ptr == (void*)arraybuffer);
#endif

    return (XmlParser)root;
}

void xmlparser_destroy(XmlParser* xmlparser_ptr) {
    if (!xmlparser_ptr || !*xmlparser_ptr) return;

    XmlParser xmlparser = *xmlparser_ptr;

    const uint8_t* ptr = (const uint8_t*)xmlparser;
    ArrayBuffer arraybuffer = (ArrayBuffer)(ptr - sizeof(struct ArrayBuffer_s));

    arraybuffer_destroy(&arraybuffer);
    *xmlparser_ptr = NULL;
}


XmlNode xmlparser_get_root(XmlParser xmlparser) {
    XmlNode root = (XmlNode)xmlparser->data;
    if (root->type == BNO_Type_EOF) {
        return NULL;
    }

    return root;
}


uint8_t xmlparser_get_attribute_count(XmlNode node) {
    return node->attributes_count;
}

const char* xmlparser_get_attribute_name(XmlNode node, int32_t index) {
    BNO_Attribute* attr = seek_to_attribute(node, index);
    if (!attr) return NULL;

    return (const char*)attr->data;
}

const char* xmlparser_get_attribute_value(XmlNode node, int32_t index) {
    BNO_Attribute* attr = seek_to_attribute(node, index);
    if (!attr) return NULL;

    // seek to value
    return (const char*)(attr->data + attr->name_length);
}

const char* xmlparser_get_attribute_value2(XmlNode node, const char* attribute_name) {
    BNO_Attribute* attr = seek_to_attribute_by_name(node, attribute_name);
    if (!attr) return NULL;

    // seek to value
    return (const char*)(attr->data + attr->name_length);
}

XmlAttribute xmlparser_get_attribute(XmlNode node, int32_t index) {
    BNO_Attribute* attr = seek_to_attribute(node, index);

    XmlAttribute ret = (XmlAttribute){
        .name = NULL,
        .value = NULL
    };

    if (attr) {
        ret.name = (const char*)attr->data;
        ret.value = (const char*)(attr->data + attr->name_length);
    }

    return ret;
}

bool xmlparser_has_attribute(XmlNode node, const char* attribute_name) {
    BNO_Attribute* attr = seek_to_attribute_by_name(node, attribute_name);

    return attr != NULL;
}


const char* xmlparser_get_tag_name(XmlNode node) {
    if (!node || node->type != BNO_Type_Object) return NULL;

    return (const char*)node->data;
}

XmlNode xmlparser_get_first_children(XmlNode node, const char* tag_name) {
    if (!node || node->value_length < 1) return NULL;
    if (node->type != BNO_Type_Object) return 0;

    const uint8_t* ptr = node->data + node->name_length + node->attributes_length;
    const uint8_t* ptr_end = ptr + node->value_length;
    size_t tag_name_length = strlen(tag_name) + 1;

    assert(tag_name_length < UINT8_MAX);

    while (ptr < ptr_end) {
        BNO_Node* child = (BNO_Node*)ptr;

        if (child->type == BNO_Type_Object) {
            if (tag_name_length == child->name_length && memcmp(child->data, tag_name, tag_name_length) == 0) {
                return child;
            }
        }

        ptr += sizeof(BNO_Node) + child->name_length + child->attributes_length + child->value_length;
    }

    return NULL;
}

int32_t xmlparser_get_children_count(XmlNode node) {
    if (!node || node->value_length < 1) return 0;
    if (node->type != BNO_Type_Object) return 0;

    const uint8_t* ptr = node->data + node->name_length + node->attributes_length;
    const uint8_t* ptr_end = ptr + node->value_length;
    int32_t count = 0;

    while (ptr < ptr_end) {
        BNO_Node* child = (BNO_Node*)ptr;
        ptr += sizeof(BNO_Node) + child->name_length + child->attributes_length + child->value_length;
        count++;
    }

    return count;
}

XmlNode xmlparser_get_children_at(XmlNode node, int32_t index) {
    if (!node || node->value_length < 1) return NULL;
    if (node->type != BNO_Type_Object) return 0;

    const uint8_t* ptr = node->data + node->name_length + node->attributes_length;
    const uint8_t* ptr_end = ptr + node->value_length;
    int32_t i = 0;

    while (ptr < ptr_end) {
        BNO_Node* child = (BNO_Node*)ptr;
        if (i == index) {
            return child;
        }

        ptr += sizeof(BNO_Node) + child->name_length + child->attributes_length + child->value_length;
        i++;
    }

    return NULL;
}


XmlChildrenIterator xmlparser_iterator_get_children(XmlNode node) {
    XmlChildrenIterator iterator = {.ptr = NULL};
    iterator.len = xmlparser_get_children_count(node);
    iterator.__run = iterator.len > 0;

    if (iterator.len > 0) {
        iterator.ptr = node->data + node->name_length + node->attributes_length;
    }

    return iterator;
}

XmlChildrenIterator xmlparser_iterator_get_children2(XmlNode node, const char* tag_name) {
    XmlChildrenIterator iterator = xmlparser_iterator_get_children(node);
    iterator.tag_name = tag_name;
    return iterator;
}

XmlRecursiveIterator xmlparser_iterator_get_recursive(XmlNode node, const char* tag_name) {
    assert(tag_name);

    XmlRecursiveIterator iterator = {.tag_name = tag_name, .stack_used = 1};
    iterator.stack[0].len = xmlparser_get_children_count(node);
    iterator.__run = iterator.stack[0].len > 0;

    if (iterator.stack[0].len > 0) {
        iterator.stack[0].ptr = node->data + node->name_length + node->attributes_length;
    }

    return iterator;
}

bool xmlparser_iterate_children(XmlChildrenIterator* iterator, XmlNode* node) {
L_start_iterate:
    if (iterator->len < 1) {
        *node = NULL;
        return false;
    }

    BNO_Node* child = (BNO_Node*)iterator->ptr;
    uint32_t child_size = sizeof(BNO_Node) + child->name_length + child->attributes_length + child->value_length;

    iterator->ptr += child_size;
    iterator->len--;

    if (iterator->tag_name) {
        // filter by name
        const char* child_tag_name = (const char*)child->data;
        if (child->type != BNO_Type_Object || !string_equals(iterator->tag_name, child_tag_name)) {
            goto L_start_iterate;
        }
    }

    *node = child;
    return true;
}

bool xmlparser_iterate_recursive_children(XmlRecursiveIterator* iterator, XmlNode* node) {
    uint8_t index = iterator->stack_used - 1;

L_start_iterate:
    if (iterator->stack[index].len < 1) {
        if (index < 1) {
            *node = NULL;
            return false;
        } else {
            // continue on previous silbing node
            iterator->stack_used--;
            index--;
            goto L_start_iterate;
        }
    }

    BNO_Node* child = (BNO_Node*)iterator->stack[index].ptr;
    uint32_t child_size = sizeof(BNO_Node) + child->name_length + child->attributes_length + child->value_length;

    iterator->stack[index].ptr += child_size;
    iterator->stack[index].len--;

    // ignore text nodes
    if (child->type != BNO_Type_Object) {
        goto L_start_iterate;
    }

    int32_t sub_length = xmlparser_get_children_count(child);
    if (sub_length > 0) {
        // push child on the stack
        iterator->stack_used++;
        assert(iterator->stack_used <= XMLPARSER_ITERATOR_MAX_RECURSIVE_DEEP);

        // make it current
        index++;
        iterator->stack[index].len = sub_length;
        iterator->stack[index].ptr = (const uint8_t*)child + child->name_length + child->attributes_length;
    }

    // filter by name
    const char* child_tag_name = (const char*)child->data;
    if (!string_equals(iterator->tag_name, child_tag_name)) {
        goto L_start_iterate;
    }

    *node = child;
    return true;
}


XmlAttributesIterator xmlparser_iterator_get_attributes(XmlNode node) {
    XmlAttributesIterator iterator = {.ptr = NULL};
    iterator.len = xmlparser_get_attribute_count(node);
    iterator.__run = iterator.len > 0;

    if (iterator.len > 0) {
        iterator.ptr = node->data + node->name_length;
    }

    return iterator;
}

bool xmlparser_iterate_attributes(XmlAttributesIterator* iterator, XmlAttribute* attribute) {
    if (iterator->len < 1) {
        *attribute = (XmlAttribute){.name = NULL, .value = NULL};
        return false;
    }

    BNO_Attribute* attr = (BNO_Attribute*)iterator->ptr;
    int32_t child_size = (int32_t)sizeof(BNO_Attribute) + attr->name_length + attr->value_length;

    iterator->ptr += child_size;
    iterator->len--;

    *attribute = (XmlAttribute){
        .name = (const char*)attr->data,
        .value = (const char*)(attr->data + attr->name_length)
    };
    return true;
}


const char* xmlparser_get_text(XmlNode text_node) {
    if (!text_node) return NULL;
    if (text_node->type == BNO_Type_String) {
        return (const char*)text_node->data;
    }

    return NULL;
}

bool xmlparser_is_node_text(XmlNode node) {
    if (!node) return NULL;

    return node->type == BNO_Type_String;
}


char* xmlparser_get_outerXml(XmlNode node) {
    // determine the string size
    size_t length = outer_xml_stringify(node, NULL);

    char* str = malloc_chk(length + 1);
    assert(str);

    outer_xml_stringify(node, str);
    str[length] = '\0';

    return str;
}

char* xmlparser_get_textContext(XmlNode node) {
    /*if (!node) return NULL;
    if (node->type == BNO_Type_String) {
        return strdup((const char*)node->data);
    } else if (node->type != BNO_Type_Object) {
        return NULL;
    }

    size_t string_length = 0;
    XmlChildrenIterator iterator = xmlparser_iterator_get_children(node);
    XmlNode child;

    while (xmlparser_iterate_children(&iterator, &child)) {
        if (child->type == BNO_Type_String && child->value_length > 1) {
            string_length += (size_t)child->value_length - 1;
        }
    }

    char* str = malloc_chk(string_length + 1);
    assert(str);
    char* str_ptr = str;

    iterator = xmlparser_iterator_get_children(node);
    while (xmlparser_iterate_children(&iterator, &child)) {
        if (child->type == BNO_Type_String && child->value_length > 1) {
            int32_t value_length = child->value_length - 1;
            memcpy(str_ptr, child->data, (size_t)value_length);
            str_ptr += value_length;
        }
    }

    str[string_length] = '\0';
    return str;
    */

    // determine the string size
    size_t length = concat_text_nodes(node, NULL);

    char* str = malloc_chk(length + 1);
    assert(str);

    concat_text_nodes(node, str);
    str[length] = '\0';

    return str;
}
