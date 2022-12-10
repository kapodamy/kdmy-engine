"use strict";

async function modding_init(layout, src_script) {
    let moddingcontext = {
        script: null,
        layout,
        has_exit: 0,
        has_halt: 0,
        ui_visible: 1,
        has_funkinsave_changes: 0
    };

    if (src_script && await fs_file_exists(src_script))
        moddingcontext.script = await weekscript_init(src_script, moddingcontext, 0);

    return moddingcontext;
}

async function modding_destroy(moddingcontext) {
    if (moddingcontext.has_funkinsave_changes) funkinsave_write_to_vmu();
    if (moddingcontext.script != null) weekscript_destroy(moddingcontext.script);
    moddingcontext = undefined;
}

function modding_get_layout(moddingcontext) {
    return moddingcontext.layout;
}

function modding_exit(moddingcontext) {
    moddingcontext.has_exit = 1;
}

function modding_set_halt(moddingcontext, halt) {
    moddingcontext.has_halt = !!halt;
}


function modding_set_ui_visibility(moddingcontext, visible) {
    moddingcontext.ui_visible = !!visible;
}

function modding_unlockdirective_create(moddingcontext, name, value) {
    moddingcontext.has_funkinsave_changes = 1;
    funkinsave_create_unlock_directive(name, value);
}

function modding_unlockdirective_has(moddingcontext, name) {
    return funkinsave_contains_unlock_directive(name);
}

function modding_unlockdirective_get(moddingcontext, name) {
    let value = [0.0];
    funkinsave_read_unlock_directive(name, value);
    return value[0];
}

function modding_unlockdirective_remove(moddingcontext, name) {
    moddingcontext.has_funkinsave_changes = 1;
    funkinsave_delete_unlock_directive(name);
}

