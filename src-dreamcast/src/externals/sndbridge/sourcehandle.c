#include "externals/sndbridge/sourcehandle.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"


#ifdef _arch_dreamcast

#ifndef MALLOC_CHK_BOUNDS_CHECK
#include <arch/arch.h>
#endif

#define MAX_SIZE_IN_MEMORY (1 * 1024 * 1024)     // 1MiB
#define MAX_SIZE_IN_MEMORY_DBL (3 * 1024 * 1024) // 3MiB

#else

#define MAX_SIZE_IN_MEMORY (128 * 1024 * 1024) // 128MiB
#define DBL_CHOOSE(MACRO) (MACRO)

#endif


static volatile SourceHandle* current_filehandle_loaded_in_ram = NULL;


typedef struct {
    SourceHandle handle;
    char* native_path;
    long restore_offset;
    FILE* file;
    int64_t file_length;
} FileHandle;
static inline void FileHandle_Resume(FileHandle* filehandle) {
    if (!filehandle->file) {
        filehandle->file = fopen(filehandle->native_path, "rb");
        if (filehandle->file) {
            fseek(filehandle->file, filehandle->restore_offset, SEEK_SET);
        }
    }
}
static int32_t FileHandle_Read(SourceHandle* handle, void* buffer, int32_t buffer_size) {
    FileHandle* filehandle = (FileHandle*)handle;
    if (buffer_size < 0) return -1;

    FileHandle_Resume(filehandle);

    size_t ret = fread(buffer, 1, (size_t)buffer_size, filehandle->file);
    if (!ret && ferror(filehandle->file)) return -1;

    return (int32_t)ret;
}
static int32_t FileHandle_Seek(SourceHandle* handle, int64_t offset, int whence) {
    FileHandle* filehandle = (FileHandle*)handle;
    FileHandle_Resume(filehandle);
    return fseek(filehandle->file, offset, whence);
}
static int64_t FileHandle_Tell(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    FileHandle_Resume(filehandle);
    return ftell(filehandle->file);
}
static int64_t FileHandle_Length(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    return filehandle->file_length;
}
static void FileHandle_Destroy(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    if (!filehandle->native_path) return;

    if (filehandle->file) fclose(filehandle->file);
    free_chk(filehandle->native_path);

    filehandle->file = NULL;
    filehandle->native_path = NULL;
    free_chk(filehandle);
}
static void FileHandle_Suspend(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;

    if (filehandle->file) {
        filehandle->restore_offset = ftell(filehandle->file);
        fclose(filehandle->file);
        filehandle->file = NULL;
    }
}

typedef struct {
    SourceHandle handle;
    int32_t size;
    int32_t offset;
    uint8_t* data;
    bool allow_dispose;
} BufferHandle;
static int32_t BufferHandle_Read(SourceHandle* handle, void* buffer, int32_t buffer_size) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    if (buffer_size < 0) return -1;
    if (bufferhandle->offset >= bufferhandle->size) return 0;

    int32_t end = bufferhandle->offset + buffer_size;
    if (end > bufferhandle->size) buffer_size = (int32_t)(bufferhandle->size - bufferhandle->offset);

    memcpy(buffer, bufferhandle->data + bufferhandle->offset, (size_t)buffer_size);
    bufferhandle->offset += buffer_size;

    return buffer_size;
}
static int32_t BufferHandle_Seek(SourceHandle* handle, int64_t offset, int whence) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    int64_t newoffset;

    switch (whence) {
        case SEEK_SET:
            newoffset = offset;
            break;
        case SEEK_CUR:
            newoffset = bufferhandle->offset + offset;
            break;
        case SEEK_END:
            newoffset = bufferhandle->size - offset;
            break;
        default:
            goto L_invalid_seek;
    }

    if (newoffset < 0 || newoffset > bufferhandle->size) {
        goto L_invalid_seek;
    }

    bufferhandle->offset = newoffset;
    return 0;

L_invalid_seek:
    errno = EINVAL;
    return 1;
}
static int64_t BufferHandle_Tell(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    return bufferhandle->offset;
}
static int64_t BufferHandle_Length(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    return bufferhandle->size;
}
static void BufferHandle_Destroy(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    if (bufferhandle->allow_dispose) free_chk(bufferhandle->data);

#ifdef _arch_dreamcast
    if (current_filehandle_loaded_in_ram == handle) {
        // allow load in ram another files
        current_filehandle_loaded_in_ram = NULL;
    }
#endif

    bufferhandle->data = NULL;
    bufferhandle->size = bufferhandle->offset = -1;
    free_chk(bufferhandle);
}
static void BufferHandle_Suspend(SourceHandle* handle) {
    (void)handle;
}

typedef struct {
    BufferHandle buffer_handle;
    ArrayBuffer arraybuffer;
} ArrayBufferHandle;
static void ArrayBufferHandle_Destroy(SourceHandle* handle) {
    ArrayBufferHandle* arraybufferhandle = (ArrayBufferHandle*)handle;
    if (arraybufferhandle->buffer_handle.allow_dispose) arraybuffer_destroy(&arraybufferhandle->arraybuffer);

    arraybufferhandle->buffer_handle.data = NULL;
    arraybufferhandle->buffer_handle.size = arraybufferhandle->buffer_handle.offset = -1;
    free_chk(arraybufferhandle);
}


