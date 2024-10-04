#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch/irq.h>
#include <arch/timer.h>
#include <dc/flashrom.h>
#include <dc/pvr.h>
#include <dc/video.h>
#include <kos/thread.h>

#include "externals/luascript_platform.h"
#include "kdt.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"

#define PVRCONTEXT_CLEAR_COLOR ((RGBA){0.5f, 0.5f, 0.5f, 1.0f})
#define PREVIOUS_STATE(PVRCTX) (PVRCTX->stack_index > 0 ? &PVRCTX->stack[PVRCTX->stack_index - 1] : NULL)

static const float MIN_FRAME_FPS = 15.0f;
static const uint64_t MIN_FRAME_TIME_NS = (1000UL * 1000UL * 1000UL) / MIN_FRAME_FPS;
static const uint64_t THRESHOLD_FRAME_TIME_TO_ZERO = 200UL * 1000UL * 1000UL; // 5fps
const RGBA PVR_DEFAULT_OFFSET_COLOR = {1.0f, 1.0f, 1.0f, -1.0f};
float pvr_context_current_z_index = 0.0f;

struct PVRContext_s pvr_context = {
    .last_timestamp = 0,
    .resolution_changes = 0,
    .last_elapsed = 0,
    //.frame_rendered = 0,
    //.native_window_title = NULL,

    //.shader_framebuffer_front = NULL,
    //.shader_framebuffer_back = NULL,
    //.target_framebuffer = NULL,
    //.shader_needs_flush = false,
    //.shader_last_resolution_changes = 0,

    .stack_index = 0,

    .vertex_alpha = 1.0f,

    .global_alpha = 1.0f,
    .global_offsetcolor = NULL,

    .global_antialiasing = PVRCTX_FLAG_DEFAULT,
    .vertex_antialiasing = PVRCTX_FLAG_DEFAULT,

    .global_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT,
    .vertex_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT,

    .render_alpha = 1.0f,
    .render_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT,
    .render_antialiasing = PVRCTX_FLAG_DEFAULT,

    .options = {
        .blend_enabled = false,
        .blend_src_rgb = BLEND_DEFAULT,
        .blend_dst_rgb = BLEND_DEFAULT,
        .blend_src_alpha = BLEND_DEFAULT,
        .blend_dst_alpha = BLEND_DEFAULT,
        .darken = false
    }
};


static int flashrom_get_region2();
static int get_pvr_blending(Blend blend);
static inline void draw_polygon(uint32_t vertexcolor, uint32_t offsetcolor, float dx, float dy, float dw, float dh, float u0, float v0, float u1, float v1);


void pvr_context_init() {
    pvr_init(&(pvr_init_params_t){
        // Enable only translucent polygons with size 16
        .opb_sizes = {PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0},

        // Vertex buffer size
        .vertex_buf_size = PVRCTX_STATE_VERTEX_BUFFER_SIZE,

        // ¿Enable DMA?
        .dma_enabled = PVRCTX_DMA_ENABLED,

        // No FSAA
        .fsaa_enabled = 0,

        // Translucent Autosort disabled.
        .autosort_disabled = 1,

        // Extra OPBs
        .opb_overflow_count = 0
    });

#if PVRCTX_DMA_ENABLED
    if (pvr_vertex_dma_enabled() != 0) {
        void* buffer = memalign_chk(32, PVRCTX_STATE_VERTEX_BUFFER_SIZE * 2);
        pvr_set_vertbuf(PVR_LIST_TR_POLY, buffer, PVRCTX_STATE_VERTEX_BUFFER_SIZE * 2);
    }
#endif

#ifndef INIT_NO_VID_EMPTY
    pvr_context_clear_screen(&pvr_context, PVRCONTEXT_CLEAR_COLOR);
#endif

    for (size_t i = 0; i < PVRCTX_STATE_STACK_LENGTH; i++) {
        pvr_context.stack[i] = (PVRContextState){
            .global_alpha = 1.0f,
            .global_antialiasing = PVRCTX_FLAG_DEFAULT,
            .global_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT,
            //.added_shaders = 0
        };
        sh4matrix_clear(pvr_context.stack[i].matrix);
        pvr_context_helper_clear_offsetcolor(pvr_context.stack[i].offsetcolor);
    }
    pvr_context.global_offsetcolor = &pvr_context.stack[0].offsetcolor;
    pvr_context_helper_clear_offsetcolor(pvr_context.vertex_offsetcolor);
    pvr_context_helper_clear_offsetcolor(pvr_context.render_offsetcolor);

    pvr_context.current_matrix = pvr_context.stack[0].matrix;

    // pvr_context.screen_stride = pvr_context.screen_width;
    //  pvr_context.native_window_title = "KDMY Engine - Friday Night Funkin";

    //
    // show init info
    //
    // Notes: KallistiOS does not recognize "xxx43" regions, only "xxxx1"
    int region = flashrom_get_region2();
    if (region == FLASHROM_REGION_UNKNOWN) region = flashrom_get_region();

    puts("\n");
    logger_info("PVRContext initialized:");

    printf("    Region=");
    switch (region) {
        case FLASHROM_REGION_EUROPE:
            printf("Europe (50Hz/60Hz tweak not supported)");
            break;
        case FLASHROM_REGION_JAPAN:
            printf("Japan");
            break;
        case FLASHROM_REGION_US:
            printf("Europe");
            break;
        case FLASHROM_REGION_UNKNOWN:
            printf("(Unknown)");
            break;
        default:
            if (region < 0)
                printf("(Error %i)", region);
            else
                printf("(Id %i)", region);
            break;
    }

    printf("  Output=");
    int8_t cable = vid_check_cable();
    switch (cable) {
        case CT_ANY:
            printf("ANY");
            break;
        case CT_COMPOSITE:
            printf("COMPOSITE");
            break;
        case CT_NONE:
            printf("NONE");
            break;
        case CT_RGB:
            printf("RGB");
            break;
        case CT_VGA:
            printf("VGA");
            break;
        default:
            printf("(Unknown %i)", region);
            break;
    }

    printf("  Resolution=%ix%i\n", vid_mode->width, vid_mode->height);

    pvr_context.screen_width = vid_mode->width;
    pvr_context.screen_height = vid_mode->height;

    // gl_helper_initialize();
    // pvr_context.shader_stack = stacklist_init(PVRCTX_SHADER_STACK_LENGTH);
}

