#ifndef _weekselector_weekmsg_h
#define _weekselector_weekmsg_h

#include <stdbool.h>
#include <stdint.h>

//
//  forward references
//

typedef struct Layout_s* Layout;


typedef struct WeekSelectorWeekMSG_s* WeekSelectorWeekMSG;


WeekSelectorWeekMSG weekselector_weekmsg_init(Layout layout, const char* title, const char* placeholder_title_name, const char* placeholder_message_name);
void weekselector_weekmsg_destroy(WeekSelectorWeekMSG* weekmsg);
void weekselector_weekmsg_set_message(WeekSelectorWeekMSG weekmsg, const char* message);
void weekselector_weekmsg_visible(WeekSelectorWeekMSG weekmsg, bool visible);
void weekselector_weekmsg_disabled(WeekSelectorWeekMSG weekmsg, bool disabled);


#endif
