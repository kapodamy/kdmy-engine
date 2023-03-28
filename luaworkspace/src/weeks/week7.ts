const SKIP_CUTSCENE_BUTTONS: GamepadButtons =
	GamepadButtons.START | GamepadButtons.BACK | GamepadButtons.A | GamepadButtons.B;

let freeplay_mode: boolean;
let layout: Layout;
let songplayer: SongPlayer;

let tankAngle: number = math.random(-90, 45);
let tankSpeed: number = math.random(5, 7);
let tankGround: Sprite;
let tankGroundModifier: Modifier;
let tankX: number = 400;
let tankmanRun: TankmenBG[] = null;
let inCutscene: boolean = false;
let inPauseMenu: boolean = false;
let animationNotes: ChartNote[] = null;
let animationNotesIndexPico: number;
let animationNotesIndexTankman: number;

let picospeaker: Character;
let tankman_captain: Character;
let is_ugh_song: boolean;
let is_guns_song: boolean;
let is_stress_song: boolean;
let no_distractions: boolean = !Settings.distractionsEnabled;
let skip_cutscene: boolean;
let last_jeff_voiceline: SoundPlayer = null;

function f_weekinit(freeplay_index: number): void {
	freeplay_mode = freeplay_index >= 0;
	week_override_common_folder("./kickstater_mainmenu_common_assets");

	if (!freeplay_mode) {
		week_enable_credits_on_completed();
		week_unlockdirective_create("FNF_WEEK7_COMPLETED", false, true, 9942069);
	}
}

function f_beforeready(from_retry: boolean): void {
	layout = week_get_stage_layout();
	songplayer = week_get_songplayer();
	tankGround = layout.get_sprite("tankGround");
	tankGroundModifier = tankGround.matrix_get_modifier();
	let [, , track_index] = week_get_current_song_info();

	tankGroundModifier.rotatePivotEnabled = true;
	tankGroundModifier.rotatePivotU = 0.5;
	tankGroundModifier.rotatePivotV = 0.5;

	animationNotesIndexPico = 0;
	animationNotesIndexTankman = 0;

	if (tankmanRun == null) {
		let max_tankman_killed: number = layout.get_attached_value("max_tankman_killed") as number;

		tankmanRun = [];
		for (let i = 0; i < max_tankman_killed; i++) {
			let tempTankman: TankmenBG = new TankmenBG(i, 20, 500, true);
			tempTankman.strumTime = 10;
			tempTankman.resetShit(20, 600, true);
			tankmanRun.push(tempTankman);
			tempTankman.hide();
		}
	} else {
		for (let tankman of tankmanRun) tankman.hide();
	}

	if (animationNotes == null) {
		animationNotes = [];
		loadMappedAnims();
	}

	is_ugh_song = track_index == 0;
	is_guns_song = track_index == 1;
	is_stress_song = track_index == 2;
	picospeaker = week_get_girlfriend();
	tankman_captain = week_get_character(0);

	if (!freeplay_mode && !from_retry) {
		if (is_ugh_song)
			play_cutscene("ugh");
		else if (is_guns_song)
			play_cutscene("guns");
	}
}

function f_frame(elapsed: number): void {
	if (inPauseMenu || no_distractions) return;

	moveTank(elapsed / 1000.0);

	if (!is_stress_song) return;

	for (let tankman of tankmanRun) {
		if (tankman.inUse) tankman.update(/*elapsed / 1000.0*/);
	}

	let songPosition: number = songplayer.get_timestamp();

	scroll_picospeaker_chart(songPosition);
	play_picospeaker_anim(songPosition);
}

function f_pause(pause_or_resume: boolean): void {
	inPauseMenu = pause_or_resume;
}

function f_roundend(loose: boolean): void {
	if (!loose && is_guns_song) play_cutscene("stress");
}

function f_gameoverloop(): void {
	if (!Settings.distractionsEnabled) return;

	if (last_jeff_voiceline != null) last_jeff_voiceline.destroy();

	let index = math.random(1, 25);
	last_jeff_voiceline = SoundPlayer.init(`./funkin/week7/sounds/jeffGameover/jeffGameover-${index}.ogg`);

	if (last_jeff_voiceline != null) last_jeff_voiceline.play();
}

