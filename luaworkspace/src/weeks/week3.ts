let philly: Layout = null;
let girlfriend: Character;
let trainCooldown: number = 0;
let trainMoving: boolean = false;
let girlfriendAnimationTimeout: number = -1.0;
let song: SongPlayer = null;

function f_weekinit(freeplay_index: number): void {
    if (freeplay_index >= 0) return;
    let [, difficult,] = week_get_current_song_info();
    if (difficult == "HARD") week_unlockdirective_create("WEEK3_HARD", true, false, 1);
}

function f_beforeready(): void {
    trainCooldown = 0;
    philly = week_get_stage_layout();
    girlfriend = week_get_girlfriend();
    song = week_get_songplayer();

    // animate city lights
    if (philly) philly.trigger_trigger("lights_trigger");
}

function f_frame(elapsed: number) {
    if (girlfriendAnimationTimeout > 0 && song && song.get_timestamp() >= girlfriendAnimationTimeout) {
        girlfriendAnimationTimeout = -1;
        if (girlfriend) girlfriend.play_extra("hair", true);
    }
}

function f_beat(curBeat: number, since: number): void {
    if (!philly) return;
    if (philly.animation_is_completed("train-engine") != 1) return;

    if (trainMoving && girlfriendAnimationTimeout < 0) {
        trainMoving = false;
        if (girlfriend) girlfriend.play_idle();
    }

    trainCooldown++;

    if (curBeat % 8 != 4) return;
    if (trainCooldown <= 8) return;
    if (Math.random() >= 0.3) return;// 30% chance

    //
    // TODO: implement distractions disable
    //
    trainCooldown = math.random(-4, 0);
    trainMoving = true;

    // animate train
    if (philly) philly.trigger_action(null, "train_passes");

    // wait for the train
    girlfriendAnimationTimeout = song.get_timestamp() + 4800;
}

