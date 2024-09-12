#ifndef _arraybuffer_h
#define _arraybuffer_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ArrayBuffer_s {
    size_t length;
    int32_t references;
    const uint8_t data[];
}* ArrayBuffer;


ArrayBuffer arraybuffer_init(size_t data_size);
ArrayBuffer arraybuffer_init2(const char* native_file_path, size_t max_file_size);
void arraybuffer_destroy(ArrayBuffer* arraybuffer);
void* arraybuffer_destroy2(ArrayBuffer* arraybuffer, bool add_null_terminator);
void arraybuffer_force_destroy(ArrayBuffer* arraybuffer);
ArrayBuffer arraybuffer_share_reference(ArrayBuffer arraybuffer);

#endif
