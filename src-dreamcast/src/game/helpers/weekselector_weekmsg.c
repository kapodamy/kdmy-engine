#include "game/helpers/weekselector_weekmsg.h"

#include "layout.h"
#include "malloc_utils.h"
#include "textsprite.h"

struct WeekSelectorWeekMSG_s {
    TextSprite title;
    TextSprite message;
    bool no_message;
};


static const float WEEKSELECTOR_MSG_DISABLED_ALPHA = 0.5f;


WeekSelectorWeekMSG weekselector_weekmsg_init(Layout layout, const char* title, const char* placeholder_title_name, const char* placeholder_message_name) {
    WeekSelectorWeekMSG weekmsg = malloc_chk(sizeof(struct WeekSelectorWeekMSG_s));
    malloc_assert(weekmsg, WeekSelectorWeekMSG);

    *weekmsg = (struct WeekSelectorWeekMSG_s){
        .title = layout_get_textsprite(layout, placeholder_title_name),
        .message = layout_get_textsprite(layout, placeholder_message_name),
        .no_message = true
    };
    if (weekmsg->title) textsprite_set_text_intern(weekmsg->title, true, (const char* const*)&title);
    weekselector_weekmsg_visible(weekmsg, false);
    return weekmsg;
}

void weekselector_weekmsg_destroy(WeekSelectorWeekMSG* weekmsg_ptr) {
    if (!weekmsg_ptr || !*weekmsg_ptr) return;

    WeekSelectorWeekMSG weekmsg = *weekmsg_ptr;

    free_chk(weekmsg);
    *weekmsg_ptr = NULL;
}

void weekselector_weekmsg_set_message(WeekSelectorWeekMSG weekmsg, const char* message) {
    weekmsg->no_message = !message;

    if (weekmsg->title) textsprite_set_alpha(weekmsg->title, 1.0f);

    if (!weekmsg->no_message) {
        textsprite_set_text_intern(weekmsg->message, true, (const char* const*)&message);
        textsprite_set_alpha(weekmsg->message, 1.0f);
    }

    weekselector_weekmsg_visible(weekmsg, true);
}

void weekselector_weekmsg_visible(WeekSelectorWeekMSG weekmsg, bool visible) {
    if (visible && weekmsg->no_message) visible = false;
    if (weekmsg->title) textsprite_set_visible(weekmsg->title, visible);
    if (weekmsg->message) textsprite_set_visible(weekmsg->message, visible);
}

void weekselector_weekmsg_disabled(WeekSelectorWeekMSG weekmsg, bool disabled) {
    float alpha = disabled ? WEEKSELECTOR_MSG_DISABLED_ALPHA : 1.0f;
    if (weekmsg->title) textsprite_set_alpha(weekmsg->title, alpha);
    if (weekmsg->message) textsprite_set_alpha(weekmsg->message, alpha);
}
