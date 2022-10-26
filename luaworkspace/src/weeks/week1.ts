let curSong_bopeebo: boolean;
let boyfriend: Character;

function f_beforeready(from_retry: boolean): void {
    let [name, difficult, index] = week_get_current_track_info();

    curSong_bopeebo = index == 0;
    boyfriend = week_get_character(1);
}

function f_beat(curBeat: number, since: number): void {
    if (curBeat % 8 == 7 && curSong_bopeebo) {
        boyfriend.play_hey();
    }
}
