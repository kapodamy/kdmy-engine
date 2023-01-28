#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "filehandle.h"

// 128MiB
#define MAX_SIZE_IN_MEMORY (128*1024*1024)

typedef struct {
    int64_t size;
    int64_t offset;
    uint8_t* buffer;
    bool internal_allocated;
} Data;


static int32_t read_file(void* stream, void* buffer, int32_t buffer_size) {
    FILE* file = (FILE*)stream;

    if (feof(file))
        return -1;
    else
        return fread(buffer, 1, buffer_size, file);
}
static int64_t seek_file(void* stream, int64_t offset, int32_t whence) {
    int ret = fseek((FILE*)stream, offset, whence);
    
    return ret == 0 ? ftell((FILE*)stream) : -1;
}
static int64_t tell_file(void* stream) {
    return ftell((FILE*)stream);
}
static int64_t length_file(void* stream) {
    FILE* fp = (FILE*)stream;
    int64_t offset = ftell(fp);

    fseek(fp, 0L, SEEK_END);
    int64_t sz = ftell(fp);

    fseek(fp, offset, SEEK_SET);

    return sz;
}

static int32_t read_data(void* stream, void* buffer, int32_t buffer_size) {
    Data* data = (Data*)stream;

    if (data->offset >= data->size) return -1;

    long end = data->offset + buffer_size;
    if (end > data->size) buffer_size = data->size - data->offset;

    memcpy(buffer, data->buffer + data->offset, buffer_size);
    data->offset += buffer_size;

    return buffer_size;
}
static int64_t seek_data(void* stream, int64_t offset, int32_t whence) {
    long newoffset;
    Data* data = (Data*)stream;

    switch (whence)
    {
    case SEEK_SET:
        newoffset = offset;
        break;
    case SEEK_CUR:
        newoffset = data->offset + offset;
        break;
    case SEEK_END:
        newoffset = data->size - offset;
        break;
    default:
        return -1;
    }

    if (newoffset < 0 || newoffset > data->size) return -2;
    data->offset = newoffset;
    return newoffset;
}
static int64_t tell_data(void* stream) {
    Data* data = (Data*)stream;
    return data->offset;
}
static int64_t length_data(void* stream) {
    Data* data = (Data*)stream;
    return data->size;
}


extern FileHandle_t* filehandle_init(const char* fullpath) {
    FILE* file = fopen(fullpath, "rb");
    if (!file) return NULL;

    if (fseek(file, 0, SEEK_END)) {
        fclose(file);
        return NULL;
    }
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length > MAX_SIZE_IN_MEMORY) {
        FileHandle_t* filehandle = malloc(sizeof(FileHandle_t));
        assert(filehandle);
        filehandle->handle = file;
        filehandle->is_file = true;
        filehandle->read = read_file;
        filehandle->seek = seek_file;
        filehandle->tell = tell_file;
        filehandle->length = length_file;
        return filehandle;
    }

    Data* data = calloc(1, sizeof(Data));
    assert(data);

    data->internal_allocated = true;
    data->size = length;
    data->buffer = malloc(length);
    if (!data->buffer) {
        fclose(file);
        free(data);
        return NULL;
    }

    long readed = (long)fread(data->buffer, 1, length, file);
    if (readed < length) {
        data = realloc(data->buffer, readed);
        assert(data->buffer);
    }

    fclose(file);

    FileHandle_t* filehandle = malloc(sizeof(FileHandle_t));
    assert(filehandle);
    filehandle->handle = data;
    filehandle->is_file = false;
    filehandle->read = read_data;
    filehandle->seek = seek_data;
    filehandle->tell = tell_data;
    filehandle->length = length_data;
    return filehandle;
}

extern FileHandle_t* filehandle_init2(const uint8_t* buffer, int32_t size) {  
    if (!buffer) return NULL;

    Data* data = calloc(1, sizeof(Data));
    assert(data);
    data->size = size;
    data->buffer = (uint8_t*)buffer;
    data->internal_allocated = false;

    FileHandle_t* filehandle = malloc(sizeof(FileHandle_t));
    assert(filehandle);
    filehandle->handle = data;
    filehandle->is_file = false;
    filehandle->read = read_data;
    filehandle->seek = seek_data;
    filehandle->tell = tell_data;
    filehandle->length = length_data;
    return filehandle;
}

extern FileHandle_t* filehandle_init3(const char* fullpath) {
    FILE* file = fopen(fullpath, "rb");
    if (!file) return NULL;

    FileHandle_t* filehandle = malloc(sizeof(FileHandle_t));
    assert(filehandle);
    filehandle->handle = file;
    filehandle->is_file = true;
    filehandle->read = read_file;
    filehandle->seek = seek_file;
    filehandle->tell = tell_file;
    filehandle->length = length_file;

    return filehandle;
}

extern void filehandle_destroy(FileHandle_t* filehandle) {
    if (!filehandle) return;

    if (filehandle->is_file) {
        fclose(filehandle->handle);
    } else {
        Data* data = filehandle->handle;
        if (data->internal_allocated) free(data->buffer);
        free(data);
    }

    filehandle->handle = NULL;
    filehandle->read = NULL;
    filehandle->seek = NULL;
    filehandle->tell = NULL;
    free(filehandle);
}
