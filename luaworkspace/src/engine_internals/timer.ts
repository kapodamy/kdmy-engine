
function timer_ms_gettime() {
    return Math.random() * 10000;
}


type TimerCallback = (args?: any) => void;
type TimerMapEntry = {
    timestamp: number;
    delay: number;
    is_interval: boolean;
    callback: TimerCallback;
    args: any;
};


let ______kdmyEngine_timer_ids: number = 0;
let ______kdmyEngine_timer_table: LuaTable<number, TimerMapEntry> = new LuaTable<number, TimerMapEntry>();


function ______kdmyEngine_timer_add(is_interval: boolean, delay: number, callback: TimerCallback, args: any) {
    if (typeof (callback) !== "function") error("The callback is not a function");
    if (typeof (delay) !== "number") error("The delay is not a number");
    if (!Number.isFinite(delay)) error("The delay is not a finite number");

    let id = ______kdmyEngine_timer_ids;
    let timestamp = timer_ms_gettime();

    ______kdmyEngine_timer_table.set(id, { timestamp, delay, is_interval, callback, args });
    ______kdmyEngine_timer_ids++;

    return id;
}
function ______kdmyEngine_timer_run(timestamp: number) {
    for (const [id, entry] of ______kdmyEngine_timer_table) {
        if (timestamp >= entry.timestamp) {
            if (entry.is_interval) {
                entry.timestamp += entry.delay;
            } else {
                ______kdmyEngine_timer_table.delete(id);
            }

            let [result, error] = pcall(entry.callback, unpack(entry.args));
            if (!result) console.error(error);
        }
    }
}


function timer_callback_interval(delay: number, fn: TimerCallback, ...args[]: any) {
    return ______kdmyEngine_timer_add(true, delay, fn, args);
}
function timer_callback_timeout(delay: number, fn: TimerCallback, ....args[]: any) {
    return ______kdmyEngine_timer_add(false, delay, fn, args);
}
function timer_callback_cancel(callback_id: number): boolean {
    return ______kdmyEngine_timer_table.delete(callback_id);
}
function timer_callback_cancel_all(): number {
    let length = ______kdmyEngine_timer_table.length();
    ______kdmyEngine_timer_table = new LuaTable<number, TimerMapEntry>();
    collectgarbage();
    return length;
}

