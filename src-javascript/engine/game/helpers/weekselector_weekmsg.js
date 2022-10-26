"use strict";

const WEEKSELECTOR_MSG_DISABLED_ALPHA = 0.5;

function weekselector_weekmsg_init(layout, title, placeholder_title_name, placeholder_message_name) {
    let weekmsg = {
        title: layout_get_textsprite(layout, placeholder_title_name),
        message: layout_get_textsprite(layout, placeholder_message_name),
        no_message: 1
    };
    if (weekmsg.title) textsprite_set_text_intern(weekmsg.title, 1, title);
    weekselector_weekmsg_visible(weekmsg, 0);
    return weekmsg;
}

function weekselector_weekmsg_destroy(weekmsg) {
    weekmsg = undefined;
}

function weekselector_weekmsg_set_message(weekmsg, message) {
    weekmsg.no_message = message ? 0 : 1;

    if (weekmsg.title) textsprite_set_alpha(weekmsg.title, 1.0);

    if (!weekmsg.no_message) {
        textsprite_set_text_intern(weekmsg.message, 1, message);
        textsprite_set_alpha(weekmsg.message, 1.0);
    }

    weekselector_weekmsg_visible(weekmsg, 1);
}

function weekselector_weekmsg_visible(weekmsg, visible) {
    if (visible && weekmsg.no_message) visible = 0;
    if (weekmsg.title) textsprite_set_visible(weekmsg.title, visible);
    if (weekmsg.message) textsprite_set_visible(weekmsg.message, visible);
}

function weekselector_weekmsg_disabled(weekmsg, disabled) {
    let alpha = disabled ? WEEKSELECTOR_MSG_DISABLED_ALPHA : 1.0;
    if (weekmsg.title) textsprite_set_alpha(weekmsg.title, alpha);
    if (weekmsg.message) textsprite_set_alpha(weekmsg.message, alpha);
}

