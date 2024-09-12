#ifndef _pvrcontext_h
#define _pvrcontext_h

#include <stdbool.h>
#include <stdint.h>

#include "pvrcontext_types.h"
#include <stdalign.h>

#include "color2d.h"
#include "modifier.h"
#include "sh4matrix.h"
#include "texture.h"
#include "vertexprops_types.h"


#define PVRCTX_STATE_STACK_LENGTH 16
#define PVRCTX_STATE_VERTEX_BUFFER_SIZE (512 * 1024) // 512KiB
#define PVRCTX_Z_INDEX_STEPS 1.0f

typedef struct {
    alignas(32) SH4Matrix matrix;
    float global_alpha;
    RGBA offsetcolor;
    PVRFlag global_antialiasing;
    PVRFlag global_offsetcolor_multiply;
    // int32_t  added_shaders;
} PVRContextState;


struct PVRContext_s {
    PVRContextState stack[PVRCTX_STATE_STACK_LENGTH];
    int32_t resolution_changes;
    float last_elapsed;
    uint64_t last_timestamp;
    bool last_was_threshold;
    // int32_t frame_rendered;
    // char* native_window_title;

    // PSFramebuffer shader_framebuffer_front;
    // PSFramebuffer shader_framebuffer_back;
    // PSFramebuffer target_framebuffer;
    // bool shader_needs_flush;
    // StackList shader_stack;
    // uint32_t shader_last_resolution_changes;
    // int32_t screen_stride;


    uint16_t stack_index;

    float vertex_alpha;
    RGBA vertex_offsetcolor;

    float global_alpha;
    RGBA* global_offsetcolor;

    PVRFlag global_antialiasing;
    PVRFlag vertex_antialiasing;

    PVRFlag global_offsetcolor_multiply;
    PVRFlag vertex_offsetcolor_multiply;

    float render_alpha;
    RGBA render_offsetcolor;
    PVRFlag render_offsetcolor_multiply;
    PVRFlag render_antialiasing;

    float* current_matrix;
    PVRContextState* current_state;

    int32_t screen_width;
    int32_t screen_height;

    struct {
        bool blend_enabled;
        Blend blend_src_rgb;
        Blend blend_dst_rgb;
        Blend blend_src_alpha;
        Blend blend_dst_alpha;
        bool darken;
    } options;
};

typedef struct PVRContext_s* PVRContext;


void pvr_context_init();
float pvrctx_wait_ready();
void pvr_context_reset(PVRContext pvrctx);
void pvr_context_apply_modifier(PVRContext pvrctx, Modifier* modifier);
void pvr_context_apply_modifier2(PVRContext pvrctx, Modifier* modifier, float draw_x, float draw_y, float draw_width, float draw_height);
void pvr_context_clear_screen(PVRContext pvrctx, const RGBA rgba_color);
void pvr_context_flush(PVRContext pvrctx);
bool pvr_context_save(PVRContext pvrctx);
bool pvr_context_restore(PVRContext pvrctx);


void pvr_context_set_vertex_alpha(PVRContext pvrctx, float alpha);
void pvr_context_set_vertex_antialiasing(PVRContext pvrctx, PVRFlag flag);
void pvr_context_set_vertex_offsetcolor(PVRContext pvrctx, const RGBA offsetcolor);
void pvr_context_vertex_offsetcolor_multiply(PVRContext pvrctx, PVRFlag flag);
void pvr_context_set_vertex_blend(PVRContext pvrctx, bool enabled, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);
void pvr_context_set_vertex_textured_darken(PVRContext pvrctx, bool enabled);


void pvr_context_draw_texture(PVRContext pvrctx, Texture texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void pvr_context_draw_solid_color(PVRContext pvrctx, const RGBA rgb_color, float dx, float dy, float dw, float dh);
// void pvr_context_draw_framebuffer(PVRContext pvrctx, PSFramebuffer psframebuffer, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);


bool pvr_context_is_offscreen(PVRContext pvrctx);
void pvr_context_set_global_alpha(PVRContext pvrctx, float alpha);
void pvr_context_set_global_antialiasing(PVRContext pvrctx, PVRFlag flag);
void pvr_context_set_global_offsetcolor(PVRContext pvrctx, const RGBA offsetcolor);
void pvr_context_global_offsetcolor_multiply(PVRContext pvrctx, PVRFlag flag);


// bool pvr_context_add_shader(PVRContext pvrctx, PSShader psshader);
// void pvr_context_set_framebuffer(PVRContext pvrctx, PSFramebuffer psframebuffer);


bool pvr_context_is_widescreen();


void pvr_context_helper_clear_modifier(Modifier* modifier);
void pvr_context_helper_invalidate_modifier(Modifier* modifier);
void pvr_context_helper_clear_offsetcolor(RGBA offsetcolor);
void pvr_context_helper_set_modifier_property(Modifier* modifier, int32_t property, float value);
void pvr_context_helper_copy_modifier(const Modifier* modifier_source, Modifier* modifier_dest);
void pvr_context_helper_set_blending_to_poly_ctx(PVRContext pvrctx, pvr_poly_cxt_t* ctx);

void pvr_context_draw_sliced_texture(pvr_poly_cxt_t* ctx, KDTSliceTable* slices, uint32_t vtxcolor, uint32_t offcolor, uint32_t fcmd, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

#endif
