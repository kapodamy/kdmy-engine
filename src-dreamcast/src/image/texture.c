#include "texture.h"

#include <assert.h>
#include <malloc.h>

#include <dc/pvr.h>
#include <dc/sq.h>

#include "fs.h"
#include "io.h"
#include "kdt.h"
#include "malloc_utils.h"
#include "map.h"
#include "math2d.h"
#include "number_format_specifiers.h"
#include "stringutils.h"

#define TEXTURE_MAX_BYTE_SIZE (6 * 1024 * 1024) // 6MiB
#define TEXTURE_LZSS_WINDOW 128


typedef struct {
    const uint8_t* data_ptr;
    const uint8_t* data_end;
    bool fill_window;

    pvr_ptr_t vram_data;
    uintptr_t vram_data_ptr;

    size_t window_used;
    size_t out_buffer_used;

    size_t chunk_written;
    size_t chunk_size;

    uint8_t window[TEXTURE_LZSS_WINDOW * 4];
    uint8_t out_buffer[TEXTURE_LZSS_WINDOW * 4];
} LZSSCTX;

typedef struct {
    ArrayBuffer source;
    KDT* kdt;
    size_t length;
    char* absolute_path;
} KDTWrapper;

typedef struct {
    LZSSCTX lzss_ctx;
    size_t offset;
} KDTContext;


static int32_t TEXTURE_IDS = 0;
static Map TEXTURE_POOL = NULL;


static Texture texture_get_from_global_pool(const char* src);
static KDTWrapper fs_readimagebitmap(const char* src);
static bool texture_internal_lzss_integrity_check(const uint8_t* data, size_t compressed_size);
static uint16_t texture_internal_calc_slice_block_dimmen(uint16_t offset, uint16_t subslice_offset, uint16_t frame_dimmen);
static KDTSliceTable* texture_internal_build_kdt_blocks_list(KDT* kdt, size_t kdt_data_size, bool* failed);
static void texture_internal_free_from_vram(Texture texture);
static pvr_ptr_t texture_internal_upload_kdt_to_pvr(KDT* kdt, KDTContext* context, size_t slice_size);
static pvr_ptr_t lzss_buff_dec(LZSSCTX* ctx, size_t chunk_size);
static LZSSCTX lzss_buff_dec_prepare(const uint8_t* compressed_data, size_t compressed_data_size);


void __attribute__((constructor)) __ctor_texture() {
    TEXTURE_POOL = map_init();
}


Texture texture_init(const char* src) {
    return texture_init_deferred(src, false);
}

Texture texture_init_deferred(const char* src, bool deffered) {
    Texture loaded_texture = texture_get_from_global_pool(src);
    if (loaded_texture) {
        // This texture is already in use, bump the reference count and reuse it
        loaded_texture->references++;
        return loaded_texture;
    }

    KDTWrapper kdtwpr = fs_readimagebitmap(src);
    KDT* kdt = kdtwpr.kdt;
    if (!kdt) return NULL;

    int format;
    switch ((KDT_PixelFormat)kdt->pixel_format) {
        case KDT_PixelFormat_YUV422:
            format = PVR_TXRFMT_YUV422;
            break;
        case KDT_PixelFormat_RGB565:
            format = PVR_TXRFMT_RGB565;
            break;
        case KDT_PixelFormat_ARGB1555:
            format = PVR_TXRFMT_ARGB1555;
            break;
        case KDT_PixelFormat_ARGB4444:
            format = PVR_TXRFMT_ARGB4444;
            break;
        default:
            format = 0x00;
            break;
    }

    format |= PVR_TXRFMT_NOSTRIDE;

    if (kdt->flags & KDT_HEADER_FLAG_VQ)
        format |= PVR_TXRFMT_VQ_ENABLE;
    else
        format |= PVR_TXRFMT_VQ_DISABLE;

    if (kdt->flags & KDT_HEADER_FLAG_TWIDDLE)
        format |= PVR_TXRFMT_TWIDDLED;
    else
        format |= PVR_TXRFMT_NONTWIDDLED;

    bool failed;
    KDTSliceTable* slices = texture_internal_build_kdt_blocks_list(kdt, kdtwpr.length, &failed);
    if (failed) {
        logger_error("texture_init_deffered() failed to build slices of %s", kdtwpr.absolute_path);
        arraybuffer_destroy(&kdtwpr.source);
        return NULL;
    }

    Texture texture = texture_init_from_raw(
        kdt, kdtwpr.length, false,
        kdt->frame_width, kdt->frame_height,
        kdt->original_width, kdt->original_height
    );

    texture->format = format;
    texture->scale_factor_width = (kdt->original_width - kdt->padding_width) / (float)kdt->encoded_width;
    texture->scale_factor_height = (kdt->original_height - kdt->padding_height) / (float)kdt->encoded_height;
    texture->is_data_ram_from_kdt = true;
    texture->slices = slices;
    texture->source_arraybuffer = kdtwpr.source;

    // remember the filename to avoid duplicated textures
    texture->src_filename = kdtwpr.absolute_path;

    if (!deffered) texture_upload_to_pvr(texture);

    return texture;
}

