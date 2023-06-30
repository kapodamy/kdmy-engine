"use strict";


async function menu_init(menumanifest, x, y, z, width, height) {
    const params = menumanifest.parameters;
    console.assert(menumanifest.items.length == menumanifest.items_size);

    let items = new Array(menumanifest.items_size);
    let modelholder = await modelholder_init2(params.font_color, params.atlas, params.animlist);

    let fontholder = null;
    if (params.font) {
        fontholder = await fontholder_init(params.font, params.font_size, params.font_glyph_suffix);
        if (!fontholder) throw new Error("Missing or invalid font: " + params.font);
        if (fontholder.font_from_atlas) {
            fontglyph_enable_color_by_difference(fontholder.font, params.font_color_by_difference);
        }
    }

    let border = [0, 0, 0, 0];
    math2d_color_bytes_to_floats(params.font_border_color, 1, border);

    let anim_discarded = null;
    let anim_idle = null;
    let anim_rollback = null;
    let anim_selected = null;
    let anim_choosen = null;
    let anim_in = null;
    let anim_out = null;

    if (params.anim_discarded)
        anim_discarded = modelholder_create_animsprite(modelholder, params.anim_discarded, false, false);
    if (params.anim_idle)
        anim_idle = modelholder_create_animsprite(modelholder, params.anim_idle, false, false);
    if (params.anim_rollback)
        anim_rollback = modelholder_create_animsprite(modelholder, params.anim_rollback, false, false);
    if (params.anim_selected)
        anim_selected = modelholder_create_animsprite(modelholder, params.anim_selected, false, false);
    if (params.anim_choosen)
        anim_choosen = modelholder_create_animsprite(modelholder, params.anim_choosen, false, false);
    if (params.anim_in)
        anim_in = modelholder_create_animsprite(modelholder, params.anim_in, false, false);
    if (params.anim_out)
        anim_out = modelholder_create_animsprite(modelholder, params.anim_out, false, false);

    for (let i = 0; i < menumanifest.items_size; i++) {
        let src_item = menumanifest.items[i];

        items[i] = {
            name: null,

            anim_self: null,

            anim_choosen: null,
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,
            anim_selected: null,
            anim_in: null,
            anim_out: null,

            is_text: 0,
            vertex: null,

            center_x: 0,
            center_y: 0,

            cell_dimmen: NaN,
            cell_gap: menumanifest.items[i].placement.gap,
            placement_x: menumanifest.items[i].placement.x,
            placement_y: menumanifest.items[i].placement.y,

            rollback_active: 0,
            render_distance: 0,
            has_scale: 0,

            hidden: 0
        };

        menu_internal_build_item(items[i], src_item, params, modelholder, fontholder, border);

        if (!items[i].anim_choosen && anim_choosen) items[i].anim_choosen = animsprite_clone(anim_choosen);
        if (!items[i].anim_discarded && anim_discarded) items[i].anim_discarded = animsprite_clone(anim_discarded);
        if (!items[i].anim_idle && anim_idle) items[i].anim_idle = animsprite_clone(anim_idle);
        if (!items[i].anim_rollback && anim_rollback) items[i].anim_rollback = animsprite_clone(anim_rollback);
        if (!items[i].anim_selected && anim_selected) items[i].anim_selected = animsprite_clone(anim_selected);
        if (!items[i].anim_in && anim_in) items[i].anim_in = animsprite_clone(anim_in);
        if (!items[i].anim_out && anim_out) items[i].anim_out = animsprite_clone(anim_out);
    }

    if (anim_discarded) animsprite_destroy(anim_discarded);
    if (anim_idle) animsprite_destroy(anim_idle);
    if (anim_rollback) animsprite_destroy(anim_rollback);
    if (anim_selected) animsprite_destroy(anim_selected);
    if (anim_choosen) animsprite_destroy(anim_choosen);
    if (anim_in) animsprite_destroy(anim_in);
    if (anim_out) animsprite_destroy(anim_out);

    modelholder_destroy(modelholder);

    let align;
    switch (params.items_align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            align = params.items_align;
            break;
        default:
            align = ALIGN_START;
            break;
    }

    let menu = {
        items,
        items_size: menumanifest.items_size,

        fontholder,

        transition_delay_in: params.anim_transition_in_delay,
        transition_delay_out: params.anim_transition_out_delay,

        is_vertical: params.is_vertical,
        align: align,
        gap: params.items_gap,
        sparse: params.is_sparse,
        static_index: params.static_index,
        tweenlerp: null,

        drawable: null,
        x, y, width, height,

        index_selected: -1,
        item_choosen: 0,
        offset: 0,

        transition_active: 0,
        transition_out: 0,

        render_start: 0,
        render_end: -1,
        render_distance: 0,
        render_distance_end: 0,
        render_distance_last: 0,

        per_page: params.is_per_page,
        map: null,
        map_size: 0,
        drawcallback_before: 0,
        drawcallback_privatedata: 0,
        drawcallback: null
    };

    if (params.static_index) {
        menu.tweenlerp = tweenlerp_init();
        tweenlerp_add_linear(menu.tweenlerp, 0, 0, 0, 100);
    }

    menu.drawable = drawable_init(z, menu, menu_draw, menu_animate);

    let modifier = drawable_get_modifier(menu.drawable);
    modifier.x = x;
    modifier.y = y;
    modifier.width = width;
    modifier.height = height;

    if (menu.sparse) {
        for (let i = 0; i < menu.items_size; i++) {
            menu.items[i].center_x = menumanifest.items[i].placement.x;
            menu.items[i].center_y = menumanifest.items[i].placement.y;
        }
    } else {
        menu_internal_build_map(menu);
    }

    // select the first visible item
    for (let i = 0; i < menu.items_size; i++) {
        if (!menu.items[i].hidden) {
            menu_internal_set_index_selected(menu, i);
            break;
        }
    }

    return menu;
}

