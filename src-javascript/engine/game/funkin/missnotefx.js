"use strict";

async function missnotefx_init() {
    return {
        missnote1: await soundplayer_init("/assets/common/sound/missnote1.ogg"),
        missnote2: await soundplayer_init("/assets/common/sound/missnote2.ogg"),
        missnote3: await soundplayer_init("/assets/common/sound/missnote3.ogg"),
        disabled: false
    };
}

function missnotefx_destroy(missnotefx) {
    luascript_drop_shared(missnotefx);

    if (missnotefx.missnote1) soundplayer_destroy(missnotefx.missnote1);
    if (missnotefx.missnote2) soundplayer_destroy(missnotefx.missnote2);
    if (missnotefx.missnote3) soundplayer_destroy(missnotefx.missnote3);
    missnotefx = undefined;
}


function missnotefx_stop(missnotefx) {
    if (missnotefx.missnote1) soundplayer_stop(missnotefx.missnote1);
    if (missnotefx.missnote2) soundplayer_stop(missnotefx.missnote2);
    if (missnotefx.missnote3) soundplayer_stop(missnotefx.missnote3);
}

function missnotefx_disable(missnotefx, disabled) {
    missnotefx.disabled = !!disabled;
}

function missnotefx_play_effect(missnotefx) {
    if (missnotefx.disabled) return;

    let soundplayer_id = math2d_random_int(0, 3);
    let volume = math2d_lerp(0.1, 0.3, math2d_random_float());

    let soundplayer;
    switch (soundplayer_id) {
        case 2:
            soundplayer = missnotefx.missnote3;
            break;
        case 1:
            soundplayer = missnotefx.missnote2;
            break;
        default:
        case 0:
            soundplayer = missnotefx.missnote1;
            break;
    }

    if (soundplayer) {
        soundplayer_set_volume(soundplayer, volume);
        soundplayer_replay(soundplayer);
    }
}