Texture texture_init_from_raw(void* ptr, size_t size, bool in_vram, int32_t width, int32_t height, int32_t orig_width, int32_t orig_height) {
    Texture texture = malloc_chk(sizeof(struct Texture_s));
    malloc_assert(texture, Texture);

    *texture = (struct Texture_s){
        .data_size = size,
        .data_vram = NULL,
        .data_ram = NULL,
        .source_arraybuffer = NULL,

        .width = width,
        .height = height,
        .original_width = orig_width,
        .original_height = orig_height,

        .id = TEXTURE_IDS++,
        .src_filename = NULL,
        .references = 1,
        .has_mipmaps = false,

        .scale_factor_width = 1.0f,
        .scale_factor_height = 1.0f,
        .is_data_ram_from_kdt = false,
        .format = 0x00,
        .slices = NULL
    };

    if (in_vram)
        texture->data_vram = ptr;
    else
        texture->data_ram = ptr;

    map_add(TEXTURE_POOL, texture->id, texture);

    return texture;
}

Texture texture_share_reference(Texture texture) {
    texture->references++;
    return texture;
}

void texture_upload_to_pvr(Texture texture) {
    if (texture->data_vram) return;

    if (!texture->data_ram) {
        // NULL data loaded
        return;
    }

    KDT* kdt = (KDT*)texture->data_ram;
    KDTContext kdtctx = {
        .lzss_ctx = lzss_buff_dec_prepare(kdt->data, texture->data_size),
        .offset = 0
    };

    if (texture->slices) {
        uint8_t* data_ptr = texture->is_data_ram_from_kdt ? kdt->data : texture->data_ram;

        for (size_t block_y = 0; block_y < KDT_MAX_SLICES; block_y++) {
            for (size_t block_x = 0; block_x < KDT_MAX_SLICES; block_x++) {
                KDTBlock* block = &(*texture->slices)[block_y][block_x];
                if (block->width == 0 || block->height == 0) continue;

                if (texture->is_data_ram_from_kdt) {
                    block->vram_ptr = texture_internal_upload_kdt_to_pvr(kdt, &kdtctx, block->size);
                } else {
                    block->vram_ptr = pvr_mem_malloc(block->size);
                    if (block->vram_ptr) pvr_txr_load(data_ptr, block->vram_ptr, block->size);
                    data_ptr += block->size;
                }

                if (block->vram_ptr) {
                    // placeholder for pvr_context_draw_texture() function
                    texture->data_vram = block->vram_ptr;
                } else {
                    logger_error(
                        "texture_upload_to_pvr() no enough video memory for slice, required=%u available=" FMT_U4 "",
                        block->size, pvr_mem_available()
                    );
                }
            }
        }

        goto L_return;
    }

    size_t data_size;
    if (texture->is_data_ram_from_kdt) {
        data_size = (size_t)(texture->width * texture->height) * sizeof(uint16_t);
        if (kdt->flags & KDT_HEADER_FLAG_VQ) data_size = (data_size / 8) + KDT_VQ_CODEBOOK_SIZE;
        texture->data_vram = texture_internal_upload_kdt_to_pvr(kdt, &kdtctx, data_size);
    } else {
        data_size = texture->data_size;
        texture->data_vram = pvr_mem_malloc(data_size);
        if (texture->data_vram != NULL) pvr_txr_load(texture->data_ram, texture->data_vram, data_size);
    }

    if (!texture->data_vram) {
        logger_error(
            "texture_upload_to_pvr() no enough video memory, required=%u available=" FMT_U4 "",
            data_size, pvr_mem_available()
        );
        goto L_return;
    }

L_return:
    if (texture->source_arraybuffer)
        arraybuffer_destroy(&texture->source_arraybuffer);
    else
        free_chk(texture->data_ram);

    texture->data_ram = NULL;
    texture->is_data_ram_from_kdt = false;
    return;
}