float pvrctx_wait_ready() {
#ifdef _arch_dreamcast
    if (pvr_context_current_z_index != 0.0f) {
        pvr_list_finish();
        pvr_scene_finish();
        pvr_context_current_z_index = 0.0f;
    }
#else
    // flush front framebuffer if there anything drawn
    pvr_context.FlushFramebuffer();
    pvr_context.shader_framebuffer_front.Invalidate();
    pvr_context.shader_framebuffer_back.Invalidate();

    mastervolume_draw(pvr_context);

    if (SETTINGS.show_fps) pvr_context_draw_fps(); // if enabled draw it

    if (pvr_context.is_deterministic)
        thd_sleep(1);
        /*else if (!pvr_context.vsync_enabled)
            thd_pass();*/
#endif

    pvr_wait_ready();

    uint64_t now = timer_ns_gettime64();
    float elapsed;
    if (pvr_context.last_timestamp == 0) {
        elapsed = 0.0f;
    } else {
        uint64_t diff = now - pvr_context.last_timestamp;

        //
        // Notes:
        //          * the engine should not run below 15fps, or the beat synchronization will be lost.
        //          * the "diff" should not exceed the INT32_MAX othewise will overflow to negative.
        //          * set elapsed to zero is bigger than 200ms, this happen between load screens,
        //          * do not fire threshold twice, check if last elapsed value was zero.
        //
        if (diff >= THRESHOLD_FRAME_TIME_TO_ZERO && pvr_context.last_elapsed != 0.0f)
            elapsed = 0.0f;
        if (diff > MIN_FRAME_TIME_NS)
            elapsed = MIN_FRAME_FPS;
        else
            elapsed = (float)diff / (1000.0f * 1000.0f);
    }

    pvr_context.last_timestamp = now;
    pvr_context.last_elapsed = elapsed;

#ifndef _arch_dreamcast
    pvr_context.frame_rendered++;

    // resize framebuffers if the screen size has changed
    pvr_context.CheckFrameBufferSize();
#endif

    // notify all lua scripts about gamepad/screen changes
    luascript_platform_poll_window_state();

    return elapsed;
}


#ifndef _arch_dreamcast
void PVRContext_FlushFramebuffer(PVRContext* pvrctx) {
    if (!pvrctx->shader_needs_flush) return;

    PSFramebuffer front = pvrctx->shader_framebuffer_front;
    PSFramebuffer back = pvrctx->shader_framebuffer_back;
    int32_t count = stacklist_count(pvrctx->shader_stack);
    int32_t last_index = count - 1;

    for (int32_t i = 0; i < count; i++) {
        if (i == last_index)
            PVRContext_UseDefaultFramebuffer(pvrctx);
        else
            PSFramebuffer_Use(back, true);

        PSShader_Draw(stacklist_get(pvrctx->shader_stack, i), front);

        PSFramebuffer tmp = front;
        front = back;
        back = tmp;
    }

    pvrctx->shader_needs_flush = false;
}

void PVRContext_ShaderStackPush(PVRContext* pvrctx, PSShader psshader) {
    if (!psshader) {
        logger_error("psshader can not be null");
        assert(psshader);
    }

    PVRContext_FlushFramebuffer(pvrctx);

    if (stacklist_count(pvrctx->shader_stack) >= PVRCTX_SHADER_STACK_LENGTH) {
        logger_warn("PVRContext::ShaderStackPush() failed, the stack is full");
        return false;
    }

    stacklist_push(pvrctx->shader_stack, psshader);
    PSFramebuffer_Use(pvrctx->shader_framebuffer_front, true);

    return true;
}

PVRContext_ShaderStackPop(PVRContext* pvrctx, int32_t count) {
    PVRContext_FlushFramebuffer(pvrctx);

    if (stacklist_count(pvrctx->shader_stack) < 1) {
        logger_warn("PVRContext::ShaderStackPop() failed, the stack is empty");
        return false;
    }

    stacklist_pop2(pvrctx->shader_stack, count);

    if (stacklist_count(pvrctx->shader_stack) < 1)
        PVRContext_UseDefaultFramebuffer(pvrctx);
    else
        PSFramebuffer_Use(pvrctx->shader_framebuffer_front, true);

    return true;
}

