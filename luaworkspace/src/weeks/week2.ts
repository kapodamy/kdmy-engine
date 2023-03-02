let lightningStrikeBeat: number = 24;


// called by the engine
function f_beforeready(from_restart: boolean): void {
    lightningStrikeBeat = 24;

    if (from_restart) return;

    let [song_name, song_difficult, song_index] = week_get_current_song_info();

    print(`song: name=${song_name} difficult=${song_difficult} number=${song_index}`);
}

// called by the engine
function f_beat(curBeat: number, since: number): void {
    if (math.random(0.0, 1.0) < 0.1) return;// 10% chance

    if (curBeat <= lightningStrikeBeat) return;
    let random_boolean = Math.random() < 0.5;

    lightningStrikeBeat = curBeat + math.random(8, 24);// next lightning strike

    let stage = week_get_stage_layout();
    let girlfriend = week_get_girlfriend();
    let boyfriend = week_get_character(1);

    if (stage) {
        stage.trigger_any('lightstrike');
        stage.trigger_action(null, random_boolean ? 'thunder_1' : 'thunder_2');
    }

    girlfriend.play_extra('fear', false);
    boyfriend.play_extra('shaking', false);
}

