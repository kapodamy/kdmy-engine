#ifndef _freeplaymenu_songicons_h
#define _freeplaymenu_songicons_h

#include <stdint.h>
#include <stdbool.h>

#include "pvrcontext_types.h"


//
//  forward references
//

typedef struct ArrayList_s* ArrayList;
typedef struct Menu_s* Menu;

typedef struct FreeplaySongIcons_s* FreeplaySongIcons;


FreeplaySongIcons freeplaymenu_songicons_init(ArrayList song_map, float max_dimmen, float font_size);
void freeplaymenu_songicons_destroy(FreeplaySongIcons* songicons);
bool freeplaymenu_songicons_draw_item_icon(FreeplaySongIcons songicons, PVRContext pvrctx, Menu menu, int32_t idx, float x, float y, float w, float h);

#endif