PVRContext_CheckFramebufferSize(PVRContext* pvrctx) {
    if (pvrctx->resolution_changes == pvrctx->shader_last_resolution_changes) return;
    gl_helper_resize_projection(0, 0, pvrctx->screen_width, pvrctx->screen_height);

    pvrctx->shader_last_resolution_changes = pvrctx->resolution_changes;
    PSFramebuffer_Resize(pvrctx->shader_framebuffer_front);
    PSFramebuffer_Resize(pvrctx->shader_framebuffer_back);
    PSFramebuffer_ResizeQuadScreen(pvrctx);
}

PVRContext_UseDefaultFramebuffer(PVRContext* pvrctx) {
    if (pvrctx->target_framebuffer)
        PSFramebuffer_Use(pvrctx->target_framebuffer, false);
    else
        PSFramebuffer_UseScreenFramebuffer(pvrctx);
}
#endif

void pvr_context_reset(PVRContext pvrctx) {
    pvrctx->stack_index = 0;
    sh4matrix_clear(pvrctx->stack[0].matrix);

    pvrctx->stack[0].global_alpha = 1.0f;
    pvrctx->global_offsetcolor = &pvrctx->stack[0].offsetcolor;

    pvrctx->global_alpha = 1.0f;
    pvrctx->vertex_alpha = 1.0f;
    pvrctx->render_alpha = 1.0f;

    pvr_context_helper_clear_offsetcolor(*pvrctx->global_offsetcolor);
    pvr_context_helper_clear_offsetcolor(pvrctx->vertex_offsetcolor);

    pvrctx->global_antialiasing = PVRCTX_FLAG_ENABLE;
    pvrctx->vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx->render_antialiasing = PVRCTX_FLAG_DEFAULT;

    pvrctx->global_offsetcolor_multiply = PVRCTX_FLAG_ENABLE;
    pvrctx->vertex_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;
    pvrctx->render_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;

    pvrctx->current_matrix = pvrctx->stack[0].matrix;

    // PVRContext_FlushFramebuffer(pvrctx);
    // stacklist_clear(pvrctx->shader_stack);
    // PSFramebuffer_UseScreenFramebuffer(pvrctx);
    // gl_helper_set_blend(1, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT);
    // gl_helper.program_textured.darken_enabled = false;
}

void pvr_context_apply_modifier(PVRContext pvrctx, Modifier* modifier) {
    PVRContextState* state = &pvrctx->stack[pvrctx->stack_index];
    sh4matrix_apply_modifier(state->matrix, modifier);
}

void pvr_context_apply_modifier2(PVRContext pvrctx, Modifier* modifier, float draw_x, float draw_y, float draw_width, float draw_height) {
    PVRContextState* state = &pvrctx->stack[pvrctx->stack_index];
    sh4matrix_apply_modifier2(state->matrix, modifier, draw_x, draw_y, draw_width, draw_height);
}

void pvr_context_clear_screen(PVRContext pvrctx, const RGBA rgba_color) {
    (void)pvrctx;

    vid_clear(
        (uint8_t)(rgba_color[0] * 255.0f),
        (uint8_t)(rgba_color[1] * 255.0f),
        (uint8_t)(rgba_color[2] * 255.0f)
    );
}

void pvr_context_prepare(PVRContext pvrctx) {
    (void)pvrctx;

    if (pvr_context_current_z_index == 0.0f) {
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_TR_POLY);
        pvr_context_current_z_index = 1.0f;
    }
}

void pvr_context_flush(PVRContext pvrctx) {
    (void)pvrctx;
}

bool pvr_context_save(PVRContext pvrctx) {
    uint16_t next_index = pvrctx->stack_index + 1;

    if (pvrctx->stack_index >= PVRCTX_STATE_STACK_LENGTH) {
        logger_error("pvr_context_save() the PVRContext stack is full");
        return false;
    }

    // int old_irq = irq_disable();

    PVRContextState* previous_state = &pvrctx->stack[pvrctx->stack_index];
    PVRContextState* next_state = &pvrctx->stack[next_index];

    // copy previous matrix
    sh4matrix_copy_to(previous_state->matrix, next_state->matrix);

    // backup
    previous_state->global_alpha = pvrctx->global_alpha;
    previous_state->global_antialiasing = pvrctx->global_antialiasing;
    previous_state->global_offsetcolor_multiply = pvrctx->global_offsetcolor_multiply;
    memcpy(next_state->offsetcolor, previous_state->offsetcolor, sizeof(RGBA));

    // set previous values as default
    pvrctx->vertex_alpha = pvrctx->global_alpha;
    pvrctx->render_alpha = pvrctx->global_alpha;

    pvrctx->vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx->render_antialiasing = previous_state->global_antialiasing;

    pvrctx->vertex_offsetcolor_multiply = pvrctx->global_offsetcolor_multiply;
    pvrctx->render_offsetcolor_multiply = pvrctx->global_offsetcolor_multiply;

    pvrctx->global_offsetcolor = &next_state->offsetcolor; // change reference
    memcpy(pvrctx->vertex_offsetcolor, pvrctx->global_offsetcolor, sizeof(RGBA));
    memcpy(pvrctx->render_offsetcolor, pvrctx->global_offsetcolor, sizeof(RGBA));

    pvrctx->stack_index = next_index;
    pvrctx->current_matrix = next_state->matrix;

    // irq_restore(old_irq);

    // remember the last count of added shaders
    // previous_state->added_shaders = stacklist_count(pvrctx->shader_stack);

    return true;
}