void texture_get_original_dimmensions(Texture texture, float* original_width, float* original_height) {
    *original_width = texture->original_width;
    *original_height = texture->original_height;
}

void texture_get_dimmensions(Texture texture, int32_t* width, int32_t* height) {
    *width = texture->width;
    *height = texture->height;
}


void texture_destroy_force(Texture* texture_ptr) {
    if (!texture_ptr || !*texture_ptr) return;

    (*texture_ptr)->references = 0;
    texture_destroy(texture_ptr);
}

void texture_destroy(Texture* texture_ptr) {
    if (!texture_ptr || !*texture_ptr) return;

    Texture texture = *texture_ptr;
    if (!texture) return;

    texture->references--;

    // check if the texture is shared somewhere
    if (texture->references > 0) {
        return;
    }

    if (!texture->source_arraybuffer && texture->data_ram) {
        free_chk(texture->data_ram);
    }
    if (texture->data_vram) {
        texture_internal_free_from_vram(texture);
    }
    if (texture->source_arraybuffer) {
        arraybuffer_destroy(&texture->source_arraybuffer);
    }

    map_delete(TEXTURE_POOL, texture->id);

    free_chk(texture->src_filename);
    free_chk(texture->slices);
    free_chk(texture);

    *texture_ptr = NULL;
    return;
}


static Texture texture_get_from_global_pool(const char* src) {
    Texture texture = NULL;
    if (src) {
        char* filename = fs_get_full_path_and_override(src);
        foreach (Texture, loaded_texture, MAP_ITERATOR, TEXTURE_POOL) {
            if (string_equals_ignore_case(loaded_texture->src_filename, filename)) {
                texture = loaded_texture;
                break;
            }
        }
        free_chk(filename);
    }
    return texture;
}

static KDTWrapper fs_readimagebitmap(const char* src) {
    char* absolute_path = fs_get_full_path_and_override(src);
    ArrayBuffer arraybuffer = io_read_arraybuffer(absolute_path);

    if (!arraybuffer) goto L_failed;

    size_t length = arraybuffer->length;
    KDT* kdt = (KDT*)arraybuffer->data;

    length -= sizeof(KDT);
    if (length > TEXTURE_MAX_BYTE_SIZE) goto L_failed;

    if (kdt->signature != KDT_SIGNATURE) {
        logger_error("fs_readimagebitmap() the file '%s' is not a KDT texture.", src);
        goto L_failed;
    }

    switch ((KDT_PixelFormat)kdt->pixel_format) {
        case KDT_PixelFormat_YUV422:
        case KDT_PixelFormat_RGB565:
        case KDT_PixelFormat_ARGB1555:
        case KDT_PixelFormat_ARGB4444:
            break;
        default:
#ifdef DEBUG
            logger_error("fs_readimagebitmap() unknown pixel format: 0x%x", kdt->pixel_format);
#endif
            goto L_failed;
    }

    //
    // integrity checks
    //
    if (kdt->encoded_width < 1 || kdt->encoded_height < 1) goto L_failed;
    if (kdt->encoded_width > kdt->original_width || kdt->encoded_height > kdt->original_height) goto L_failed;
    if (kdt->encoded_width > kdt->frame_width || kdt->encoded_height > kdt->frame_height) goto L_failed;

    if (~kdt->flags & KDT_HEADER_FLAG_UPERSLICE) {
        if (kdt->encoded_width > KDT_MAX_DIMMEN || kdt->encoded_height > KDT_MAX_DIMMEN) goto L_failed;
    }

    if (kdt->flags & KDT_HEADER_FLAG_SUBSLICE) {
        if (kdt->subslice_cutoff_x > kdt->frame_width) goto L_failed;
        if (kdt->subslice_cutoff_y > kdt->frame_height) goto L_failed;
        if (kdt->subslice_cutoff_x == 0 && kdt->subslice_cutoff_y == 0) goto L_failed;
    } else {
        if (kdt->subslice_cutoff_x != 0 && kdt->subslice_cutoff_y != 0) goto L_failed;
    }

    if (kdt->flags & KDT_HEADER_FLAG_OPACITYSLICE) {
        if (kdt->padding_width == 0 && kdt->padding_height == 0) goto L_failed;
        if (kdt->padding_width >= kdt->original_width) goto L_failed;
        if (kdt->padding_height >= kdt->original_height) goto L_failed;
    } else {
        if (kdt->padding_width != 0) goto L_failed;
        if (kdt->padding_height != 0) goto L_failed;
    }

    if (kdt->flags & KDT_HEADER_FLAG_LZSS) {
        // read uncompressed size from LZSS stream
        uint32_t lzss_uncompresed_size = *((uint32_t*)kdt->data);
        if (lzss_uncompresed_size > TEXTURE_MAX_BYTE_SIZE) goto L_failed;

        if (!texture_internal_lzss_integrity_check(kdt->data, length)) {
            // corrupted or truncated lzss data
            goto L_failed;
        }

        assert(lzss_uncompresed_size > length);
    }

    return (KDTWrapper){
        .source = arraybuffer,
        .kdt = kdt,
        .length = length,
        .absolute_path = absolute_path
    };

L_failed:
    free_chk(absolute_path);
    if (arraybuffer) arraybuffer_destroy(&arraybuffer);
    return (KDTWrapper){.source = NULL, .kdt = NULL, .length = 0, .absolute_path = NULL};
}



