let boyfriend: Character, girlfriend: Character;

function f_beforeready(from_retry: boolean): void {
    girlfriend = week_get_character(0);
    boyfriend = week_get_character(1);
}

function f_beat(curBeat: number, since: number): void {
    if (curBeat % 16 == 15 && curBeat > 16 && curBeat < 48) {
        boyfriend.play_hey();
        girlfriend.play_hey();
    }
}
