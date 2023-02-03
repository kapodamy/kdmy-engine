#include "commons.h"
#include "menumanifest.h"

static MenuManifestItem test_mm_items[] = {
    {
        .placement = {.gap = 10.0f, .x = 20.0f, .y = 30.0f, .dimmen = 40.0f},
        .has_font_color = true,
        .font_color = 0xBC614E,
        .text = "text",
        .modelholder = "model.xml",
        .texture_scale = 10.0f,
        .name = "name",
        .anim_selected = "anim-selected",
        .anim_choosen = "anim-choosen",
        .anim_discarded = "anim-discarded",
        .anim_idle = "anim-idle",
        .anim_rollback = "anim-rollback",
        .anim_in = "anim-in",
        .anim_out = "anim-out",
        .gap = 20.0f,
        .hidden = false,
    },
    {
        .placement = {.gap = 11.1f, .x = 22.2f, .y = 33.3f, .dimmen = 44.4f},
        .has_font_color = false,
        .font_color = 0xBC614E,
        .text = NULL,
        .modelholder = "stub model.xml",
        .texture_scale = 100.0f,
        .name = "stub name (.text is null)",
        .anim_selected = "selected",
        .anim_choosen = "choosen",
        .anim_discarded = "discarded",
        .anim_idle = "idle",
        .anim_rollback = "rollback",
        .anim_in = "in",
        .anim_out = "out",
        .gap = 200.0f,
        .hidden = true,
    }};
static MenuManifest_t test_mm = {
    .parameters = {
        .font_color = 0x3039,
        .atlas = "atlas.xml",
        .animlist = "animlist.xml",
        .font = "font.ttf",
        .font_size = 20.0f,
        .font_glyph_suffix = "bold",
        .font_color_by_difference = true,
        .font_border_color = 0x1A85,
        .anim_discarded = "discarded",
        .anim_idle = "idle",
        .anim_rollback = "rollback",
        .anim_selected = "selected",
        .anim_choosen = "choosen",
        .anim_in = "in",
        .anim_out = "out",
        .anim_transition_in_delay = 123.0f,
        .anim_transition_out_delay = 456.0f,
        .is_vertical = true,
        .items_align = ALIGN_END,
        .items_gap = 456.0f,
        .is_sparse = false,
        .static_index = 10,
        .is_per_page = 20,
        .items_dimmen = 30,
        .font_border_size = 40,
        .texture_scale = 50.0f,
        .enable_horizontal_text_correction = true,
        .suffix_selected = "selected",
        .suffix_choosen = "choosen",
        .suffix_discarded = "discarded",
        .suffix_idle = "idle",
        .suffix_rollback = "rollback",
        .suffix_in = "in",
        .suffix_out = "out"},
    .items_size = sizeof(test_mm_items) / sizeof(MenuManifestItem),
    .items = test_mm_items};


MenuManifest menumanifest_init(const char* src) {
    print_stub("menumanifest_init", "src=%s", src);
    return &test_mm;
}
void menumanifest_destroy(MenuManifest* menumanifest) {
    print_stub("menumanifest_destroy", "menumanifest=%p", menumanifest);
}