static bool texture_internal_lzss_integrity_check(const uint8_t* data, size_t compressed_size) {
    uint32_t uncompressed_size = *((uint32_t*)data);
    size_t written = 0;
    const uint8_t* data_end = data + compressed_size;

    data += sizeof(uint32_t);

    while (data < data_end) {
        uint16_t token = *((uint16_t*)data);

        size_t pattern_offset = (token & 0xFC00) >> 10;
        size_t pattern_size = (token & 0x03E0) >> 5;
        size_t literal_size = (token & 0x001F) >> 0;

        data += sizeof(uint16_t);

        if (pattern_size == 0 && literal_size == 0) {
            if (pattern_offset < 1) {
                // inavalid size
                goto L_failed;
            }

            written += pattern_offset;
            if (written > uncompressed_size) {
                // something went wrong
                goto L_failed;
            }

            data++;
            continue;
        }

        written += pattern_size + literal_size;
        if (written > uncompressed_size) {
            // something went wrong
            goto L_failed;
        }

        /*if (pattern_size > 0) {
            size_t backward_offset = pattern_offset + pattern_size;
        }*/


        if (literal_size > 0) {
            // copy literal (no compressed bytes)
            data += literal_size;
        }
    }

    if ((written < 1 && uncompressed_size > 0) || written > uncompressed_size) {
        // this never should happen
        goto L_failed;
    }
    return true;

L_failed:
    return false;
}

static uint16_t texture_internal_calc_slice_block_dimmen(uint16_t offset, uint16_t subslice_offset, uint16_t frame_dimmen) {
    if (subslice_offset > 0) {
        if (offset < subslice_offset) {
            uint16_t block_subslice_dimmen = subslice_offset - offset;
            if (block_subslice_dimmen < KDT_MAX_DIMMEN) {
                return block_subslice_dimmen;
            }
        } else if (offset == subslice_offset) {
            return frame_dimmen - subslice_offset;
        }
    }

    uint16_t block_dimmen = frame_dimmen - offset;
    if (block_dimmen > KDT_MAX_DIMMEN) {
        return KDT_MAX_DIMMEN;
    }

    return block_dimmen;
}

