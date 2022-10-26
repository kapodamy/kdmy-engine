#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "filehandle.h"

// 128MiB
#define MAX_SIZE_IN_MEMORY (128*1024*1024)

typedef struct {
    long size;
    long offset;
    uint8_t* buffer;
} Data;


static int32_t read_file(void* stream, void* buffer, int32_t buffer_size) {
    return fread(buffer, 1, buffer_size, (FILE*)stream);
}
static int64_t seek_file(void* stream, int64_t offset, int32_t whence) {
    return fseek((FILE*)stream, offset, whence);
}
static int64_t tell_file(void* stream) {
    return ftell((FILE*)stream);
}

static int32_t read_data(void* stream, void* buffer, int32_t buffer_size) {
    Data* data = (Data*)stream;

    long end = data->offset + buffer_size;
    if (end == data->size) return 0;
    if (end > data->size) buffer_size = data->size - data->offset;

    if (buffer_size < 1) return 0;

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
    return 0;
}
static int64_t tell_data(void* stream) {
    Data* data = (Data*)stream;
    return data->offset;
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
        filehandle->read = read_file;
        filehandle->seek = seek_file;
        filehandle->tell = tell_file;
        return filehandle;
    }

    Data* data = calloc(1, sizeof(Data));
    assert(data);

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
    filehandle->read = read_data;
    filehandle->seek = seek_data;
    filehandle->tell = tell_data;
    return filehandle;
}

extern void filehandle_destroy(FileHandle_t* filehandle) {
    if (!filehandle) return;

    filehandle->handle = NULL;
    filehandle->read = NULL;
    filehandle->seek = NULL;
    filehandle->tell = NULL;
    free(filehandle);
}
