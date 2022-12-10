
const PREFIX_DAD: string = ":dad:";
const PREFIX_BF: string = ":bf:";
const CHARS_DELAY: number = 40;// milliseconds
const GAMEPAD_XA = GamepadButtons.X | GamepadButtons.A;

type DialogLine = { isSenpai: boolean; text: string; };

let stage: Layout = null;
let lyt_icon_boyfriend: Sprite = null;
let lyt_icon_senpai: Sprite = null;
let lyt_dialog_text: TextSprite = null;
let lyt_snd_char0: SoundPlayer = null;
let lyt_snd_char1: SoundPlayer = null;
let lyt_snd_click: SoundPlayer = null;
let lyt_bg_music: SoundPlayer = null;
let lyt_dialog_normal: Sprite = null;
let lyt_dialog_angry: Sprite = null;
let lyt_snd_angry: SoundPlayer = null;

let dialogs_senpai: DialogLine[] = null;
let dialogs_roses: DialogLine[] = null;
let dialogs_thorns: DialogLine[] = null;

let dialog_corutine: LuaThread = null;
let dialog_button_pressed: boolean = false;
let dialog_waiting_confirm: boolean = false;
let dialog_force_end: boolean = false;
let dialog_is_angry: boolean = false;
let dialog_is_evil: boolean = false;
let in_freeplay_mode: boolean = false;



function f_weekinit(freeplay_index: number): void {
    if (freeplay_index >= 0) {
        in_freeplay_mode = true;
        return;
    }
    dialogs_senpai = dialog_parse("/assets/weeks/week6/weeb/dialogs/senpaiDialogue.txt");
    dialogs_roses = dialog_parse("/assets/weeks/week6/weeb/dialogs/rosesDialogue.txt");
    dialogs_thorns = dialog_parse("/assets/weeks/week6/weeb_but_evil/dialogs/thornsDialogue.txt");

    week_enable_credits_on_completed();
    week_unlockdirective_create("FNF_COMPLETED", false, true, 9942069);
}

function f_beforeready(from_retry: boolean): void {
    stage = week_get_stage_layout();

    let [, , track_index] = week_get_current_track_info();

    switch (track_index) {
        case 0:
            stage.trigger_trigger("girls_trigger");
            break;
        case 1:
            stage.trigger_trigger("dissuaded_girls_trigger");
            break;
        case 2:
            stage.stop_all_triggers();
            break;
        default:
            return;
    }


    if (from_retry || in_freeplay_mode) return;

    lyt_icon_boyfriend = stage.get_sprite("dialog_icon_boyfriend");
    lyt_icon_senpai = stage.get_sprite("dialog_icon_senpai");
    lyt_dialog_text = stage.get_textsprite("dialog_text");
    lyt_snd_char0 = stage.get_soundplayer("sndChar0");
    lyt_snd_char1 = stage.get_soundplayer("sndChar1");
    lyt_snd_click = stage.get_soundplayer("sndClick");
    lyt_bg_music = stage.get_soundplayer("bgMusic");
    lyt_dialog_normal = stage.get_sprite("dialog_normal");
    lyt_dialog_angry = stage.get_sprite("dialog_angry");
    lyt_snd_angry = stage.get_soundplayer("angrySenpai");
    dialog_is_angry = track_index == 1;
    dialog_is_evil = track_index == 2;

    let dialog_lines: DialogLine[];
    switch (track_index) {
        case 0:
            dialog_lines = dialogs_senpai;
            break;
        case 1:
            dialog_lines = dialogs_roses;
            break;
        case 2:
            dialog_lines = dialogs_thorns;
            break;
        default:
            return;
    }

    if (!dialog_lines) return;

    week_set_halt(true);
    week_ui_set_visibility(false);

    if (track_index == 0 || track_index == 2) lyt_bg_music.play();

    if (track_index == 2) stage.trigger_action(null, "fade_in_screen");

    timer_callback_timeout(1830, function () {
        // CORUTINES SON
        dialog_corutine = coroutine.create(dialog_show);
        coroutine.resume(dialog_corutine, dialog_lines);
    });
}

function f_frame(elapsed: number): void {
    if (dialog_corutine != null && !dialog_waiting_confirm) {
        coroutine.resume(dialog_corutine, elapsed);
    }
}

function f_buttons(player_id: number, buttons: GamepadButtons): void {
    if (dialog_corutine == null || player_id == 0) return;

    if ((buttons & (GamepadButtons.A | GamepadButtons.X)) != 0x00) {
        dialog_button_pressed = true;
    } else if ((buttons & GamepadButtons.START) != 0x00) {
        dialog_force_end = true;
    } else
        return;

    if (dialog_waiting_confirm) coroutine.resume(dialog_corutine);
}