function menu_destroy(menu) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(menu);

    if (menu.fontholder) fontholder_destroy(menu.fontholder);

    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].is_text) {
            textsprite_destroy(menu.items[i].vertex);
        } else {
            statesprite_destroy_texture_if_stateless(menu.items[i].vertex);// important step
            statesprite_destroy(menu.items[i].vertex);
        }

        menu.items[i].name = undefined;

        if (menu.items[i].anim_self) animsprite_destroy(menu.items[i].anim_self);
        if (menu.items[i].anim_discarded) animsprite_destroy(menu.items[i].anim_discarded);
        if (menu.items[i].anim_idle) animsprite_destroy(menu.items[i].anim_idle);
        if (menu.items[i].anim_rollback) animsprite_destroy(menu.items[i].anim_rollback);
        if (menu.items[i].anim_selected) animsprite_destroy(menu.items[i].anim_selected);
        if (menu.items[i].anim_choosen) animsprite_destroy(menu.items[i].anim_choosen);
        if (menu.items[i].anim_in) animsprite_destroy(menu.items[i].anim_in);
        if (menu.items[i].anim_out) animsprite_destroy(menu.items[i].anim_out);
    }

    if (menu.tweenlerp) tweenlerp_destroy(menu.tweenlerp);

    drawable_destroy(menu.drawable);

    menu.items = undefined;
    menu.map = undefined;
    menu = undefined;
}


function menu_get_drawable(menu) {
    return menu.drawable;
}

function menu_trasition_in(menu) {
    menu.transition_out = 0;
    menu_internal_transition(menu);
}

function menu_trasition_out(menu) {
    menu.transition_out = 1;
    menu_internal_transition(menu);
}


function menu_select_item(menu, name) {
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].name == name) {
            if (menu.items[i].hidden) return 0;
            menu_internal_set_index_selected(menu, i);
            return 1;
        };
    }
    menu_internal_set_index_selected(menu, -1);
    return 0;
}

function menu_select_index(menu, index) {
    if (index >= 0 && index < menu.items_size && menu.items[index].hidden) return;
    menu_internal_set_index_selected(menu, index);
}

function menu_select_vertical(menu, offset) {
    if (offset == 0) return 1;
    if (menu.sparse) return menu_internal_set_selected_sparse(menu, 0, offset);
    if (!menu.is_vertical) return 0;

    return menu_internal_scroll(menu, offset);
}

function menu_select_horizontal(menu, offset) {
    if (offset == 0) return 1;
    if (menu.sparse) return menu_internal_set_selected_sparse(menu, offset, 0);
    if (menu.is_vertical) return 0;

    return menu_internal_scroll(menu, offset);
}

