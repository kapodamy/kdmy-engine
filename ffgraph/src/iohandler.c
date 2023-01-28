#include "ffgraph.h"

#define BUFFER_SIZE (256 * 1024)                                // 256KiB
#define BUFFER_SIZE_BIG_FILES (1024 * 1024 * 4)                 // 4MiB
#define FILEHANDLE_MAX_BUFFERED_FILE_LENGTH (128 * 1024 * 1024) // 128MiB


static int iohandler_read(void* opaque, uint8_t* buf, int buf_size) {
    FileHandle_t* filehandle = opaque;

    int readed = filehandle_read(filehandle, buf, buf_size);

    // check if the end-of-file was reached
    if (readed < 0) return AVERROR_EOF;

    return readed;
}

static int64_t iohandler_seek(void* opaque, int64_t offset, int whence) {
    FileHandle_t* filehandle = opaque;
    if (whence == AVSEEK_SIZE) return filehandle_length(filehandle);

    whence &= ~AVSEEK_FORCE;

    int64_t new_offset = filehandle_seek(filehandle, offset, whence);

    return new_offset < 0 ? AVERROR_UNKNOWN : new_offset;
}


AVIOContext* iohandler_init(const FileHandle_t* filehandle) {
    FileHandle_t* filehnd = (FileHandle_t*)filehandle;

    // just in case
    filehandle_seek(filehnd, 0, SEEK_SET);

    int buffer_size;
    if (filehandle_length(filehnd) > FILEHANDLE_MAX_BUFFERED_FILE_LENGTH)
        buffer_size = BUFFER_SIZE_BIG_FILES;
    else
        buffer_size = BUFFER_SIZE;

    uint8_t* buffer = av_malloc(buffer_size);
    if (!buffer) {
        printf("iohandler_init() Can not allocate the buffer.\n");
        return NULL;
    }

    AVIOContext* avio_ctx = avio_alloc_context(
        buffer, buffer_size, 0, filehnd, iohandler_read, NULL, iohandler_seek
    );
    if (!avio_ctx) {
        printf("iohandler_init() call to avio_alloc_context() failed.\n");
        return NULL;
    }

    return avio_ctx;
}

void iohandler_destroy(AVIOContext** iohandle) {
    AVIOContext* avio_ctx = *iohandle;

    if (avio_ctx->buffer) av_free(avio_ctx->buffer);
    avio_context_free(iohandle);
}
