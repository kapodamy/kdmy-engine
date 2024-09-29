#include "glyphrenderer.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"


#define GLYPHSRENDERER_PALETTE_INDEX 0

typedef union {
    pvr_poly_cxt_t ctx;
    pvr_poly_hdr_t hdr;
} pvr_ctxhdr_t;

static PVRContext glyphrenderer_pvrctx = NULL;
static bool glyphrenderer_by_add = NULL;
static bool glyphrenderer_is_gryscl = NULL;
static Texture glyphrenderer_tex0 = NULL;
static Texture glyphrenderer_tex1 = NULL;

static alignas(32) pvr_ctxhdr_t glyphrenderer_ctxhdr_tex0, glyphrenderer_ctxhdr_tex1;
static uint32_t glyphrenderer_tint_argb, glyphrenderer_tint_oargb;
static uint32_t glyphrenderer_outline_argb, glyphrenderer_outline_oargb;
static uint32_t glyphrenderer_fcmd;


static void submit_grayscale_palette() {
    pvr_set_pal_format(PVR_PAL_ARGB4444);

    /*for (uint32_t i = 0, color = 0x0FFF; i < 16; i++) {
        pvr_set_pal_entry(i + GLYPHSRENDERER_PALETTE_INDEX, color);
        color += 0x1000;
    }
    return;*/

    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x00, 0x00000FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x01, 0x00001FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x02, 0x00002FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x03, 0x00003FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x04, 0x00004FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x05, 0x00005FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x06, 0x00006FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x07, 0x00007FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x08, 0x00008FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x09, 0x00009FFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0A, 0x0000AFFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0B, 0x0000BFFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0C, 0x0000CFFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0D, 0x0000DFFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0E, 0x0000EFFF);
    pvr_set_pal_entry(GLYPHSRENDERER_PALETTE_INDEX + 0x0F, 0x0000FFFF);
}

static void compile_translucent_polygon_header(Texture tex, pvr_ctxhdr_t* dst) {
    pvr_poly_cxt_t ctx;

    int filtering;
    if (glyphrenderer_pvrctx->render_antialiasing == PVRCTX_FLAG_DISABLE)
        filtering = PVR_FILTER_NEAREST;
    else
        filtering = PVR_FILTER_BILINEAR;

    int fmt = tex->format;
    if (glyphrenderer_is_gryscl) {
        // pick palette
        fmt |= PVR_TXRFMT_4BPP_PAL(GLYPHSRENDERER_PALETTE_INDEX);
    }

    pvr_poly_cxt_txr(&ctx, PVR_LIST_TR_POLY, fmt, tex->width, tex->height, tex->data_vram, filtering);

    if (glyphrenderer_pvrctx->options.blend_enabled) {
        pvr_context_helper_set_blending_to_poly_ctx(glyphrenderer_pvrctx, &ctx);
    }

    if (tex->slices)
        dst->ctx = ctx;
    else
        pvr_poly_compile(&dst->hdr, &ctx);
}

static void calculate_color(RGBA color, bool by_add, bool is_tint) {
    const PVRContext pvrctx = glyphrenderer_pvrctx;
    const float render_alpha = pvrctx->render_alpha;
    const float* render_offsetcolor = pvrctx->render_offsetcolor;
    const bool render_offsetcolor_multiply = pvrctx->render_offsetcolor_multiply != PVRCTX_FLAG_DISABLE;
    bool no_render_offset_color = render_offsetcolor[3] < 0.0f;

    // calc offsetcolor, render alpha and (if necessary) darken colors
    uint32_t vertexcolor, offsetcolor = 0x00000000;
    bool enable_offsetcolor;

    //
    // Note: the "color[3]" (alpha component) must be ignored, is already applied in the "render_alpha"
    //

    if (!by_add && no_render_offset_color) {
        enable_offsetcolor = false;
        offsetcolor = 0x00000000;
        vertexcolor = PVR_PACK_COLOR(
            render_alpha, color[0], color[1], color[2]
        );
    } else if (by_add) {
        enable_offsetcolor = true;
        if (no_render_offset_color) {
            offsetcolor = PVR_PACK_COLOR(
                0.0f,
                color[0],
                color[1],
                color[2]
            );
            vertexcolor = PVR_PACK_COLOR(
                render_alpha,
                1.0f,
                1.0f,
                1.0f
            );
        } else {
            if (render_offsetcolor_multiply) {
                offsetcolor = PVR_PACK_COLOR(
                    0.0f,
                    color[0],
                    color[1],
                    color[2]
                );
                vertexcolor = PVR_PACK_COLOR(
                    render_offsetcolor[3] * render_alpha,
                    render_offsetcolor[0],
                    render_offsetcolor[1],
                    render_offsetcolor[2]
                );
            } else {
                offsetcolor = PVR_PACK_COLOR(
                    render_offsetcolor[3],
                    color[0] + render_offsetcolor[0],
                    color[1] + render_offsetcolor[1],
                    color[2] + render_offsetcolor[2]
                );
                vertexcolor = PVR_PACK_COLOR(render_alpha, 1.0f, 1.0f, 1.0f);
            }
        }
    } else {
        if (render_offsetcolor_multiply) {
            enable_offsetcolor = false;
            offsetcolor = 0x00000000;
            vertexcolor = PVR_PACK_COLOR(
                render_offsetcolor[3] * render_alpha,
                color[0] * render_offsetcolor[0],
                color[1] * render_offsetcolor[1],
                color[2] * render_offsetcolor[2]
            );
        } else {
            enable_offsetcolor = true;
            offsetcolor = PVR_PACK_COLOR(
                render_offsetcolor[3], render_offsetcolor[0], render_offsetcolor[1], render_offsetcolor[2]
            );
            vertexcolor = PVR_PACK_COLOR(
                render_alpha, color[0], color[1], color[2]
            );
        }
    }

    // set common parameters
    glyphrenderer_fcmd = enable_offsetcolor ? 0x04 : 0x00;
    if (is_tint) {
        glyphrenderer_tint_argb = vertexcolor;
        glyphrenderer_tint_oargb = offsetcolor;
    } else {
        glyphrenderer_outline_argb = vertexcolor;
        glyphrenderer_outline_oargb = offsetcolor;
    }
}