function menu_toggle_choosen(menu, enable) {
    menu.item_choosen = !!enable;
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].anim_choosen) animsprite_restart(menu.items[i].anim_choosen);
        if (menu.items[i].anim_discarded) animsprite_restart(menu.items[i].anim_discarded);

        if (menu.index_selected == i) {
            menu.items[i].rollback_active = 0;

            if (menu.items[i].anim_rollback) {
                animsprite_restart(menu.items[i].anim_rollback);
                animsprite_force_end(menu.items[i].anim_rollback);

                if (menu.items[i].is_text) {
                    animsprite_update_textsprite(menu.items[i].anim_rollback, menu.items[i].vertex, 1);
                } else {
                    animsprite_update_statesprite(menu.items[i].anim_rollback, menu.items[i].vertex, 1);
                }
            }
        }
    }
}

function menu_get_selected_index(menu) {
    return menu.index_selected;
}

function menu_get_items_count(menu) {
    return menu.items_size;
}

function menu_set_item_text(menu, index, text) {
    if (index < 0 || index >= menu.items_size) return 0;
    if (!menu.items[index].is_text) return 0;
    textsprite_set_text_intern(menu.items[index].vertex, 0, text);
    return 1;
}

function menu_set_item_visibility(menu, index, visible) {
    if (index < 0 || index >= menu.items_size) return 0;

    menu.items[index].hidden = !visible;

    if (menu.index_selected == index) {
        for (index = menu.index_selected; index > 0; index--) {
            if (!menu.items[menu.index_selected].hidden) break;
        }
        menu_internal_set_index_selected(menu, menu.index_selected);
    }

    menu_internal_build_map(menu);
    return 1;
}

function menu_has_valid_selection(menu) {
    return menu.index_selected >= 0 && menu.index_selected < menu.items_size;
}


function menu_animate(menu, elapsed) {
    for (let i = 0; i < menu.items_size; i++) {
        menu_internal_animate(menu, i, menu.items[i].anim_self, elapsed);

        if (menu.index_selected == i) {
            let anim = menu.item_choosen ? menu.items[i].anim_choosen : menu.items[i].anim_selected;
            menu_internal_animate(menu, i, anim, elapsed);
        } else {
            if (menu.item_choosen) {
                menu_internal_animate(menu, i, menu.items[i].anim_discarded, elapsed);
            } else if (menu.items[i].rollback_active) {
                if (menu_internal_animate(menu, i, menu.items[i].anim_rollback, elapsed)) {
                    menu.items[i].rollback_active = 0;
                }
            } else {
                menu_internal_animate(menu, i, menu.items[i].anim_idle, elapsed);
            }
        }

        if (menu.transition_active) {
            if (menu.transition_out)
                menu_internal_animate(menu, i, menu.items[i].anim_out, elapsed);
            else
                menu_internal_animate(menu, i, menu.items[i].anim_in, elapsed);
        }

        menu_internal_calc_item_bounds(menu, i);
    }

    if (menu.fontholder && menu.fontholder.font_from_atlas)
        fontglyph_animate(menu.fontholder.font, elapsed);

    if (menu.tweenlerp) tweenlerp_animate(menu.tweenlerp, elapsed);

    return 0;
}