bool pvr_context_restore(PVRContext pvrctx) {
    if (pvrctx->stack_index < 1) {
        /*if (stacklist_count(pvrctx->shader_stack) > 0) {
            logger_warn("pvr_context_restore() the current PVRContext has stacked shaders on empty stack");
        }*/
        logger_error("pvr_context_restore() the PVRContext stack was empty");
        return false;
    }

    // int old_irq = irq_disable();

    pvrctx->stack_index--;

    PVRContextState* previous_state = &pvrctx->stack[pvrctx->stack_index];

    pvrctx->current_matrix = previous_state->matrix;

    pvrctx->global_antialiasing = previous_state->global_antialiasing;
    pvrctx->vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx->render_antialiasing = previous_state->global_antialiasing;

    pvrctx->global_offsetcolor_multiply = previous_state->global_offsetcolor_multiply;
    pvrctx->vertex_offsetcolor_multiply = previous_state->global_offsetcolor_multiply;
    pvrctx->render_offsetcolor_multiply = previous_state->global_offsetcolor_multiply;

    pvrctx->global_alpha = previous_state->global_alpha;
    pvrctx->vertex_alpha = previous_state->global_alpha;
    pvrctx->render_alpha = previous_state->global_alpha;

    pvrctx->global_offsetcolor = &previous_state->offsetcolor;                     // change reference
    memcpy(pvrctx->vertex_offsetcolor, previous_state->offsetcolor, sizeof(RGBA)); // restore
    memcpy(pvrctx->render_offsetcolor, previous_state->offsetcolor, sizeof(RGBA)); // restore

    // irq_restore(old_irq);

    // remove all shaders added in the current state
    // int32_t added_shaders = stacklist_count(pvrctx->shader_stack) - previous_state.added_shaders;
    // assert(added_shaders >= 0);
    // if (added_shaders > 0) PVRContext_ShaderStackPop(pvrctx, added_shaders);

    // gl_helper_set_blend(1, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT);

    return true;
}



void pvr_context_set_vertex_alpha(PVRContext pvrctx, float alpha) {
    pvrctx->vertex_alpha = alpha;
    pvrctx->render_alpha = alpha * pvrctx->global_alpha;
}

void pvr_context_set_vertex_antialiasing(PVRContext pvrctx, PVRFlag flag) {
    pvrctx->vertex_antialiasing = flag;
    pvrctx->render_antialiasing = flag == PVRCTX_FLAG_DEFAULT ? pvrctx->global_antialiasing : flag;
}

void pvr_context_set_vertex_offsetcolor(PVRContext pvrctx, const RGBA offsetcolor) {
    memcpy(pvrctx->vertex_offsetcolor, offsetcolor, sizeof(RGBA));
    math2d_color_blend_normal(offsetcolor, *pvrctx->global_offsetcolor, pvrctx->render_offsetcolor);
}

void pvr_context_vertex_offsetcolor_multiply(PVRContext pvrctx, PVRFlag flag) {
    pvrctx->vertex_offsetcolor_multiply = flag;
    pvrctx->render_offsetcolor_multiply = flag == PVRCTX_FLAG_DEFAULT ? pvrctx->global_offsetcolor_multiply : flag;
}

void pvr_context_set_vertex_blend(PVRContext pvrctx, bool enabled, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
#ifndef _arch_dreamcast
    gl_helper_set_blend(pvrctx, enabled, src_rgb, dst_rgb, src_alpha, dst_alpha);
#else
    pvrctx->options.blend_enabled = enabled;
    pvrctx->options.blend_src_rgb = src_rgb;
    pvrctx->options.blend_dst_rgb = dst_rgb;
    pvrctx->options.blend_src_alpha = src_alpha;
    pvrctx->options.blend_dst_alpha = dst_alpha;
#endif
}

void pvr_context_set_vertex_textured_darken(PVRContext pvrctx, bool enabled) {
#ifndef _arch_dreamcast
    gl_helper.program_textured.darken_enabled = enabled;
    gl_helper.program_solid.darken_enabled = enabled;
#else
    pvrctx->options.darken = enabled;
#endif
}



