#ifndef _texture_h
#define _texture_h

#include <stdbool.h>
#include <stdint.h>

#include <dc/pvr.h>


#define KDT_MAX_SLICES 4

//
//  forward reference
//

typedef struct ArrayBuffer_s* ArrayBuffer;

typedef struct {
    uint16_t x, y;
    uint16_t width, height;
    size_t size;
    pvr_ptr_t vram_ptr;
} KDTBlock;

typedef KDTBlock KDTSliceTable[KDT_MAX_SLICES][KDT_MAX_SLICES];


typedef struct Texture_s {
    size_t data_size;
    pvr_ptr_t data_vram;
    void* data_ram;
    ArrayBuffer source_arraybuffer;

    int32_t width;
    int32_t height;
    int32_t original_width;
    int32_t original_height;

    int32_t id;
    char* src_filename;
    int32_t references;
    bool has_mipmaps;

    int format;
    float scale_factor_width;
    float scale_factor_height;
    bool is_data_ram_from_kdt;
    KDTSliceTable* slices;
}* Texture;


Texture texture_init(const char* src);
Texture texture_init_deferred(const char* src, bool deffered);
Texture texture_init_from_raw(void* ptr, size_t size, bool in_vram, int32_t width, int32_t height, int32_t orig_width, int32_t orig_height);

Texture texture_share_reference(Texture texture);
void texture_upload_to_pvr(Texture texture);
void texture_get_original_dimmensions(Texture texture, float* original_width, float* original_height);
void texture_get_dimmensions(Texture texture, int32_t* width, int32_t* height);

void texture_destroy_force(Texture* texture);
void texture_destroy(Texture* texture);


#endif