static KDTSliceTable* texture_internal_build_kdt_blocks_list(KDT* kdt, size_t texture_size, bool* failed) {
#ifdef DEBUG
    assert(sizeof(uintptr_t) >= sizeof(pvr_ptr_t));
#endif

    if (kdt->flags & KDT_HEADER_FLAG_LZSS) {
        texture_size = *((uint32_t*)kdt->data);
    }

    uint8_t* data_ptr = kdt->data;
    uint8_t* data_end_ptr = kdt->data + texture_size;

    KDTSliceTable* slices = malloc_chk(sizeof(KDTSliceTable));
    malloc_assert(slices, KDTSliceTable);
    memset(slices, 0x00, sizeof(KDTSliceTable));

    uint16_t offset_x = 0, offset_y = 0;
    uint8_t block_x = 0, block_y = 0;
    uint8_t slice_count = 0;
    uintptr_t data_offset = 0;

    while (offset_y < kdt->frame_height) {
        uint16_t block_width = texture_internal_calc_slice_block_dimmen(offset_x, kdt->subslice_cutoff_x, kdt->frame_width);
        uint16_t block_height = texture_internal_calc_slice_block_dimmen(offset_y, kdt->subslice_cutoff_y, kdt->frame_height);

        size_t block_size = block_width * block_height * sizeof(uint16_t);
        if (kdt->flags & KDT_HEADER_FLAG_VQ) block_size = (block_size / 8) + KDT_VQ_CODEBOOK_SIZE;

        if (block_x >= KDT_MAX_SLICES || block_y >= KDT_MAX_SLICES) {
#ifdef DEBUG
            logger_error("texture_internal_build_kdt_blocks_list() too many slices, maximum is %ix%i", KDT_MAX_SLICES, KDT_MAX_SLICES);
#endif
            goto L_failed;
        }

        if (data_ptr > data_end_ptr || (size_t)(data_end_ptr - data_ptr) < block_size) {
#ifdef DEBUG
            logger_error("texture_internal_build_kdt_blocks_list() truncated file");
#endif
            goto L_failed;
        }

        // must be "slices[y][x]" not "slices[x][y]"
        (*slices)[block_y][block_x] = (KDTBlock){
            .x = offset_x,
            .y = offset_y,
            .width = block_width,
            .height = block_height,
            .size = block_size,
            .vram_ptr = NULL
        };

        // seek texture data
        data_offset += block_size;
        data_ptr += block_size;
        slice_count++;

        // move to the next block
        offset_x += block_width;
        block_x++;
        if (offset_x >= kdt->frame_width) {
            offset_x = 0;
            block_x = 0;
            offset_y += block_height;
            block_y++;
        }
    }

    *failed = false;

    if (slice_count < 2) {
        // single block texture, nothing to do
        free_chk(slices);
        slices = NULL;
    }

    return slices;

L_failed:
    *failed = true;
    free_chk(slices);
    return NULL;
}

static void texture_internal_free_from_vram(Texture texture) {
    if (!texture->slices) {
        pvr_mem_free(texture->data_vram);
        return;
    }

    for (size_t block_y = 0; block_y < KDT_MAX_SLICES; block_y++) {
        for (size_t block_x = 0; block_x < KDT_MAX_SLICES; block_x++) {
            KDTBlock* block = &(*texture->slices)[block_y][block_x];
            if (block->width == 0 || block->height == 0) break;

            if (block->vram_ptr) {
                pvr_mem_free(block->vram_ptr);
                block->vram_ptr = NULL;
            }
        }
    }
}

static pvr_ptr_t texture_internal_upload_kdt_to_pvr(KDT* kdt, KDTContext* context, size_t slice_size) {
    if (kdt->flags & KDT_HEADER_FLAG_LZSS) {
        return lzss_buff_dec(&context->lzss_ctx, slice_size);
    }

    pvr_ptr_t vram = pvr_mem_malloc(slice_size);
    if (vram != NULL) pvr_txr_load(&kdt->data[context->offset], vram, slice_size);

    context->offset += slice_size;
    return vram;
}


