#ifndef _weekselector_h
#define _weekselector_h

#include <stdint.h>


#define WEEKSELECTOR_BUTTON_DELAY 200
extern const char* WEEKSELECTOR_BUTTON_X;
extern const char* WEEKSELECTOR_BUTTON_B;
extern const char* WEEKSELECTOR_BUTTON_LT_RT;
extern const char* WEEKSELECTOR_BUTTON_START;

extern const char* WEEKSELECTOR_LOCKED;

extern const char* WEEKSELECTOR_BUTTONS_MODEL;
extern const char* WEEKSELECTOR_UI_ICONS_MODEL;
extern const char* WEEKSELECTOR_UI_ANIMS;

#define WEEKSELECTOR_ARROW_DISABLED_ALPHA 0.1f
#define WEEKSELECTOR_PREVIEW_DISABLED_ALPHA 0.7f
extern const char* WEEKSELECTOR_ARROW_SPRITE_NAME;


int32_t weekselector_main();


#endif
