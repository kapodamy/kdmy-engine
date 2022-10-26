#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <wchar.h>
#include "./../fontatlas.h"

static inline void* read(const char* path, int32_t* size) {
    FILE* fp;
    long lSize;
    char* buffer;

    fp = fopen(path, "rb");
    if (!fp) perror(path), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    *size = lSize;
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    /* do your work here, buffer is a string contains the whole text */

    fclose(fp);
    return buffer;
}
/*
static uint8_t const u8_length[] = {
    // 0 1 2 3 4 5 6 7 8 9 A B C D E F
       1,1,1,1,1,1,1,1,0,0,0,0,2,2,3,4
};

#define u8length(s) u8_length[(((uint8_t *)(s))[0] & 0xFF) >> 4];

int u8strlen(const char* s) {
    int len = 0;
    while (*s) {
        if ((*s & 0xC0) != 0x80) len++;
        s++;
    }
    return len;
}

static int u8chrisvalid(int32_t c) {
    if (c <= 0x7F) return 1;                    // [1]

    if (0xC280 <= c && c <= 0xDFBF)             // [2]
        return ((c & 0xE0C0) == 0xC080);

    if (0xEDA080 <= c && c <= 0xEDBFBF)         // [3]
        return 0; // Reject UTF-16 surrogates

    if (0xE0A080 <= c && c <= 0xEFBFBF)         // [4]
        return ((c & 0xF0C0C0) == 0xE08080);

    if (0xF0908080 <= c && c <= 0xF48FBFBF)     // [5]
        return ((c & 0xF8C0C0C0) == 0xF0808080);

    return 0;
}

static int u8next(char* txt, int32_t* ch) {
    int len;
    int32_t encoding = 0;

    len = u8length(txt);

    for (int i = 0; i < len && txt[i] != '\0'; i++) {
        encoding = (encoding << 8) | txt[i];
    }

    errno = 0;
    if (len == 0 || !u8chrisvalid(encoding)) {
        encoding = txt[0];
        len = 1;
        errno = EINVAL;
    }

    if (ch) *ch = encoding;

    return encoding ? len : 0;
}

static uint32_t u8decode(uint32_t c) {
    uint32_t mask;

    if (c > 0x7F) {
        mask = (c <= 0x00EFBFBF) ? 0x000F0000 : 0x003F0000;
        c = ((c & 0x07000000) >> 6) |
            ((c & mask) >> 4) |
            ((c & 0x00003F00) >> 2) |
            (c & 0x0000003F);
    }

    return c;
}

*/

int main() {
    printf("FontAtlas: FreeType v%s\n",fontatlas_get_version());

    int32_t buffer_size;
    uint8_t* buffer = read("./vcr.ttf", &buffer_size);

    fontatlas_enable_sdf(true);
    FontAtlas fontatlas = fontatlas_init(buffer, buffer_size);

    uint32_t size_common = wcslen(FONTATLAS_BASE_LIST_COMMON);
    uint32_t size_extended = wcslen(FONTATLAS_BASE_LIST_EXTENDED);
    uint32_t size_total = size_extended + size_common;
    uint32_t* list_complete = malloc((size_total + 1) * sizeof(uint32_t));
    uint32_t j = 0;

    for (size_t i = 0; i < size_common; i++)  list_complete[j++] = FONTATLAS_BASE_LIST_COMMON[i];
    for (size_t i = 0; i < size_extended; i++) list_complete[j++] = FONTATLAS_BASE_LIST_EXTENDED[i];
    list_complete[size_total] = 0;

    //uint32_t test[] = {L'↔', 0};
    FontCharMap* charmap = fontatlas_atlas_build(fontatlas, 128, 16, list_complete);

    printf("\ncreating bitmap.data width=%hu height=%hu\n", charmap->texture_width, charmap->texture_height);

#ifdef FONT_4BPP
    // 4bit coversion
    bool high_or_low = true;
    size_t j = 0;
    for (size_t i = 0; i < fontatlas->texture_byte_size; i++) {
        uint8_t pixel_4bit = fontatlas->texture[i] / 0xF0;

        if (high_or_low)
            fontatlas->texture[j] = pixel_4bit << 4;
        else
            fontatlas->texture[j++] |= pixel_4bit & 0x0F;

        high_or_low = !high_or_low;
    }
    fontatlas->texture = realloc(fontatlas->texture, fontatlas->texture_byte_size /= 2);
#endif

    FILE* image = fopen("bitmap.data", "wb");
    fwrite(charmap->texture, charmap->texture_byte_size, 1, image);
    fflush(image);
    fclose(image);

    fontatlas_atlas_destroy(&charmap);
    fontatlas_destroy(&fontatlas);

    return 0;
}