void pvr_context_draw_texture(PVRContext pvrctx, Texture texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
    if (!texture->data_vram) return;
    // if (stacklist_count(pvrctx->shader_stack) > 0) pvrctx->shader_needs_flush = true;

    pvr_poly_cxt_t ctx;

    int textureformat = texture->format;
    int pow2_width = texture->width;
    int pow2_height = texture->height;
    pvr_ptr_t textureaddr = texture->data_vram;
    int filtering = pvrctx->render_antialiasing == PVRCTX_FLAG_DISABLE ? PVR_FILTER_NEAREST : PVR_FILTER_BILINEAR;

    // scale factors (if the original texture was bigger than 1024x1024)
    if (texture->scale_factor_width != 1.0f) {
        sx /= texture->scale_factor_width;
        sw /= texture->scale_factor_width;
    }
    if (texture->scale_factor_height != 1.0f) {
        sy /= texture->scale_factor_height;
        sh /= texture->scale_factor_height;
    }

    // create translucent polygon
    pvr_poly_cxt_txr(&ctx, PVR_LIST_TR_POLY, textureformat, pow2_width, pow2_height, textureaddr, filtering);

    if (pvrctx->options.blend_enabled) {
        pvr_context_helper_set_blending_to_poly_ctx(pvrctx, &ctx);
    }

    //
    // apply offsetcolor, render alpha and (if necessary) darken colors
    // the PVR renders a texel like: output = texel_color * argb + oargb;
    //
    uint32_t vertexcolor, offsetcolor;
    bool enable_offsetcolor, calc_vertexcolor = true;
    if (pvrctx->render_offsetcolor[3] < 0.0f) {
        calc_vertexcolor = true;
        offsetcolor = 0x00000000;
    } else {
        if (pvrctx->render_offsetcolor_multiply != PVRCTX_FLAG_DISABLE) {
            // for multiply use offsetcolor as vertex color
            calc_vertexcolor = false;
            offsetcolor = 0x00000000;

            if (pvrctx->options.darken) {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_offsetcolor[3] * pvrctx->render_alpha,
                    pvrctx->render_offsetcolor[0] * pvrctx->render_alpha,
                    pvrctx->render_offsetcolor[1] * pvrctx->render_alpha,
                    pvrctx->render_offsetcolor[2] * pvrctx->render_alpha
                );
            } else {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_offsetcolor[3] * pvrctx->render_alpha,
                    pvrctx->render_offsetcolor[0],
                    pvrctx->render_offsetcolor[1],
                    pvrctx->render_offsetcolor[2]
                );
            }
        } else {
            // enable oargb usage
            enable_offsetcolor = true;

            calc_vertexcolor = true;
            offsetcolor = PVR_PACK_COLOR(
                pvrctx->render_offsetcolor[3],
                pvrctx->render_offsetcolor[0],
                pvrctx->render_offsetcolor[1],
                pvrctx->render_offsetcolor[2]
            );
        }
    }

    if (calc_vertexcolor) {
        uint8_t render_u8_alpha = (uint8_t)(pvrctx->render_alpha * 255.0f);

        vertexcolor = render_u8_alpha << 24;

        if (pvrctx->options.darken)
            vertexcolor |= (render_u8_alpha << 16) | (render_u8_alpha << 8) | (render_u8_alpha << 0);
        else
            vertexcolor |= 0x00FFFFFFu;
    }

    // tranform draw coordinates
    mat_load((matrix_t*)pvrctx->current_matrix);

    // begin scene and polygon list (if necessary)
    pvr_context_prepare(pvrctx);

    if (texture->slices) {
        uint32_t cmd_flags = enable_offsetcolor ? 0x04 : 0x00;
        pvr_context_draw_sliced_texture(
            &ctx, texture->slices,
            vertexcolor, offsetcolor, cmd_flags,
            sx, sy, sw, sh, dx, dy, dw, dh
        );
        return;
    }

    // calculate UV coordinates
    float u0 = sx / texture->width;
    float v0 = sy / texture->height;
    float u1 = (sx + sw) / texture->width;
    float v1 = (sy + sh) / texture->height;

    // compile and submit polygon header
    alignas(32) pvr_poly_hdr_t hdr;
    pvr_poly_compile(&hdr, &ctx);
    if (enable_offsetcolor) hdr.cmd |= 4;
    pvr_prim(&hdr, sizeof(hdr));

    // flip texture coordinates (if necessary)
    if (math2d_is_float_sign_negative(dw)) {
        float tmp = u0;
        u0 = u1;
        u1 = tmp;

        dx += dw;
        dw = -dw;
    }
    if (math2d_is_float_sign_negative(dh)) {
        float tmp = v0;
        v0 = v1;
        v1 = tmp;

        dy += dh;
        dh = -dh;
    }

    // submit polygon
    draw_polygon(vertexcolor, offsetcolor, dx, dy, dw, dh, u0, v0, u1, v1);
}

