
const CREDITS_BUTTONS = GamepadButtons.START | GamepadButtons.A | GamepadButtons.B | GamepadButtons.X | GamepadButtons.Y;

// adquire intro texts
let introText: string = fs_readfile("/assets/common/introText.txt");
let layout: Layout = modding_get_layout();
let introTextHolder: TextSprite = layout.get_textsprite("intro-texts");
let introTextGroupModifier: Modifier = layout.get_group_modifier("intro-texts-group");


let base_scrolled_to: number = layout.get_attached_value("scripted_base_scrolled_to") as number;
let base_scrolled_duration: number = layout.get_attached_value("scripted_base_scrolled_duration") as number;
let thanks_scroll_duration = layout.get_attached_value("scripted_thanks_scroll_duration") as number;
let thanks_duration = layout.get_attached_value("scripted_thanks_duration") as number;
let shoot_duration = layout.get_attached_value("scripted_shoot_duration") as number;

let do_scroll: boolean = true;
let check_outro: boolean = false;
let wait_duration: number = base_scrolled_duration;

let scroll_duration: number = 0;
let scroll_distance: number = 0;
let scroll_elapsed: number = 0;

if (introText != null && introTextHolder != null) {
	// parse
	let introTextList: string[] = introText.split("\n");
	introText = "";
	for (let i = 0; i < introTextList.length; i++) {
		introText += introTextList[i].replaceAll("--", "\r\n") + "\r\n\r\n";
	}

	// set
	introTextHolder.set_text(introText);

	//calc height
	let [, holder_height] = introTextHolder.get_draw_size();

	// calc scrolling
	scroll_distance = holder_height;
	scroll_duration = (base_scrolled_duration * holder_height) / base_scrolled_to;
} else {
	scroll_distance = 0;
	scroll_duration = 0;
}

function f_frame(elapsed: number) {
	if (check_outro) {
		if (layout.animation_is_completed("transition_effect") > 0) {
			// everything is done, end script execution
			modding_exit();
		}
		return;
	}

	if (!do_scroll) return;

	if (wait_duration > 0) {
		wait_duration -= elapsed;
		return;
	}

	let current_distance = math2d_lerp(0, scroll_distance, scroll_elapsed / scroll_duration);
	scroll_elapsed += elapsed;

	if (scroll_elapsed >= scroll_duration || introTextGroupModifier == null) {
		do_scroll = false;
		current_distance = scroll_distance;
		layout.trigger_any("show-thanks");
		timer_callback_timeout(thanks_scroll_duration, thanks_callback);
	}

	if (introTextGroupModifier) introTextGroupModifier.translateY = -current_distance;
}

function f_buttons(player_id: number, buttons: number): void {
	if (check_outro) return;

	if (buttons & CREDITS_BUTTONS) {
		layout.trigger_any("outro");
		check_outro = true;
	}
}


function thanks_callback() {
	let bg_music = layout.get_soundplayer("bg-music");
	if (bg_music) bg_music.fade(false, thanks_duration);
	timer_callback_timeout(thanks_duration, shoot_callback);
}

function shoot_callback() {
	layout.trigger_any("shoot");

	timer_callback_timeout(shoot_duration, function () {
		layout.trigger_any("outro");
		check_outro = true;
	});
}

