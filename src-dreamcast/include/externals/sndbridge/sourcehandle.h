#ifndef __sourcehandle_h
#define __sourcehandle_h

#include <stdbool.h>
#include <stdint.h>

#include "arraybuffer.h"


typedef struct SourceHandle_s SourceHandle;

struct SourceHandle_s {
    int32_t (*read)(SourceHandle* handle, void* buffer, int32_t buffer_size);
    int32_t (*seek)(SourceHandle* handle, int64_t offset, int whence);
    int64_t (*tell)(SourceHandle* handle);
    int64_t (*length)(SourceHandle* handle);
    void (*destroy)(SourceHandle* handle);
    void (*suspend)(SourceHandle* handle);
};

SourceHandle* filehandle_init1(const char* fullpath, bool try_load_in_ram);
SourceHandle* filehandle_init2(void* buffer, int32_t size);
SourceHandle* filehandle_init3(void* buffer, int32_t buffer_length, int32_t offset, int32_t size);
SourceHandle* filehandle_init4(ArrayBuffer arraybuffer, bool allow_dispose);

#ifdef _arch_dreamcast
SourceHandle* filehandle_init5(const char* native_path);
#else
#define filehandle_init5(native_path) filehandle_init1(native_path, true)
#endif

#endif
