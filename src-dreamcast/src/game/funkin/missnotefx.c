#include "game/funkin/missnotefx.h"

#include "externals/luascript.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "soundplayer.h"


struct MissNoteFX_s {
    SoundPlayer missnote1;
    SoundPlayer missnote2;
    SoundPlayer missnote3;
    bool disabled;
};


MissNoteFX missnotefx_init() {
    MissNoteFX missnotefx = malloc_chk(sizeof(struct MissNoteFX_s));
    malloc_assert(missnotefx, MissNoteFX);

    *missnotefx = (struct MissNoteFX_s){
        .missnote1 = soundplayer_init("/assets/common/sound/missnote1.ogg"),
        .missnote2 = soundplayer_init("/assets/common/sound/missnote2.ogg"),
        .missnote3 = soundplayer_init("/assets/common/sound/missnote3.ogg"),
        .disabled = false
    };

    return missnotefx;
}

void missnotefx_destroy(MissNoteFX* missnotefx_ptr) {
    if (!missnotefx_ptr || !*missnotefx_ptr) return;

    MissNoteFX missnotefx = *missnotefx_ptr;

    luascript_drop_shared(missnotefx);

    if (missnotefx->missnote1) soundplayer_destroy(&missnotefx->missnote1);
    if (missnotefx->missnote2) soundplayer_destroy(&missnotefx->missnote2);
    if (missnotefx->missnote3) soundplayer_destroy(&missnotefx->missnote3);

    free_chk(missnotefx);
    *missnotefx_ptr = NULL;
}

void missnotefx_stop(MissNoteFX missnotefx) {
    if (missnotefx->missnote1) soundplayer_stop(missnotefx->missnote1);
    if (missnotefx->missnote2) soundplayer_stop(missnotefx->missnote2);
    if (missnotefx->missnote3) soundplayer_stop(missnotefx->missnote3);
}

void missnotefx_disable(MissNoteFX missnotefx, bool disabled) {
    missnotefx->disabled = disabled;
}

void missnotefx_play_effect(MissNoteFX missnotefx) {
    if (missnotefx->disabled) return;

    int32_t soundplayer_id = math2d_random_int(0, 3);
    float volume = math2d_lerp(0.1f, 0.3f, math2d_random_float());

    SoundPlayer soundplayer;
    switch (soundplayer_id) {
        case 2:
            soundplayer = missnotefx->missnote3;
            break;
        case 1:
            soundplayer = missnotefx->missnote2;
            break;
        default:
        case 0:
            soundplayer = missnotefx->missnote1;
            break;
    }

    if (soundplayer) {
        soundplayer_set_volume(soundplayer, volume);
        soundplayer_replay(soundplayer);
    }
}
