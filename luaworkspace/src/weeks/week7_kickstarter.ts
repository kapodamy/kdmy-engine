let layout: Layout;
let www_selected: boolean = true;
let video_playing: boolean = false;
let video: VideoPlayer = null;
let bg_music: SoundPlayer = modding_get_native_background_music();
let bg_music_fade_id: number = -1;

const enum Nav {
    back = "back",
    ok = "ok",
    up = "up",
    down = "down"
}


function f_modding_init(arg: BasicValue): void {
    layout = modding_get_layout();
    video = layout.get_videoplayer("trailer");

    modding_set_exit_delay(500);

    if (bg_music) {
        bg_music.fade(false, 2000);
        bg_music_fade_id = timer_callback_timeout(1000, function () {
            bg_music.set_volume(0.7);
        });
    }
}

function f_modding_back(): boolean {
    if (video_playing) {
        funkin_handle_nav(Nav.back);
        return true;
    }

    if (bg_music) bg_music.set_volume(1.0);
    layout.trigger_action(null, "outro");
    return false;
}

function f_buttons(player_id: number, buttons: GamepadButtons): void {
    if ((buttons & (GamepadButtons.A | GamepadButtons.X | GamepadButtons.START)) != GamepadButtons.NOTHING) {
        funkin_handle_nav(Nav.ok);
    } else if ((buttons & GamepadButtons.Y) != GamepadButtons.NOTHING) {
        funkin_handle_nav(Nav.back);
    } else if ((buttons & GamepadButtons.DALL_UP) != GamepadButtons.NOTHING) {
        funkin_handle_nav(Nav.up);
    } else if ((buttons & GamepadButtons.DALL_DOWN) != GamepadButtons.NOTHING) {
        funkin_handle_nav(Nav.down);
    }

}

function f_frame(elapsed: number): void {
    if (video_playing && video.has_ended()) {
        funkin_handle_nav(Nav.ok);
    }
}


function funkin_handle_nav(nav: Nav): void {
    if (video_playing) {
        if (nav == Nav.ok || nav == Nav.back) {
            video_playing = false;
            video.pause();
            layout.trigger_action("trailer", "fade_out");

            if (bg_music) {
                bg_music.play();
                bg_music.set_volume(0.7);
                bg_music.fade(true, 500);
            }
        }
        return;
    }

    if (nav == Nav.up) {
        if (!www_selected) {
            www_selected = true;
            layout.trigger_action(null, "select-www");
        }
        return;
    } else if (nav == Nav.down) {
        if (www_selected) {
            www_selected = false;
            layout.trigger_action(null, "select-trailer");
        }
        return;
    } else if (nav == Nav.back) {
        return;
    }

    if (!www_selected) {
        video.stop();// rewind
        video.play();

        if (bg_music) {
            timer_callback_cancel(bg_music_fade_id);
            bg_music.pause();
        }

        video_playing = true;
        layout.trigger_action("trailer", "fade_in");
        return;
    }

    Environment.open_www_link('https://www.kickstarter.com/projects/funkin/friday-night-funkin-the-full-ass-game/');
}

