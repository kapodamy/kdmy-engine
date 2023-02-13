//
// Timer LUA API, code is bundled with the engine and can not be modified without recompiling the engine
//

type TimerMapEntry = {
    timestamp: number;
    delay: number;
    is_interval: boolean;
    is_song_driven: boolean;
    callback: TimerCallback;
    args: any[];
    id: number;
};


let ______kdmyEngine_timer_ids: number = 0;
let ______kdmyEngine_timer_table = new LuaTable<number, TimerMapEntry>();
let ______kdmyEngine_timer_last_sngts = 0.0;
let ______kdmyEngine_timer_removes = 0;

function ______kdmyEngine_timer_add(intrv: boolean, sng: boolean, dly: number, cb: TimerCallback, args: any): number {
    if (typeof cb !== "function")
        error("The callback is not a function")
    if (typeof dly !== "number")
        error("The delay is not a number");
    if (typeof dly !== "number" || dly != dly || dly == Infinity || dly == -Infinity)
        error("The delay is not a finite number");

    let timestamp: number;

    if (sng) {
        if (!week_get_songplayer) {
            error("The script is not running in a week context, use timer_callback_* functions instead");
        }
        timestamp = ______kdmyEngine_timer_last_sngts;
    } else {
        timestamp = timer_ms_gettime();
    }

    timestamp += dly;

    let entry: TimerMapEntry = {
        timestamp: timestamp,
        delay: dly,
        is_interval: intrv,
        is_song_driven: sng,
        callback: cb,
        args: args,
        id: ______kdmyEngine_timer_ids
    };

    table.insert(______kdmyEngine_timer_table as any, entry);
    ______kdmyEngine_timer_ids++;

    return entry.id;
}
function ______kdmyEngine_timer_run(timestamp: number, song_driven_only: boolean) {
    if (song_driven_only) ______kdmyEngine_timer_last_sngts = timestamp;

    // keep referene until iteration is done
    let removes = ______kdmyEngine_timer_removes;

    for (let i = 1, length = ______kdmyEngine_timer_table.length(); i <= length; i++) {
        let entry = ______kdmyEngine_timer_table.get(i);
        if (entry && entry.is_song_driven == song_driven_only && timestamp >= entry.timestamp) {
            if (entry.is_interval) {
                entry.timestamp += entry.delay;
            } else {
                ______kdmyEngine_timer_table.delete(i);
            }

            let [result, error] = pcall(entry.callback, ...table.unpack(entry.args));
            if (!result) print(error);

            // check if any callback was removed, start again
            if (removes != ______kdmyEngine_timer_removes) {
                i = 1;
                length = ______kdmyEngine_timer_table.length();
                removes = ______kdmyEngine_timer_removes;
            }
        }
    }
}


function timer_callback_interval(delay: number, fn: TimerCallback, ...args: any[]) {
    return ______kdmyEngine_timer_add(true, false, delay, fn, args);
}
function timer_callback_timeout(delay: number, fn: TimerCallback, ...args: any[]) {
    return ______kdmyEngine_timer_add(false, false, delay, fn, args);
}
function timer_songcallback_interval(delay: number, fn: TimerCallback, ...args: any[]) {
    return ______kdmyEngine_timer_add(true, true, delay, fn, args);
}
function timer_songcallback_timeout(delay: number, fn: TimerCallback, ...args: any[]) {
    return ______kdmyEngine_timer_add(false, true, delay, fn, args);
}
function timer_callback_cancel(callback_id: number): boolean {
    if (callback_id < 0) return false;

    for (let i = 1, length = ______kdmyEngine_timer_table.length(); i <= length; i++) {
        let entry = ______kdmyEngine_timer_table.get(i);
        if (entry && entry.id == callback_id) {
            ______kdmyEngine_timer_removes++;
            ______kdmyEngine_timer_table.delete(i);
            return true;
        }
    }

    return false;
}
function timer_callback_cancel_all(): number {
    let length = ______kdmyEngine_timer_table.length();

    ______kdmyEngine_timer_table = new LuaTable<number, TimerMapEntry>();
    ______kdmyEngine_timer_removes++;

    collectgarbage();
    return length;
}

