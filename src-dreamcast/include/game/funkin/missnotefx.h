#ifndef _missnotefx_h
#define _missnotefx_h

#include <stdbool.h>


typedef struct MissNoteFX_s* MissNoteFX;

MissNoteFX missnotefx_init();
void missnotefx_destroy(MissNoteFX* missnotefx);

void missnotefx_stop(MissNoteFX missnotefx);
void missnotefx_disable(MissNoteFX missnotefx, bool disabled);
void missnotefx_play_effect(MissNoteFX missnotefx);

#endif
