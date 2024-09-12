#ifndef _luascript_platform_h
#define _luascript_platform_h

#include <stdbool.h>
#include <stdint.h>


//
//  forward reference
//

typedef struct Luascript_s* Luascript;


void luascript_platform_add_luascript(Luascript luascript);
void luascript_platform_remove_luascript(Luascript luascript);

void luascript_platform_poll_window_state();
void luascript_platform_restore_window_title_if_changed();

void luascript_platform_open_www_link(const char* url);
void luascript_platform_request_window_attention();
void luascript_platform_change_window_title(const char* title, bool changed_from_modding_context);
void luascript_platform_restore_window_title_if_changed();
const char* luascript_platform_get_language();
const char* luascript_platform_get_username();
void luascript_platform_exit(int exit_code);
void luascript_platform_get_screen_size(int32_t* screen_width, int32_t* screen_height);

#endif