function menu_draw(menu, pvrctx) {
    pvr_context_save(pvrctx);

    if (menu.sparse) {
        for (let i = 0; i < menu.items_size; i++) {
            if (menu.items[i].hidden) continue;

            if (menu.drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;

            if (menu.items[i].is_text)
                textsprite_draw(menu.items[i].vertex, pvrctx);
            else
                statesprite_draw(menu.items[i].vertex, pvrctx);

            if (!menu.drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;
        }
    } else {
        let render_distance;

        if (menu.tweenlerp) {
            if (menu.render_distance_last != menu.render_distance) {
                if (tweenlerp_is_completed(menu.tweenlerp)) tweenlerp_restart(menu.tweenlerp);
                tweenlerp_change_bounds_by_index(
                    menu.tweenlerp, 0, menu.render_distance_last, menu.render_distance
                );
                menu.render_distance_last = menu.render_distance;
            }
            render_distance = tweenlerp_peek_value_by_index(menu.tweenlerp, 0);
        } else {
            render_distance = menu.render_distance;
        }

        if (menu.is_vertical)
            sh4matrix_translate_y(pvrctx.current_matrix, render_distance);
        else
            sh4matrix_translate_x(pvrctx.current_matrix, render_distance);

        pvr_context_flush(pvrctx);

        for (let i = 0; i < menu.items_size; i++) {
            if (menu.items[i].hidden) continue;

            if (DEBUG) {
                if (i >= menu.render_start && i <= menu.render_end)
                    pvr_context_set_global_alpha(pvrctx, 1.0);
                else
                    pvr_context_set_global_alpha(pvrctx, 0.2);
            } else {
                // ignore items outside of the visible space
                if (i < menu.render_start || i > menu.render_end) continue;
            }

            if (menu.drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;

            if (menu.items[i].is_text)
                textsprite_draw(menu.items[i].vertex, pvrctx);
            else
                statesprite_draw(menu.items[i].vertex, pvrctx);

            if (!menu.drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;
        }
    }
    pvr_context_restore(pvrctx);
}


function menu_get_item_rect(menu, index, output_location, output_size) {
    if (index < 0 || index >= menu.items_size) return 0;

    let item = menu.items[index];
    if (item.is_text) {
        textsprite_get_draw_location(item.vertex, output_location);
        textsprite_get_draw_size(item.vertex, output_size);
    } else {
        statesprite_get_draw_location(item.vertex, output_location);
        statesprite_get_draw_size(item.vertex, output_size);
    }

    if (menu.is_vertical)
        output_location[1] += menu.render_distance;
    else
        output_location[0] += menu.render_distance;

    return 1;
}

function menu_get_selected_item_rect(menu, output_location, output_size) {
    return menu_get_item_rect(menu, menu.index_selected, output_location, output_size);
}

function menu_get_selected_item_name(menu) {
    if (menu.index_selected < 0 || menu.index_selected >= menu.items_size) return null;

    return menu.items[menu.index_selected].name;
}

function menu_set_text_force_case(menu, none_or_lowercase_or_uppercase) {
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].is_text) {
            textsprite_force_case(menu.items[i].vertex, none_or_lowercase_or_uppercase);
        }
    }
}

function menu_set_draw_callback(menu, before_or_after, callback, privatedata) {
    menu.drawcallback_before = !!before_or_after;
    menu.drawcallback = callback;
    menu.drawcallback_privatedata = privatedata;
}

function menu_has_item(menu, name) {
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].name == name) {
            return 1;
        }
    }
    return 0;
}

function menu_index_of_item(menu, name) {
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].name == name) {
            return i;
        }
    }
    return -1;
}


async function menu_internal_build_item(item, src_item, params, modelholder, fontholder, border) {
    let custom_modelholder = false;
    item.is_text = !!src_item.text;
    item.name = strdup(src_item.name);
    item.hidden = src_item.hidden;

    let dimmen = item.is_text ? params.font_size : params.items_dimmen;
    if (src_item.placement.dimmen > 0) dimmen = src_item.placement.dimmen;

    if (item.is_text) {
        let font_color = params.font_color;
        if (src_item.has_font_color) font_color = src_item.font_color;

        // do not intern the text
        item.vertex = textsprite_init2(fontholder, dimmen, font_color);
        textsprite_set_text_intern(item.vertex, 0, src_item.text);
        if (params.font_border_size > 0) {
            textsprite_border_enable(item.vertex, 1);
            textsprite_border_set_color(
                item.vertex, border[0], border[1], border[2], border[3]
            );
            textsprite_border_set_size(item.vertex, params.font_border_size);
        }
    } else {
        if (src_item.model) {
            let temp = await modelholder_init(src_item.model);
            if (temp) {
                custom_modelholder = true;
                modelholder = temp;
            }
        }
        let statesprite = statesprite_init_from_texture(modelholder_get_texture(modelholder));
        statesprite_set_vertex_color_rgb8(statesprite, modelholder_get_vertex_color(modelholder));
        if (modelholder_is_invalid(modelholder)) statesprite_set_alpha(statesprite, 0);

        item.vertex = statesprite;

        let scale = src_item.texture_scale > 0 ? src_item.texture_scale : params.texture_scale;
        if (scale > 0) {
            item.has_scale = 1;
            statesprite_change_draw_size_in_atlas_apply(item.vertex, 1, scale);
        }
    }

    item.anim_self = menu_internal_load_anim(
        modelholder, src_item.name, null, null
    );
    item.anim_selected = menu_internal_load_anim(
        modelholder, src_item.anim_selected, src_item.name, params.suffix_selected
    );
    item.anim_choosen = menu_internal_load_anim(
        modelholder, src_item.anim_choosen, src_item.name, params.suffix_choosen
    );
    item.anim_discarded = menu_internal_load_anim(
        modelholder, src_item.anim_discarded, src_item.name, params.suffix_discarded
    );
    item.anim_idle = menu_internal_load_anim(
        modelholder, src_item.anim_idle, src_item.name, params.suffix_idle
    );
    item.anim_rollback = menu_internal_load_anim(
        modelholder, src_item.anim_rollback, src_item.name, params.suffix_rollback
    );
    item.anim_in = menu_internal_load_anim(
        modelholder, src_item.anim_in, src_item.name, params.suffix_in
    );
    item.anim_out = menu_internal_load_anim(
        modelholder, src_item.anim_out, src_item.name, params.suffix_out
    );

    if (custom_modelholder) modelholder_destroy(modelholder);

    if (item.anim_self) {
        if (item.is_text)
            animsprite_update_textsprite(item.anim_self, item.vertex, 0);
        else
            animsprite_update_statesprite(item.anim_self, item.vertex, 0);
    }

    if (!params.is_vertical && item.is_text && params.enable_horizontal_text_correction) {
        const draw_size = [0, 0];
        textsprite_get_draw_size(item.vertex, draw_size);
        dimmen = draw_size[0];
    }

    item.cell_dimmen = dimmen;
    item.cell_gap = Number.isFinite(src_item.placement.gap) ? src_item.placement.gap : 0;
}