_Noreturn static void assert_with_msg(const char* reason_message, const char* error_message) {
    logger_error("filehandle: %s %s\n", reason_message, error_message);
    abort();
}

static int32_t file_length(FILE* file) {
    long offset = ftell(file);
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, offset, SEEK_SET);

    assert(length < INT32_MAX);

    return (int32_t)length;
}

static SourceHandle* from_buffer(void* data, int32_t size, bool allow_dispose) {
    BufferHandle* obj = malloc_chk(sizeof(BufferHandle));
    if (!obj) {
        return NULL;
    }

    obj->data = (uint8_t*)data;
    obj->size = size;
    obj->offset = 0;
    obj->allow_dispose = allow_dispose;
    obj->handle.read = BufferHandle_Read;
    obj->handle.seek = BufferHandle_Seek;
    obj->handle.tell = BufferHandle_Tell;
    obj->handle.length = BufferHandle_Length;
    obj->handle.destroy = BufferHandle_Destroy;
    obj->handle.suspend = BufferHandle_Suspend;

    return (SourceHandle*)obj;
}

static SourceHandle* from_file(FILE* file, int32_t file_length, const char* native_path) {
    FileHandle* obj = malloc_chk(sizeof(FileHandle));
    if (!obj) {
        return NULL;
    }

    obj->restore_offset = 0;
    obj->native_path = string_duplicate(native_path);
    obj->file = file;
    obj->file_length = file_length;
    obj->handle.read = FileHandle_Read;
    obj->handle.seek = FileHandle_Seek;
    obj->handle.tell = FileHandle_Tell;
    obj->handle.length = FileHandle_Length;
    obj->handle.destroy = FileHandle_Destroy;
    obj->handle.suspend = FileHandle_Suspend;

    return (SourceHandle*)obj;
}


SourceHandle* filehandle_init1(const char* native_path, bool try_load_in_ram) {
    FILE* file = fopen(native_path, "rb");

    if (!file) {
        logger_error("filehandle_init1() failed to open %s\n", native_path);
        return NULL;
    }

    int32_t length = file_length(file);
    if (!file || length < 1) {
        logger_error("filehandle_init1() failed to get the length of %s\n", native_path);
        return NULL;
    }

#ifdef _arch_dreamcast
    // only load a single file in the RAM if there at least 16MiB
    if (DBL_MEM_CHECK == 0 && current_filehandle_loaded_in_ram != NULL) {
        return from_file(file, length, native_path);
    }
#endif

    if (length > DBL_CHOOSE(MAX_SIZE_IN_MEMORY) || !try_load_in_ram) {
        return from_file(file, length, native_path);
    }

    void* buffer = malloc_chk((size_t)length);
    if (!buffer) {
        logger_warn("filehandle_init1() failed to buffer the whole contents of %s\n", native_path);
        return from_file(file, length, native_path);
    }

    size_t readed = fread(buffer, 1, (size_t)length, file);
    if (!readed) {
        fclose(file);
        return NULL;
    }
    if (readed < (size_t)length) {
        buffer = realloc_chk(buffer, readed);
    }

    fclose(file);

    SourceHandle* hnd = from_buffer(buffer, (int32_t)readed, true);

#ifdef _arch_dreamcast
    // remember this instance to avoid allocation another buffer in ram
    current_filehandle_loaded_in_ram = hnd;
#endif

#ifdef DEBUG
    logger_log("filehandle_init1() loaded in ram: %s", native_path);
#endif

    return hnd;
}

SourceHandle* filehandle_init2(void* buffer, int32_t size) {
    if (!buffer) return NULL;

    return from_buffer(buffer, size, false);
}

SourceHandle* filehandle_init3(void* buffer, int32_t buffer_length, int32_t offset, int32_t size) {
    if (!buffer) return NULL;
    if (offset >= buffer_length || offset > size || offset < 0) assert_with_msg("filehandle_init3 out-of-range", "offset");
    if (size > buffer_length || size < offset || size < 0) assert_with_msg("filehandle_init3 out-of-range", "size");

    return from_buffer(buffer, offset, size);
}

SourceHandle* filehandle_init4(ArrayBuffer arraybuffer, bool allow_dispose) {
    ArrayBufferHandle* obj = malloc_chk(sizeof(ArrayBufferHandle));
    if (!obj) {
        return NULL;
    }

    obj->buffer_handle.data = (uint8_t*)arraybuffer->data;
    obj->buffer_handle.size = (int32_t)arraybuffer->length;
    obj->buffer_handle.offset = 0;
    obj->buffer_handle.allow_dispose = allow_dispose;
    obj->buffer_handle.handle.read = BufferHandle_Read;
    obj->buffer_handle.handle.seek = BufferHandle_Seek;
    obj->buffer_handle.handle.tell = BufferHandle_Tell;
    obj->buffer_handle.handle.length = BufferHandle_Length;
    obj->buffer_handle.handle.destroy = ArrayBufferHandle_Destroy;
    obj->buffer_handle.handle.suspend = BufferHandle_Suspend;
    obj->arraybuffer = arraybuffer;

    return (SourceHandle*)obj;
}