function f_gameoverdecision(retry_or_giveup: boolean, changed_difficult: string): void {
	if (last_jeff_voiceline != null && last_jeff_voiceline.is_playing()) {
		last_jeff_voiceline.fade(false, 500);
	}
}

function f_buttons(player_id: number, buttons: number): void {
	if (!inCutscene) return;
	if ((buttons & SKIP_CUTSCENE_BUTTONS) != GamepadButtons.NOTHING) {
		skip_cutscene = true;
	}
}

function f_input_keyboard(key: Keys, scancode: number, is_pressed: boolean, mods: ModKeys): void {
	if (!inCutscene || !is_pressed) return;
	if (key == Keys.KP_Enter || key == Keys.Enter || key == Keys.Backspace || key == Keys.Space) {
		skip_cutscene = true;
	}
}


function moveTank(elapsed: number): void {
	if (!inCutscene) {
		let daAngleOffset: number = 1;
		tankAngle += elapsed * tankSpeed;
		let angle = tankAngle - 90 + 15;

		let x = tankX + Math.cos(math.rad((tankAngle * daAngleOffset) + 180)) * 1500;
		let y = 1300 + Math.sin(math.rad((tankAngle * daAngleOffset) + 180)) * 1100;

		tankGroundModifier.rotate = math.rad(angle);
		tankGround.set_draw_location(x, y);
	}
}

function loadMappedAnims(): void {
	let swagshit: ChartSong = JSON.parse_from_file("funkin/preload/data/stress/picospeaker.json").song;

	let notes = swagshit.notes;

	for (let section of notes) {
		for (let idk of section.sectionNotes) {
			animationNotes.push(idk);
		}
	}

	TankmenBG.animationNotes = animationNotes;

	animationNotes.sort((val1, val2) => sortAnims(val1, val2));
}

function sortAnims(val1: ChartNote, val2: ChartNote): number {
	return val1[0] - val2[0];
}

function tankmanRun_recycle(): TankmenBG {
	// find an unused sprite
	for (let tankman of tankmanRun) {
		if (!tankman.inUse) {
			return tankman;
		}
	}

	return null;
}

function scroll_picospeaker_chart(songPosition: number): void {
	songPosition += 4000;

	for (let i = animationNotesIndexTankman; i < TankmenBG.animationNotes.length; i++) {
		if (TankmenBG.animationNotes[i][0] > songPosition) return;
		animationNotesIndexTankman++;

		if (math2d_random_boolean(16)) {
			let tankman: TankmenBG = tankmanRun_recycle();
			if (tankman == null) {
				// "max_tankman_killed" value is insufficient
				return;
			}

			// new TankmenBG(500, 200 + FlxG.random.int(50, 100), TankmenBG.animationNotes[i][1] < 2);
			tankman.strumTime = TankmenBG.animationNotes[i][0];
			tankman.resetShit(500, 200 + math.random(50, 100), TankmenBG.animationNotes[i][1] < 2);
		}
	}
}

function play_picospeaker_anim(songPosition: number): void {
	if (animationNotesIndexPico < animationNotes.length) {
		if (songPosition > animationNotes[animationNotesIndexPico][0]) {

			let shootAnim: number = 1;

			if (animationNotes[animationNotesIndexPico][1] >= 2)
				shootAnim = 3;

			shootAnim += math.random(0, 1);

			picospeaker.play_extra('shoot' + shootAnim, true);
			animationNotesIndexPico++;
		}
	}
}

function play_cutscene(what: string) {
	// guess the cutscene
	let video_name: string = "video-" + what;
	let camera_name: string = "cutscene-" + what;

	week_set_halt(true);
	week_ui_set_visibility(false);
	inCutscene = true;
	skip_cutscene = false;

	let video: VideoPlayer = layout.get_videoplayer(video_name);
	layout.trigger_action(video_name, "video_fade_in_and_play");

	layout.trigger_camera(camera_name);

	if (what == "ugh") {
		wait_for_video_end(video, true);
	} else {
		wait_for_camera(video, what == "guns");
	}
}

function wait_for_camera(video: VideoPlayer, bounce_fade: boolean): void {
	let id = timer_callback_interval(1, function () {
		if (!layout.camera_is_completed()) return;

		timer_callback_cancel(id);
		wait_for_video_end(video, bounce_fade);
	});
}