function menu_internal_load_anim(modelholder, absolute_name, prefix, suffix) {
    let temp;
    if (absolute_name)
        temp = absolute_name;
    else if (prefix && suffix)
        temp = string_concat_for_state_name(2, prefix, suffix);
    else
        return null;

    if (temp == null) return null;

    let animsprite = modelholder_create_animsprite(modelholder, temp, 1, !prefix && !suffix);
    if (!absolute_name) temp = undefined;

    return animsprite;
}

function menu_internal_calc_item_bounds(menu, index) {
    const draw_size = [0, 0];
    const menu_item = menu.items[index];
    let offset_x, offset_y;

    if (menu_item.is_text) {
        textsprite_get_draw_size(menu.items[index].vertex, draw_size);
    } else if (menu_item.has_scale) {
        statesprite_get_draw_size(menu_item.vertex, draw_size);
    } else {
        let max_width, max_height;
        if (menu.is_vertical) {
            max_width = -1;
            max_height = menu_item.cell_dimmen;
        } else {
            max_width = menu_item.cell_dimmen;
            max_height = -1;
        }
        statesprite_resize_draw_size(menu_item.vertex, max_width, max_height, draw_size);
    }

    if (menu.sparse) {
        offset_x = (draw_size[0] / -2) + menu.x;
        offset_y = (draw_size[1] / -2) + menu.y;
    } else if (menu.align == ALIGN_START) {
        offset_x = 0;
        offset_y = 0;
    } else {
        if (menu.is_vertical) {
            offset_x = menu.width - draw_size[0];
            if (menu.align == ALIGN_CENTER) offset_x /= 2;
            offset_y = 0;
        } else {
            offset_y = menu.height - draw_size[1];
            if (menu.align == ALIGN_CENTER) offset_y /= 2;
            offset_x = 0;
        }
    }

    offset_x += menu.items[index].center_x;
    offset_y += menu.items[index].center_y;

    if (menu.items[index].is_text)
        textsprite_set_draw_location(menu.items[index].vertex, offset_x, offset_y);
    else
        statesprite_set_draw_location(menu.items[index].vertex, offset_x, offset_y);
}

function menu_internal_animate(menu, index, anim, elapsed) {
    if (!anim) return 1;

    let completed = animsprite_animate(anim, elapsed);

    if (menu.items[index].is_text)
        animsprite_update_textsprite(anim, menu.items[index].vertex, 1);
    else
        animsprite_update_statesprite(anim, menu.items[index].vertex, 1);

    return completed;
}

