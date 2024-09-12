#ifndef _weekselector_helptext_h
#define _weekselector_helptext_h

#include <stdbool.h>
#include <stdint.h>

//
//  forward references
//

typedef struct Layout_s* Layout;
typedef struct ModelHolder_s* ModelHolder;
typedef struct Drawable_s* Drawable;


typedef struct WeekSelectorHelpText_s* WeekSelectorHelpText;


WeekSelectorHelpText weekselector_helptext_init(ModelHolder modelholder, Layout layout, int32_t row, bool start, const char* icon_name, const char* string1, const char* string2);
void weekselector_helptext_destroy(WeekSelectorHelpText* helptext);
void weekselector_helptext_set_visible(WeekSelectorHelpText helptext, bool visible);
void weekselector_helptext_use_alt(WeekSelectorHelpText helptext, bool use_alt);
Drawable weekselector_helptext_get_drawable(WeekSelectorHelpText helptext);

#endif