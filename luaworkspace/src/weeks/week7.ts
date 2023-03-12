/*
funkay.png
main_menu.png
noteSplashes
FlxG.openURL('https://www.kickstarter.com/projects/funkin/friday-night-funkin-the-full-ass-game/');
*/

/*
<Camera name="cutscene-ugh">
<Camera name="cutscene-guns">
<Camera name="cutscene-stress">
<Camera name="round-start">
<Video name="video-ugh" src="./funkin/preload/music/ughCutscene.mp4">
<Video name="video-guns" src="./funkin/preload/music/gunsCutscene.mp4">
<Video name="video-stress" src="./funkin/preload/music/stressCutscene.mp4">
<Action name="bounce_fade">
<Action name="video_fade_in_and_play"> (per video)
*/

let freeplay_mode: boolean;
let layout: Layout;
let song: SongPlayer;

let tankAngle: number = math.random(-90, 45);
let tankSpeed: number = math.random(5, 7);
let tankGround: Sprite;
let tankX: number = 400;
let tankmanRun: TankmenBG[] = null;
let inCutscene: boolean = false;
let animationNotes: ChartNote[] = null;

let picospeaker: Character;
let tankman_captain: Character;
let picospeaker_last_played_anim: string;
let on_ugh_song: boolean;
let on_stress_song: boolean;

function f_weekinit(freeplay_index: number): void {
	freeplay_mode = freeplay_index >= 0;
}

function f_beforeready(from_retry: boolean): void {
	layout = week_get_stage_layout();
	songplayer = week_get_songplayer();
	tankGround = layout.get_sprite("tankGround");
	let [, , track_index] = week_get_current_song_info();

	if (tankmanRun == null) {
		let max_tankman_killed = layout.get_attached_value("max_tankman_killed") as number;

		tankmanRun = [];
		for (let i = 0; i < max_tankman_killed; i++) {
			let tempTankman: TankmenBG = new TankmenBG(i, 20, 500, true);
			tempTankman.strumTime = 10;
			tempTankman.resetShit(20, 600, true);
			tankmanRun.push(tempTankman);
		}
	} else {
		for (let tankman of tankmanRun) tankman.hide();
	}

	if (animationNotes == null) {
		loadMappedAnims();
	}

	on_ugh_song = track_index == 0;
	on_stress_song = track_index == 2;
	picospeaker = week_get_girlfriend();
	picospeaker_last_played_anim = "shoot 1";
	tankman_captain = week_get_character(0);
}

function f_frame(elapsed: number): void {
	if (on_stress_song) return;

	moveTank(elapsed / 1000.0);

	for (let tankman of tankmanRun) {
		if (tankman.inUse) tankman.update(/*elapsed*/);
	}

	let songPosition: number = songplayer.get_timestamp();

	scroll_picospeaker_chart(songPosition);
	play_picospeaker_anim(songPosition);
}


function moveTank(elapsed: number): void {
	if (!inCutscene) {
		let daAngleOffset: number = 1;
		tankAngle += elapsed * tankSpeed;
		let angle = tankAngle - 90 + 15;

		let x = tankX + Math.cos(math.rad((tankAngle * daAngleOffset) + 180)) * 1500;
		let y = 1300 + Math.sin(math.rad((tankAngle * daAngleOffset) + 180)) * 1100;

		tankGround.matrix_get_modifier().rotate = angle;
		tankGround.set_draw_location(x, y);
	}
}

function loadMappedAnims(): void {
	let swagshit: Chart = JSON.parse("funkin/data/stress/picospeaker.json");

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
	for (let i = 0; i < TankmenBG.animationNotes.length; i++) {
		if (TankmenBG.animationNotes[i][0] > songPosition) return;

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
	if (animationNotes.length > 0) {
		if (songPosition > animationNotes[0][0]) {
			print('played shoot anim' + animationNotes[0][1]);

			let shootAnim: number = 1;

			if (animationNotes[0][1] >= 2)
				shootAnim = 3;

			shootAnim += math.random(0, 1);

			picospeaker.play_extra(picospeaker_last_played_anim = 'shoot' + shootAnim, true);
			animationNotes.shift();
		}
	}

	if (layout.animation_is_completed("character_girlfriend")) {
		picospeaker.play_extra(picospeaker_last_played_anim + " (loop)", true);
	}
}




type Chart = { notes: ChartSection[] };
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
		[this.layout_width/*, this.layout_height*/] = layout.get_viewport_size();

		this.sprite.set_draw_location(x = x, y = y);
	}

	public /*function*/ resetShit(x: number, y: number, isGoingRight: boolean) {
		this.sprite.set_draw_location(x = x, y = y);
		this.goingRight = isGoingRight;
		this.endingOffset = math.random(50, 200);
		tankSpeed = math.random(0.6, 1);

		if (this.goingRight)
			this.sprite.flip_rendered_texture(true, null);

		this.shot_action_name = 'shot' + math.random(1, 2);
		this.curAnimIsRun = layout.trigger_action(this.sprite_name, 'run') > 0;
		this.curAnimIsShoot = false;
		this.sprite.set_visible(this.inUse = true);
	}

	/*override function*/ update(/*elapsed: number*/) {
		let songPosition = songplayer.get_timestamp() / 1000.0;
		let offset_x = 0, offset_y = 0;

		if (this.x >= this.layout_width * 1.2 || this.x <= this.layout_width * -0.5)
			this.sprite.set_visible(this.inUse = false);
		/*else
			this.sprite.set_visible(this.inUse = true);*/

		if (this.curAnimIsRun) {
			let endDirection: number = (this.layout_width * 0.74) + this.endingOffset;

			if (this.goingRight) {
				endDirection = (this.layout_width * 0.02) - this.endingOffset;

				this.x = (endDirection + (songPosition - this.strumTime) * tankSpeed);
			} else {
				this.x = (endDirection - (songPosition - this.strumTime) * tankSpeed);
			}
		}

		if (songPosition > this.strumTime) {
			// kill();
			this.curAnimIsShoot = layout.trigger_action(this.sprite_name, this.shot_action_name) > 0;

			if (this.goingRight) {
				offset_y = 200;
				offset_x = 300;
			}
		}

		if (this.curAnimIsShoot && layout.animation_is_completed(this.sprite_name)) {
			this.sprite.set_visible(false);
		}

		this.sprite.set_draw_location(this.x + offset_x, this.y + offset_y);
	}

	public hide() {
		this.sprite.set_visible(this.inUse = false);
	}
}