void pvr_context_draw_solid_color(PVRContext pvrctx, const RGBA rgb_color, float dx, float dy, float dw, float dh) {
    // if (stacklist_count(pvrctx->shader_stack) > 0) pvrctx->shader_needs_flush = true;

    pvr_poly_cxt_t ctx;

    // create translucent polygon
    pvr_poly_cxt_col(&ctx, PVR_LIST_TR_POLY);

    if (pvrctx->options.blend_enabled) {
        pvr_context_helper_set_blending_to_poly_ctx(pvrctx, &ctx);
    }

    uint32_t vertexcolor, offsetcolor;
    bool enable_offsetcolor = false;

    //
    // apply offsetcolor, render alpha and (if necessary) darken colors
    // the PVR renders a texel like: output = texel_color * argb + oargb;
    // so, for solid colors can simply calculate the final color here
    //
    if (pvrctx->render_offsetcolor[3] < 0.0f) {
        offsetcolor = 0x00000000;

        if (pvrctx->options.darken) {
            vertexcolor = PVR_PACK_COLOR(
                pvrctx->render_alpha,
                rgb_color[0] * pvrctx->render_alpha,
                rgb_color[1] * pvrctx->render_alpha,
                rgb_color[2] * pvrctx->render_alpha
            );
        } else {
            vertexcolor = PVR_PACK_COLOR(
                pvrctx->render_alpha,
                rgb_color[0],
                rgb_color[1],
                rgb_color[2]
            );
        }
    } else {
        if (pvrctx->render_offsetcolor_multiply != PVRCTX_FLAG_DISABLE) {
            offsetcolor = 0x00000000;

            // for multiply use offsetcolor as vertex color
            if (pvrctx->options.darken) {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_alpha * pvrctx->render_offsetcolor[3],
                    rgb_color[0] * pvrctx->render_offsetcolor[0] * pvrctx->render_alpha,
                    rgb_color[1] * pvrctx->render_offsetcolor[1] * pvrctx->render_alpha,
                    rgb_color[2] * pvrctx->render_offsetcolor[2] * pvrctx->render_alpha
                );
            } else {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_alpha * pvrctx->render_offsetcolor[3],
                    rgb_color[0] * pvrctx->render_offsetcolor[0],
                    rgb_color[1] * pvrctx->render_offsetcolor[1],
                    rgb_color[2] * pvrctx->render_offsetcolor[2]
                );
            }
        } else {
            // enable oargb usage
            enable_offsetcolor = true;

            if (pvrctx->options.darken) {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_alpha,
                    rgb_color[0] * pvrctx->render_alpha,
                    rgb_color[1] * pvrctx->render_alpha,
                    rgb_color[2] * pvrctx->render_alpha
                );
            } else {
                vertexcolor = PVR_PACK_COLOR(
                    pvrctx->render_alpha,
                    rgb_color[0],
                    rgb_color[1],
                    rgb_color[2]
                );
            }

            offsetcolor = PVR_PACK_COLOR(
                pvrctx->render_offsetcolor[3],
                pvrctx->render_offsetcolor[0],
                pvrctx->render_offsetcolor[1],
                pvrctx->render_offsetcolor[2]
            );
        }
    }

    // begin scene and polygon list (if necessary)
    pvr_context_prepare(pvrctx);

    // tranform draw coordinates
    mat_load((matrix_t*)pvrctx->current_matrix);

    // correct draw coordinates (if necessary)
    if (math2d_is_float_sign_negative(dw)) {
        dx += dw;
        dw = -dw;
    }
    if (math2d_is_float_sign_negative(dh)) {
        dy += dh;
        dh = -dh;
    }

    // z-index
    float dz_stub = 0.0f;
    float dz = pvr_context_current_z_index;
    pvr_context_current_z_index = dz + PVRCTX_Z_INDEX_STEPS;

    // right and bottom corners
    float dr = dx + dw;
    float db = dy + dh;

    // compile polygon header
    alignas(32) pvr_poly_hdr_t hdr;
    pvr_poly_compile(&hdr, &ctx);
    if (enable_offsetcolor) hdr.cmd |= 4;
    pvr_prim(&hdr, sizeof(hdr));

    // common parameters
    alignas(32) pvr_vertex_t vert;
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = vertexcolor;
    vert.oargb = offsetcolor;
    vert.u = 0.0f;
    vert.v = 0.0f;
    vert.z = dz;

    vert.x = dx;
    vert.y = dy;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dr;
    vert.y = dy;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dx;
    vert.y = db;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dr;
    vert.y = db;
    vert.flags = PVR_CMD_VERTEX_EOL;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));
}

/*void pvr_context_draw_framebuffer(PVRContext pvrctx, PSFramebuffer psframebuffer, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
    pvr_context->shader_needs_flush = true;
    gl_helper_draw_framebuffer(pvrctx, psframebuffer, sx, sy, sw, sh, dx, dy, dw, dh);
}*/



bool pvr_context_is_offscreen(PVRContext pvrctx) {
    (void)pvrctx;
    return vid_check_cable() == CT_NONE;
}

void pvr_context_set_global_alpha(PVRContext pvrctx, float alpha) {
    PVRContextState* last_state = PREVIOUS_STATE(pvrctx);

    if (last_state) alpha *= last_state->global_alpha;

    pvrctx->global_alpha = alpha;
    pvr_context_set_vertex_alpha(pvrctx, pvrctx->vertex_alpha);
}

void pvr_context_set_global_antialiasing(PVRContext pvrctx, PVRFlag flag) {
    PVRContextState* last_state = PREVIOUS_STATE(pvrctx);

    if (last_state && flag == PVRCTX_FLAG_DEFAULT) flag = last_state->global_antialiasing;

    pvrctx->global_antialiasing = flag;
    pvr_context_set_vertex_antialiasing(pvrctx, pvrctx->vertex_antialiasing);
}

