#ifndef _filehandle_h
#define _filehandle_h

#include <stdint.h>
#include <stdbool.h>

typedef int32_t(*FileHandle_Read)(void* handle, void* buffer, int32_t buffer_size);
typedef int64_t(*FileHandle_Seek)(void* handle, int64_t offset, int32_t whence);
typedef int64_t(*FileHandle_Tell)(void* handle);
typedef int64_t(*FileHandle_Length)(void* handle);

typedef struct {
    void* handle;
    bool is_file;
    FileHandle_Read read;
    FileHandle_Seek seek;
    FileHandle_Tell tell;
    FileHandle_Length length;
} FileHandle_t;

static inline int32_t filehandle_read(FileHandle_t* f, void* buffer, int32_t buffer_size) {
    return f->read(f->handle, buffer, buffer_size);
}

static inline int64_t filehandle_seek(FileHandle_t* f, int64_t offset, int32_t whence) {
    return f->seek(f->handle, offset, whence);
}

static inline int64_t filehandle_tell(FileHandle_t* f) {
    return f->tell(f->handle);
}

static inline int64_t filehandle_length(FileHandle_t* f) {
    return f->length(f->handle);
}


FileHandle_t* filehandle_init(const char* fullpath);
FileHandle_t* filehandle_init2(const uint8_t* buffer, int32_t size);
FileHandle_t* filehandle_init3(const char* fullpath);
void filehandle_destroy(FileHandle_t* filehandle);

#endif