function f_roundend(loose: boolean): void {
    if (loose || !dialog_is_angry || in_freeplay_mode) return;

    //
    // senpai death
    //

    week_set_halt(true);
    week_ui_set_visibility(false);

    stage.trigger_camera("senpai_dies");
    stage.trigger_action(null, "fade_in_red");

    timer_callback_timeout(350, function () {
        stage.trigger_action(null, "senpaiCrazy");
        stage.get_soundplayer("senpaiDies").play();
    });
    timer_callback_timeout(350 + 4700, function () {
        stage.trigger_action(null, "fade_in_white");
    });
    timer_callback_timeout(350 + 4500 + 3100, function () {
        week_set_halt(false);
    });
}




function dialog_show(dialog_lines: DialogLine[]): void {

    stage.set_group_visibility("hand", false);

    if (dialog_is_evil) {
        stage.set_group_visibility("dialogs", true);
        stage.trigger_action(null, "dialog_anim");
    } else {
        dialog_mgr(true);
    }

    if (dialog_is_angry) lyt_snd_angry.play();

    for (let i = 0; i < dialog_lines.length; i++) {
        let text: string = dialog_lines[i].text;
        let text_length = text.length;
        let alt_snd: boolean = false;

        let progress: number = i == 0 ? -500 : -200;
        let next_char: number = CHARS_DELAY;
        let end: number = 0;

        // show character portrait
        dialog_chrtr(dialog_lines[i].isSenpai);
        lyt_dialog_text.set_text("");

        dialog_force_end = false;
        dialog_button_pressed = false;
        dialog_waiting_confirm = false;

        // render text char by char
        while (end <= text_length) {
            // suspend and wait for next frame
            let elapsed: number = coroutine.yield()[0];

            if (dialog_force_end) {
                break;
            } else if (progress > next_char) {
                end = Math.floor(progress / CHARS_DELAY);
                lyt_dialog_text.set_text(text.substring(0, end));
                next_char = progress + CHARS_DELAY;
                let snd = alt_snd ? lyt_snd_char0 : lyt_snd_char1;
                snd.stop();
                snd.play();
                alt_snd = !alt_snd;
            } else if (dialog_button_pressed) {
                lyt_dialog_text.set_text(text);
                dialog_button_pressed = false;
                break;
            }

            progress += elapsed;
        }

        if (dialog_force_end) {
            lyt_snd_char0.play();
            lyt_snd_char1.play();
            break;
        }

        // wait for a button before continue with the next line
        stage.set_group_visibility("hand", true);
        dialog_waiting_confirm = true;
        coroutine.yield();
        lyt_snd_click.play();
        stage.set_group_visibility("hand", false);
    }

    dialog_corutine = null;

    if (dialog_is_evil) {
        stage.trigger_action(null, "fade_out_group");
    } else {
        dialog_mgr(false);
    }

    lyt_bg_music.fade(false, 500);

    timer_callback_timeout(500, function () {
        week_set_halt(false);
        week_ui_set_visibility(true);
    });
}

function dialog_mgr(show: boolean): void {
    lyt_dialog_normal.set_visible(!dialog_is_angry);
    lyt_dialog_angry.set_visible(dialog_is_angry);

    if (show) {
        stage.set_group_visibility("dialogs", show);
        stage.trigger_action(null, "dialog_anim");
    } else {
        lyt_icon_boyfriend.set_visible(false);
        lyt_icon_senpai.set_visible(false);
        stage.trigger_action(null, "fade_out_group");
    }
}

function dialog_chrtr(isSenpai: boolean): void {
    if (dialog_is_evil) return;
    lyt_icon_boyfriend.set_visible(!isSenpai);
    lyt_icon_senpai.set_visible(isSenpai && !dialog_is_angry);
    stage.trigger_action(null, isSenpai ? "dialog_anim_senpai" : "dialog_anim_boyfriend");
}

function dialog_parse(src_dialogue: string): DialogLine[] {
    let text: string = fs_readfile(src_dialogue);
    if (text == null) return null;

    let length: number = text.length;
    let lines: DialogLine[] = [];
    let isSenpai: boolean = false;

    for (let i = 0; i < length;) {
        let end = text.indexOf("\n", i);
        if (end < 0) end = length;

        let line = text.substring(i, end);
        let prefix;

        if (line.startsWith(PREFIX_BF)) {
            prefix = PREFIX_BF;
            isSenpai = false;
        } else if (line.startsWith(PREFIX_DAD)) {
            prefix = PREFIX_DAD;
            isSenpai = true;
        } else {
            prefix = null;
        }

        if (prefix != null) {
            lines.push({
                isSenpai: isSenpai,
                text: line.substring(prefix.length)
            });
        }

        i = end + 1;
    }

    return lines.length < 1 ? null : lines;
}