function wait_for_video_end(video: VideoPlayer, bounce_fade: boolean): void {
	let id = timer_callback_interval(1, function () {
		if (!video.has_ended() && !skip_cutscene) return;

		if (skip_cutscene) video.pause();

		timer_callback_cancel(id);
		inCutscene = false;

		if (bounce_fade) {
			week_ui_set_visibility(true);
			do_bounce_fade(video);
		} else {
			do_fade_out();
		}
	});
}

function do_bounce_fade(video: VideoPlayer): void {
	layout.trigger_action(null, "bounce_fade");

	let id = timer_callback_interval(1, function () {
		if (layout.animation_is_completed("fade-effect") < 1) return;

		video.get_sprite().set_visible(false);
		week_set_halt(false);// resumes engine execution
		timer_callback_cancel(id);
	});
}

function do_fade_out(): void {
	layout.trigger_action(null, "fade_out");

	let id = timer_callback_interval(1, function () {
		if (layout.animation_is_completed("fade-effect") < 1) return;

		week_set_halt(false);// resumes engine execution

		timer_callback_cancel(id);
	});
}

type Chart = { song: ChartSong };
type ChartSong = { notes: ChartSection[] };
type ChartSection = { sectionNotes: ChartNote[] };
type ChartNote = [number, number, number];

class TankmenBG {
	public static animationNotes: ChartNote[] = null;

	public strumTime: number = 0;
	public goingRight: boolean = false;
	public tankSpeed: number = 0.7;

	public endingOffset: number;

	public inUse: boolean = false;

	private sprite: Sprite;
	private sprite_name: string;
	private x: number;
	private y: number;
	private curAnimIsRun: boolean;
	private curAnimIsShoot: boolean;
	private readonly layout_width: number;
	//private readonly layout_height: number;
	private shot_action_name: string;

	public /*function*/ constructor(spr_index: number, x: number, y: number, isGoingRight: boolean) {
		this.sprite_name = `tankmankilled${spr_index}`;
		this.sprite = layout.get_sprite(this.sprite_name);
		this.sprite.flip_rendered_texture_enable_correction(true);
		this.layout_width = layout.get_viewport_size()[0];

		this.sprite.set_draw_location(this.x = x, this.y = y);
	}

	public /*function*/ resetShit(x: number, y: number, isGoingRight: boolean) {
		this.sprite.set_draw_location(this.x = x, this.y = y);
		this.goingRight = isGoingRight;
		this.endingOffset = math.random(50, 200);
		this.tankSpeed = math2d_random(0.6, 1);

		this.sprite.flip_rendered_texture(isGoingRight, null);

		this.shot_action_name = 'shot' + math.random(1, 2);
		this.curAnimIsRun = layout.trigger_action(this.sprite_name, 'run') > 0;
		this.curAnimIsShoot = false;
		this.sprite.set_visible(false);
		this.inUse = true;
	}

	/*override function*/ update(/*elapsed: number*/) {
		let songPosition = songplayer.get_timestamp();
		let offset_x = 0, offset_y = 0;

		if (this.x >= this.layout_width * 1.2 || this.x <= this.layout_width * -0.5)
			this.sprite.set_visible(false);
		else
			this.sprite.set_visible(true);

		if (this.curAnimIsRun) {
			let endDirection: number = (this.layout_width * 0.74) + this.endingOffset;

			if (this.goingRight) {
				endDirection = (this.layout_width * 0.02) - this.endingOffset;

				this.x = (endDirection + (songPosition - this.strumTime) * this.tankSpeed);
			} else {
				this.x = (endDirection - (songPosition - this.strumTime) * this.tankSpeed);
			}
			this.sprite.set_draw_location(this.x, this.y);
		}

		if (songPosition > this.strumTime) {
			// kill();
			this.curAnimIsShoot = layout.trigger_action(this.sprite_name, this.shot_action_name) > 0;
			this.curAnimIsRun = false;
			this.strumTime = Infinity;// obligatory

			if (this.goingRight) {
				offset_x = -300;
			}

			this.sprite.set_draw_location(this.x + offset_x, this.y + offset_y);
		}

		if (this.curAnimIsShoot && layout.animation_is_completed(this.sprite_name) > 0) {
			this.sprite.set_visible(this.inUse = false);
		}

	}

	public hide() {
		this.sprite.set_visible(this.inUse = false);
	}
}