void pvr_context_set_global_offsetcolor(PVRContext pvrctx, const RGBA offsetcolor) {
    PVRContextState* last_state = PREVIOUS_STATE(pvrctx);

    if (last_state) {
        math2d_color_blend_normal(offsetcolor, last_state->offsetcolor, *pvrctx->global_offsetcolor);
    } else {
        memcpy(*pvrctx->global_offsetcolor, offsetcolor, sizeof(RGBA));
    }

    pvr_context_set_vertex_offsetcolor(pvrctx, pvrctx->vertex_offsetcolor);
}

void pvr_context_global_offsetcolor_multiply(PVRContext pvrctx, PVRFlag flag) {
    PVRContextState* last_state = PREVIOUS_STATE(pvrctx);

    if (last_state && flag == PVRCTX_FLAG_DEFAULT) flag = last_state->global_offsetcolor_multiply;

    pvrctx->global_offsetcolor_multiply = flag;
    pvr_context_vertex_offsetcolor_multiply(pvrctx, pvrctx->vertex_offsetcolor_multiply);
}


#ifndef _arch_dreamcast
bool pvr_context_add_shader(PVRContext pvrctx, PSShader psshader) {
    return PVRContext_ShaderStackPush(pvrctx, psshader);
}

void pvr_context_set_framebuffer(PVRContext pvrctx, PSFramebuffer psframebuffer) {
    pvrctx->target_framebuffer = psframebuffer;
    if (stacklist_count(pvrctx->shader_stack) < 1) PVRContext_UseDefaultFramebuffer(pvrctx);
}
#endif

bool pvr_context_is_widescreen() {
    return !math2d_floats_are_near_equal(640.0f / 480.0f, pvr_context.screen_width / (float)pvr_context.screen_height);
}


void pvr_context_helper_set_blending_to_poly_ctx(PVRContext pvrctx, pvr_poly_cxt_t* ctx) {
    // Note: blend_src_alpha and blend_dst_alpha is not supported
    int blend_src_rgb = get_pvr_blending(pvrctx->options.blend_src_rgb);
    int blend_dst_rgb = get_pvr_blending(pvrctx->options.blend_dst_rgb);

    if (blend_src_rgb == -1 || blend_dst_rgb == -1) {
        return;
    }

    ctx->gen.alpha = PVR_ALPHA_ENABLE;
    ctx->txr.alpha = PVR_TXRALPHA_ENABLE;

    if (blend_src_rgb != -1) {
        ctx->blend.src_enable = PVR_BLEND_ENABLE;
        ctx->blend.src = blend_src_rgb;
    }
    if (blend_dst_rgb != -1) {
        ctx->blend.dst_enable = PVR_BLEND_ENABLE;
        ctx->blend.dst = blend_dst_rgb;
    }
}


static int flashrom_get_region2() {
    int start, size;
    char region[6] = {0};

    if (flashrom_info(FLASHROM_PT_SYSTEM, &start, &size) == 0) {
        int ret = flashrom_read(start, region, 5);

        // for some reason flashrom_read() function returns 0, maybe the docs are wrong
        if (ret == 0 || ret == 5) {
            if (strcmp(region, "00043") == 0)
                return FLASHROM_REGION_JAPAN;
            else if (strcmp(region, "00143") == 0)
                return FLASHROM_REGION_US;
            else if (strcmp(region, "00243") == 0)
                return FLASHROM_REGION_EUROPE;
        }
    }

    return FLASHROM_REGION_UNKNOWN;
}

static int get_pvr_blending(Blend blend) {
    switch (blend) {
        case BLEND_ZERO:
            return PVR_BLEND_ZERO;
        case BLEND_ONE:
            return PVR_BLEND_ONE;
        case BLEND_DST_COLOR:
            return PVR_BLEND_DESTCOLOR;
        case BLEND_ONE_MINUS_DST_COLOR:
            return PVR_BLEND_INVDESTCOLOR;
        case BLEND_SRC_ALPHA:
            return PVR_BLEND_SRCALPHA;
        case BLEND_ONE_MINUS_SRC_ALPHA:
            return PVR_BLEND_INVSRCALPHA;
        case BLEND_DST_ALPHA:
            return PVR_BLEND_DESTALPHA;
        case BLEND_ONE_MINUS_DST_ALPHA:
            return PVR_BLEND_INVDESTALPHA;
        default:
            return -1;
    }
}


static inline void draw_polygon(uint32_t vertexcolor, uint32_t offsetcolor, float dx, float dy, float dw, float dh, float u0, float v0, float u1, float v1) {
    alignas(32) pvr_vertex_t vert;
    float dz_stub = 0.0f;

    // z-index
    float dz = pvr_context_current_z_index;
    pvr_context_current_z_index = dz + PVRCTX_Z_INDEX_STEPS;

    // right and bottom corners
    float dr = dx + dw;
    float db = dy + dh;

    // common parameters
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = vertexcolor;
    vert.oargb = offsetcolor;
    vert.z = dz;

    vert.x = dx;
    vert.y = dy;
    vert.u = u0;
    vert.v = v0;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dr;
    vert.y = dy;
    vert.u = u1;
    vert.v = v0;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dx;
    vert.y = db;
    vert.u = u0;
    vert.v = v1;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));

    vert.x = dr;
    vert.y = db;
    vert.u = u1;
    vert.v = v1;
    vert.flags = PVR_CMD_VERTEX_EOL;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(vert));
}