function menu_internal_set_index_selected(menu, new_index) {
    let old_index = menu.index_selected;

    if (old_index >= 0 && old_index < menu.items_size) {
        menu.items[old_index].rollback_active = !!menu.items[old_index].anim_rollback;

        if (menu.items[old_index].anim_rollback) {
            animsprite_restart(menu.items[old_index].anim_rollback);
            if (menu.item_choosen) {
                animsprite_force_end(menu.items[old_index].anim_rollback);
                if (menu.items[old_index].is_text) {
                    animsprite_update_textsprite(
                        menu.items[old_index].anim_rollback, menu.items[old_index].vertex, 1
                    );
                } else {
                    animsprite_update_statesprite(
                        menu.items[old_index].anim_rollback, menu.items[old_index].vertex, 1
                    );
                }
                menu.items[old_index].rollback_active = 0;
            }
        }
        if (menu.items[old_index].anim_idle)
            animsprite_restart(menu.items[old_index].anim_idle);
    }

    if (new_index >= 0 && new_index < menu.items_size) {
        menu.items[new_index].rollback_active = 0;

        if (menu.items[new_index].anim_self)
            animsprite_restart(menu.items[new_index].anim_self);

        if (menu.items[new_index].anim_rollback) {
            animsprite_restart(menu.items[new_index].anim_rollback);
            animsprite_force_end(menu.items[new_index].anim_rollback);

            if (menu.items[new_index].is_text) {
                animsprite_update_textsprite(
                    menu.items[new_index].anim_rollback, menu.items[new_index].vertex, 1
                );
            } else {
                animsprite_update_statesprite(
                    menu.items[new_index].anim_rollback, menu.items[new_index].vertex, 1
                );
            }
        }

        if (menu.items[new_index].anim_selected)
            animsprite_restart(menu.items[new_index].anim_selected);
    }

    menu.index_selected = new_index;
    if (new_index < 0 || new_index >= menu.items_size) return;
    if (menu.sparse) return;


    // calculate the visible page of the list
    if (!menu.map) {
        let has_static_index = menu.static_index != 0;

        L_static_index:
        if (has_static_index) {
            let lower_index = new_index - menu.static_index;
            if (lower_index >= menu.items_size) lower_index = menu.items_size - 1;

            while (lower_index > 0 && menu.items[lower_index].hidden) lower_index--;

            if (lower_index < 0) {
                if (old_index < 0) old_index = 0;
                has_static_index = false;
                break L_static_index;
            }

            menu.render_start = lower_index;
            menu.render_end = menu.items_size;
            menu.render_distance = menu.items[lower_index].render_distance;

        }

        if (!has_static_index) {
            if (menu.render_end > 0 && (new_index - old_index) == 1) {
                menu.render_start++;
                menu.render_end++;
                menu.render_distance = -menu.items[menu.render_start].render_distance;
                return;
            }

            menu.render_start = new_index;
            menu.render_end = menu.items_size;
            menu.render_distance = menu.items[new_index].render_distance;
        }

        let stop = menu.render_distance + (menu.is_vertical ? menu.height : menu.width);
        for (let i = menu.items_size - 1; i >= 0; i--) {
            if (menu.items[i].render_distance < stop) {
                if (stop < menu.render_distance_end) menu.render_end = i - 1;
                break;
            }
        }
        menu.render_distance = - menu.render_distance;
        return;
    }

    if (new_index < menu.render_start && new_index > menu.render_end) return;

    menu.render_start = -1;
    menu.render_end = menu.items_size;
    menu.render_distance = 0;

    for (let i = 0; i < menu.map_size; i++) {
        if (menu.map[i] > menu.index_selected) {
            menu.render_end = menu.map[i] - 1;
            i--;
            if (i >= 0) {
                menu.render_start = menu.map[i];
                menu.render_distance = menu.items[menu.map[i]].render_distance;
            }
            break;
        }
    }

    if (menu.render_start < 0 && menu.map_size > 0) {
        menu.render_start = menu.map[menu.map.length - 1];
        menu.render_distance = menu.items[menu.render_start].render_distance;
    }

    menu.render_distance = -menu.render_distance;
}

function menu_internal_transition(menu) {
    let anim;
    let transition_delay = menu.transition_out ? menu.transition_delay_out : menu.transition_delay_in;

    let reverse = transition_delay < 0;
    if (reverse) transition_delay = Math.abs(transition_delay);

    let nonvisible_first_delay = transition_delay * menu.render_start;
    let nonvisible_last_delay = transition_delay * menu.render_end;

    menu.transition_active = 1;

    for (let i = 0; i < menu.items_size; i++) {
        if (menu.transition_out)
            anim = menu.items[i].anim_out;
        else
            anim = menu.items[i].anim_in;

        if (!anim) continue;
        animsprite_restart(anim);

        let delay;

        if (i < menu.render_start) {
            delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
        } else if (i > menu.render_end) {
            delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
        } else {
            if (reverse)
                delay = transition_delay * (menu.render_end - i);
            else
                delay = transition_delay * (i - menu.render_start);
        }

        animsprite_set_delay(anim, delay);
    }

}

