#ifdef DEBUG
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "oggdecoder.h"

static FileHandle_t filehandle;

static int32_t read(void* stream, void* buffer, int32_t buffer_size) {
    return fread(buffer, 1, buffer_size, (FILE*)stream);
}
static int64_t seek(void* stream, int64_t offset, int32_t whence) {
    return fseek((FILE*)stream, offset, whence);
}
static int64_t tell(void* stream) {
    return ftell((FILE*)stream);
}

int main2() {
    FILE* file = fopen("./opus.ogg", "rb");
    assert(file);
    filehandle.handle = file;
    filehandle.read = read;
    filehandle.seek = seek;
    filehandle.tell = tell;
    OggDecoder* oggdecoder = oggdecoder_init(&filehandle);
    assert(oggdecoder);

    const int size = 64 * 1024;
    unsigned char buffer[size];

    FILE* out = fopen("./audio.pcm", "wb+");

    while (1) {
        int32_t readed = oggdecoder_read(oggdecoder, buffer, size);
        if (readed < 1) break;
        fwrite(buffer, 1, readed, out);
    }

    fflush(out);
    fclose(out);
    oggdecoder_destroy(oggdecoder);
    fclose(file);

    return 0;
}
#endif