int32_t glyphrenderer_prepare(PVRContext pvrctx, int32_t glyph_count, bool has_outlines, RGBA tint, RGBA outline, bool by_add, bool is_gryscl, Texture tex0, Texture tex1) {
    // if (stacklist_count(pvrctx->shader_stack) > 0) pvrctx->shader_needs_flush = true;

    if (is_gryscl) {
        submit_grayscale_palette();
    }

    glyphrenderer_pvrctx = pvrctx;
    glyphrenderer_by_add = by_add;
    glyphrenderer_is_gryscl = is_gryscl;
    glyphrenderer_tex0 = tex0;
    glyphrenderer_tex1 = tex1;

    // create translucent polygon for each texture and compile the vertex list header
    if (tex0) compile_translucent_polygon_header(tex0, &glyphrenderer_ctxhdr_tex0);
    if (tex1) compile_translucent_polygon_header(tex1, &glyphrenderer_ctxhdr_tex1);

    //
    // apply offsetcolor, render alpha and (if necessary) darken colors
    // the PVR renders a texel like: output = texel_color * argb + oargb;
    //
    calculate_color(tint, by_add, true);
    if (has_outlines) calculate_color(outline, by_add, false);

    // tranform draw coordinates
    mat_load((matrix_t*)pvrctx->current_matrix);

    // begin scene and polygon list (if necessary)
    pvr_context_prepare(pvrctx);

    // calculate how many glyphs can be rendered
    pvr_stats_t stat;
    if (pvr_get_stats(&stat) != 0) {
        // this never should happen
        return glyph_count;
    }

    // each polygon needs ~160bytes and the buffer is always used to render two frames
    size_t available_space = (PVRCTX_STATE_VERTEX_BUFFER_SIZE - stat.vtx_buffer_used) / 2;
    available_space /= (sizeof(pvr_vertex_t) * 4) + sizeof(pvr_poly_hdr_t);
    if (has_outlines) available_space /= 2;

    return (int32_t)available_space;
}

void glyphrenderer_draw_glyph(bool is_tex1, bool is_outline, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
    Texture texture = is_tex1 ? glyphrenderer_tex1 : glyphrenderer_tex0;
    if (!texture->data_vram) return;

    // scale factors (if the original texture was bigger than 1024x1024)
    if (texture->scale_factor_width != 1.0f) {
        sx /= texture->scale_factor_width;
        sw /= texture->scale_factor_width;
    }
    if (texture->scale_factor_height != 1.0f) {
        sy /= texture->scale_factor_height;
        sh /= texture->scale_factor_height;
    }

    if (texture->slices) {
        pvr_poly_cxt_t* ctx = is_tex1 ? &glyphrenderer_ctxhdr_tex1.ctx : &glyphrenderer_ctxhdr_tex0.ctx;
        uint32_t argb = is_outline ? glyphrenderer_outline_argb : glyphrenderer_tint_argb;
        uint32_t oargb = is_outline ? glyphrenderer_outline_oargb : glyphrenderer_tint_oargb;

        pvr_context_draw_sliced_texture(
            ctx, texture->slices,
            argb, oargb, glyphrenderer_fcmd,
            sx, sy, sw, sh, dx, dy, dw, dh
        );
        return;
    }

    // dz stub, not required for 2D rendering
    float dz_stub = 0.0f;

    // z-index
    float dz = pvr_context_current_z_index;
    pvr_context_current_z_index = dz + PVRCTX_Z_INDEX_STEPS;

    // calculate UV coordinates
    float u0 = sx / texture->width;
    float v0 = sy / texture->height;
    float u1 = (sx + sw) / texture->width;
    float v1 = (sy + sh) / texture->height;

    // right and bottom corners
    float dr = dx + dw;
    float db = dy + dh;

    // submit polygon header
    pvr_poly_hdr_t* hdr = is_tex1 ? &glyphrenderer_ctxhdr_tex1.hdr : &glyphrenderer_ctxhdr_tex0.hdr;
    pvr_prim(hdr, sizeof(pvr_poly_hdr_t));

    // prepare vertex
    alignas(32) pvr_vertex_t vert;
    vert.flags = PVR_CMD_VERTEX;
    vert.argb = is_outline ? glyphrenderer_outline_argb : glyphrenderer_tint_argb;
    vert.oargb = is_outline ? glyphrenderer_outline_oargb : glyphrenderer_tint_oargb;
    vert.z = dz;

    vert.x = dx;
    vert.y = dy;
    vert.u = u0;
    vert.v = v0;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(pvr_vertex_t));

    vert.x = dr;
    vert.y = dy;
    vert.u = u1;
    vert.v = v0;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(pvr_vertex_t));

    vert.x = dx;
    vert.y = db;
    vert.u = u0;
    vert.v = v1;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(pvr_vertex_t));

    vert.x = dr;
    vert.y = db;
    vert.u = u1;
    vert.v = v1;
    vert.flags = PVR_CMD_VERTEX_EOL;
    SH4MATRIX_POINT_TRANSFORM(vert.x, vert.y, dz_stub);
    pvr_prim(&vert, sizeof(pvr_vertex_t));
}