function menu_internal_set_selected_sparse(menu, offset_x, offset_y) {
    if (menu.index_selected < 0 || menu.index_selected >= menu.items_size) {
        menu_select_index(menu, 0);
        return 1;
    }

    let short_index = -1;
    let short_distance = Infinity;

    let center_x = menu.items[menu.index_selected].center_x;
    let center_y = menu.items[menu.index_selected].center_y;

    for (let i = 0; i < menu.items_size; i++) {
        if (menu.index_selected == i) continue;
        if (menu.items[i].hidden) continue;

        if (offset_x != 0) {
            if (offset_x > 0) {
                if (menu.items[i].center_x < center_x) continue;
            } else {
                if (menu.items[i].center_x > center_x) continue;
            }
        }
        if (offset_y != 0) {
            if (offset_y > 0) {
                if (menu.items[i].center_y < center_y) continue;
            } else {
                if (menu.items[i].center_y > center_y) continue;
            }
        }

        let distance = math2d_points_distance(
            menu.items[i].center_x, menu.items[i].center_y, center_x, center_y
        );

        if (distance < short_distance) {
            short_distance = distance;
            short_index = i;
        }
    }

    if (short_index < 0) return 0;

    menu_internal_set_index_selected(menu, short_index);
    return 1;
}

function menu_internal_build_map(menu) {
    if (menu.sparse) return;

    // dispose last map created (if defined)
    menu.map = null;
    menu.map_size = 0;

    let accumulator = 0;
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].hidden) continue;
        menu.items[i].render_distance = accumulator;
        accumulator += menu.items[i].cell_dimmen + (menu.items[i].cell_gap * 2) + menu.gap;
    }

    let render_distance = Infinity;
    for (let i = menu.items_size - 1; i >= 0; i--) {
        if (menu.items[i].hidden)
            menu.items[i].render_distance = render_distance;
        else
            render_distance = menu.items[i].render_distance;
    }

    let dimmen = menu.is_vertical ? menu.height : menu.width;
    let pages = (accumulator - menu.gap) / dimmen;

    menu.render_distance_end = accumulator;

    if (menu.per_page && pages > 0) {
        menu.map_size = Math.ceil(pages);
        menu.map = new Array(menu.map_size);
    }

    let j = 0;
    let next_distance = -1;
    for (let i = 0; i < menu.items_size; i++) {
        if (menu.items[i].hidden) continue;

        let draw_x = menu.x + menu.items[i].placement_x;
        let draw_y = menu.y + menu.items[i].placement_y;

        if (menu.is_vertical) {
            draw_y += menu.items[i].render_distance + menu.items[i].cell_gap;
        } else {
            draw_x += menu.items[i].render_distance + menu.items[i].cell_gap;
        }

        if (menu.map && (menu.items[i].render_distance - menu.gap) > next_distance) {
            menu.map[j++] = Math.max(0, i - 1);
            next_distance = dimmen * j;
        }

        menu.items[i].center_x = draw_x;
        menu.items[i].center_y = draw_y;
    }

    if (menu.map) {
        if (j < menu.map_size) menu.map[j] = menu.items_size - 1;

        if (menu.map_size > 1)
            menu.render_end = menu.map[1] - 1;
        else
            menu.render_end = menu.items_size;
    }
}

function menu_internal_scroll(menu, offset) {
    let new_index = offset + menu.index_selected;

    // skip hidden items
    while (new_index >= 0 && new_index < menu.items_size && menu.items[new_index].hidden) {
        new_index += offset;
    }

    if (new_index < 0 || new_index >= menu.items_size) return 0;

    menu_internal_set_index_selected(menu, new_index);
    return 1;
}

function menu_internal_draw_callback(menu, pvrctx, index) {
    if (menu.drawcallback == null) return false;

    const size = [0, 0];
    const location = [0, 0];
    let item = menu.items[index];

    if (item.is_text) {
        textsprite_get_draw_location(item.vertex, location);
        textsprite_get_draw_size(item.vertex, size);
    } else {
        sprite_get_draw_location(item.vertex, location);
        sprite_get_draw_size(item.vertex, size);
    }

    return !menu.drawcallback(
        menu.drawcallback_privatedata, pvrctx, menu, index, location[0], location[1], size[0], size[1]
    );
}

