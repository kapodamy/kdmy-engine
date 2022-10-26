#ifndef _filehandle_h
#define _filehandle_h

#include <stdint.h>

typedef int32_t(*filehandle_read)(void* stream, void* buffer, int32_t buffer_size);
typedef int64_t(*filehandle_seek)(void* stream, int64_t offset, int32_t whence);
typedef int64_t(*filehandle_tell)(void* stream);

typedef struct {
    void* handle;
    filehandle_read read;
    filehandle_seek seek;
    filehandle_tell tell;
} FileHandle_t;

static inline int32_t file_read(FileHandle_t* f, void* buffer, int32_t buffer_size) {
    return f->read(f->handle, buffer, buffer_size);
}

static inline int64_t file_seek(FileHandle_t* f, int64_t offset, int32_t whence) {
    return f->seek(f->handle, offset, whence);
}

static inline int64_t file_tell(FileHandle_t* f) {
    return f->tell(f->handle);
}


FileHandle_t* filehandle_init(const char* fullpath);
void filehandle_destroy(FileHandle_t* filehandle);

#endif
