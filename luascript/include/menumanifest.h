#ifndef __menumanifest_h_
#define __menumanifest_h_

#include <stdbool.h>
#include <stdint.h>

#include "vertexprops.h"

typedef struct {
    float gap;
    float x;
    float y;
    float dimmen;
} MenuManifestPlacement;

typedef struct {
    MenuManifestPlacement placement;
    bool has_font_color;
    uint32_t font_color;
    char* text;
    char* model;
    float texture_scale;
    char* name;
    char* anim_selected;
    char* anim_choosen;
    char* anim_discarded;
    char* anim_idle;
    char* anim_rollback;
    char* anim_in;
    char* anim_out;
    bool hidden;
    char* description;
} MenuManifestItem;

typedef struct {
    uint32_t font_color;
    char* atlas;
    char* animlist;
    char* font;
    float font_size;
    char* font_glyph_suffix;
    bool font_color_by_difference;
    uint32_t font_border_color;
    char* anim_discarded;
    char* anim_idle;
    char* anim_rollback;
    char* anim_selected;
    char* anim_choosen;
    char* anim_in;
    char* anim_out;
    float anim_transition_in_delay;
    float anim_transition_out_delay;
    bool is_vertical;
    Align items_align;
    float items_gap;
    bool is_sparse;
    int32_t static_index;
    bool is_per_page;
    float items_dimmen;
    float font_border_size;
    float texture_scale;
    bool enable_horizontal_text_correction;
    char* suffix_selected;
    char* suffix_choosen;
    char* suffix_discarded;
    char* suffix_idle;
    char* suffix_rollback;
    char* suffix_in;
    char* suffix_out;
} MenuManifestParameters;

typedef struct _MenuManifest_t {
    MenuManifestParameters parameters;
    MenuManifestItem* items;
    int32_t items_size;
} MenuManifest_t;

typedef MenuManifest_t* MenuManifest;

MenuManifest menumanifest_init(const char* src);
void menumanifest_destroy(MenuManifest* menumanifest);

#endif
