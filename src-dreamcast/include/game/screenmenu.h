#ifndef _screenmenu_h
#define _screenmenu_h

#include "modding_types.h"
#include "pvrcontext_types.h"


typedef struct ScreenMenu_s* ScreenMenu;


ScreenMenu screenmenu_init(const char* layout_src, const char* script_src);
void screenmenu_destroy(ScreenMenu* screenmenu);
void* screenmenu_display(ScreenMenu screenmenu, PVRContext pvrctx, const void* script_arg, const ModdingValueType script_arg_type, ModdingValueType* ret_type);


#endif