void pvr_context_draw_sliced_texture(pvr_poly_cxt_t* ctx, KDTSliceTable* slices, uint32_t vtxcolor, uint32_t offcolor, uint32_t fcmd, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
    //
    // before continue check if the supplied values
    // are finite to avoid deadlocks
    //
    if (!math2d_is_float_finite(sx)) return;
    if (!math2d_is_float_finite(sy)) return;
    if (!math2d_is_float_finite(sw)) return;
    if (!math2d_is_float_finite(sh)) return;
    if (!math2d_is_float_finite(dx)) return;
    if (!math2d_is_float_finite(dy)) return;
    if (!math2d_is_float_finite(dw)) return;
    if (!math2d_is_float_finite(dh)) return;

    // the draw dimmensions must be positive, remember the number sign
    bool negative_dw = math2d_is_float_sign_negative(dw);
    bool negative_dh = math2d_is_float_sign_negative(dh);
    if (negative_dw) dw = -dw;
    if (negative_dh) dh = -dh;

    // float source_right = sx + sw, source_bottom = sy + sh;
    float ratio_dw = dw / sw, ratio_dh = dh / sh;
    float source_y = sy, source_height = sh;
    float draw_y = dy, draw_height = dh;

    for (uint8_t idx_y = 0; idx_y < KDT_MAX_SLICES; idx_y++) {
        float last_block_height = 0.0f;
        float last_block_dh = 0.0f;

        float source_x = sx, source_width = sw;
        float draw_x = dx, draw_width = dw;

        for (uint8_t idx_x = 0; idx_x < KDT_MAX_SLICES; idx_x++) {
            KDTBlock* block = &(*slices)[idx_y][idx_x];
            if (!block->vram_ptr) break;

            float block_x = block->x;
            float block_y = block->y;
            float block_width = block->width;
            float block_right = block_x + block_width;
            float block_height = block->height;
            float block_bottom = block_y + block_height;

            if (source_x < block_x || math2d_floats_are_greater_or_near_equal(source_x, block_right)) continue;
            // if (math2d_floats_are_less_or_near_equal(source_right, block_right) && source_right > block_right) continue;

            if (source_y < block_y || math2d_floats_are_greater_or_near_equal(source_y, block_bottom)) continue;
            // if (math2d_floats_are_less_or_near_equal(source_bottom, block_bottom) && source_bottom > block_bottom) continue;

            // calc top-left texture coordinates
            float u0 = source_x - block_x;
            float v0 = source_y - block_y;

            // calc source-in-block dimmensions
            float offset_width = math2d_min_float(source_width, block_width - u0);
            float offset_height = math2d_min_float(source_height, block_height - v0);

            // calc bottom-right texture coordinates
            float u1 = u0 + offset_width;
            float v1 = v0 + offset_height;

            // calc draw coordinates
            float block_dw = math2d_min_float(offset_width * ratio_dw, draw_width);
            float block_dh = math2d_min_float(offset_height * ratio_dh, draw_height);

            // note: the following fields can stop working on future versions of KallistiOS
            ctx->txr.width = block->width;
            ctx->txr.height = block->height;
            ctx->txr.base = block->vram_ptr;

            u0 /= (float)block->width;
            u1 /= (float)block->width;
            v0 /= (float)block->height;
            v1 /= (float)block->height;

            // flip texture coordinates (if necessary)
            float block_dx = draw_x;
            if (negative_dw) {
                float tmp = u0;
                u0 = u1;
                u1 = tmp;
                block_dx -= block_dw;
            }
            float block_dy = draw_y;
            if (negative_dh) {
                float tmp = v0;
                v0 = v1;
                v1 = tmp;
                block_dy -= block_dh;
            }

            // compile and submit polygon header
            alignas(32) pvr_poly_hdr_t hdr;
            pvr_poly_compile(&hdr, ctx);
            hdr.cmd |= fcmd;
            pvr_prim(&hdr, sizeof(hdr));

            draw_polygon(vtxcolor, offcolor, block_dx, block_dy, block_dw, block_dh, u0, v0, u1, v1);

            source_x += offset_width;
            source_width -= offset_width;
            draw_width -= block_dw;

            if (negative_dw)
                draw_x -= block_dw;
            else
                draw_x += block_dw;

            last_block_height = offset_height;
            last_block_dh = block_dh;

            // check if eof was reached
            if (math2d_floats_are_less_or_near_equal(source_width, 0.0f)) {
                break;
            }
            if (math2d_floats_are_less_or_near_equal(draw_width, 0.0f)) {
                break;
            }
        }

        if (last_block_height == 0.0f) {
            continue;
        }

        source_y += last_block_height;
        source_height -= last_block_height;
        draw_height -= last_block_dh;

        if (negative_dh)
            draw_y -= last_block_dh;
        else
            draw_y += last_block_dh;

        // check if eof was reached
        if (math2d_floats_are_less_or_near_equal(source_height, 0.0f)) {
            break;
        }
        if (math2d_floats_are_less_or_near_equal(draw_height, 0.0f)) {
            break;
        }
    }
}