static bool lzss_buf_dec_recv(LZSSCTX* ctx, const uint8_t* window_ptr, size_t size) {
    size_t flush_amount;
    size_t total = ctx->out_buffer_used;
    size_t remaning = ctx->chunk_size - ctx->chunk_written;

    if (window_ptr) {
        total += size;
        memcpy(&ctx->out_buffer[ctx->out_buffer_used], window_ptr, size);
        flush_amount = total & (~((size_t)0x1F));
    } else {
        flush_amount = total;
    }

    if (flush_amount > remaning) flush_amount = remaning;

    if (flush_amount > 0) {
        if (ctx->vram_data_ptr != 0) {
            // load to vram
            pvr_txr_load(ctx->out_buffer, (pvr_ptr_t)ctx->vram_data_ptr, flush_amount);
            ctx->vram_data_ptr += flush_amount;
        }

        ctx->chunk_written += flush_amount;
        total -= flush_amount;
        memmove(ctx->out_buffer, &ctx->out_buffer[flush_amount], total);
    }

    ctx->out_buffer_used = total;
    return ctx->chunk_written >= ctx->chunk_size;
}

static pvr_ptr_t lzss_buff_dec(LZSSCTX* ctx, size_t chunk_size) {
    const uint8_t* data_ptr = ctx->data_ptr;
    const uint8_t* data_end = ctx->data_end;

    if (data_ptr >= data_end) return false;

    ctx->chunk_size = chunk_size;
    ctx->chunk_written = 0;
    ctx->vram_data = pvr_mem_malloc(chunk_size);
    ctx->vram_data_ptr = (uintptr_t)ctx->vram_data;

    size_t window_used = ctx->window_used;
    bool fill_window = ctx->fill_window;
    uint8_t* buffer_ptr = &ctx->window[fill_window ? window_used : (window_used + TEXTURE_LZSS_WINDOW)];

    while (true) {
        uint16_t token = *((uint16_t*)data_ptr);
        data_ptr += sizeof(uint16_t);

        size_t pattern_offset = (token & 0xFC00) >> 10;
        size_t pattern_size = (token & 0x03E0) >> 5;
        size_t literal_size = (token & 0x001F) >> 0;

        if (pattern_size == 0 && literal_size == 0) {
            uint8_t byte = *data_ptr++;

            memset(buffer_ptr, byte, pattern_offset);

            buffer_ptr += pattern_offset;
            window_used += pattern_offset;
        } else {
            if (pattern_size > 0) {
                size_t backward_offset = pattern_offset + pattern_size;
                memmove(buffer_ptr, buffer_ptr - backward_offset, pattern_size);

                buffer_ptr += pattern_size;
                window_used += pattern_size;
            }
            if (literal_size > 0) {
                memcpy(buffer_ptr, data_ptr, literal_size);
                data_ptr += literal_size;

                buffer_ptr += literal_size;
                window_used += literal_size;
            }
        }

        /************************************/
        if (fill_window) {
            if (window_used >= TEXTURE_LZSS_WINDOW) {
                window_used -= TEXTURE_LZSS_WINDOW;
                fill_window = false;
                if (lzss_buf_dec_recv(ctx, ctx->window, TEXTURE_LZSS_WINDOW)) break;
            }
            if (data_ptr < data_end) continue;
        }

        if (data_ptr >= data_end || window_used >= 128) {
            if (data_ptr >= data_end && window_used < 1) {
                break;
            }

            bool stop = lzss_buf_dec_recv(ctx, buffer_ptr - window_used, window_used);

            memmove(ctx->window, &ctx->window[window_used], window_used);
            buffer_ptr -= window_used;
            window_used = 0;

            if (data_ptr >= data_end) {
                assert(window_used < 1);
                break;
            } else if (stop) {
                break;
            }
        }
    }

    // flush buffer
    lzss_buf_dec_recv(ctx, NULL, 0);

    ctx->data_ptr = data_ptr;
    ctx->window_used = window_used;
    ctx->fill_window = fill_window;

    return ctx->vram_data;
}

static LZSSCTX lzss_buff_dec_prepare(const uint8_t* compressed_data, size_t compressed_data_size) {
    _Static_assert(sizeof(uintptr_t) >= sizeof(pvr_ptr_t));

    return (LZSSCTX){
        .data_ptr = compressed_data + sizeof(uint32_t),
        .data_end = compressed_data + compressed_data_size,
        .fill_window = true,

        .vram_data = NULL,
        .vram_data_ptr = 0,

        .window_used = 0,
        .out_buffer_used = 0,

        .chunk_written = 0,
        .chunk_size = 0
    };
}
