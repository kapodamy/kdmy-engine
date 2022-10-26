"use strict";

async function modding_init(layout, src_script) {
    let moddingcontext = {
        script: null,
        layout,
        has_exit: 0
    };

    if (src_script && await fs_file_exists(src_script))
        moddingcontext.script = await weekscript_init(src_script, moddingcontext, 0);

    return moddingcontext;
}

function modding_destroy(moddingcontext) {
    if (moddingcontext.script != null) weekscript_destroy(moddingcontext.script);
    moddingcontext = undefined;
}

function modding_get_layout(moddingcontext) {
    return moddingcontext.layout;
}

function modding_exit(moddingcontext) {
    moddingcontext.has_exit = 1;
}

