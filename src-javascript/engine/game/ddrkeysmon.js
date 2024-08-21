"use strict";

/** 
 * @typedef {object} DDRKey
 * @property {number} in_song_timestamp
 * @property {number} strum_id
 * @property {boolean} holding
 * @property {boolean} discard
 * @property {boolean} strum_invisible
 * @property {number} button
 * @property {GamepadKDY} gamepad
 */

/** 
 * @typedef {object} DDRKeysFIFO
 * @property {DDRKey[]} queue
 * @property {number} available
 */

/** 
 * @typedef {object} DDRKeymon
 * @property {DDRKeysFIFO} ddrkeys_fifo
 * @property {number} start_timestamp
 * @property {object[]} strum_binds
 * @property {number} strum_binds_size
 * @property {number} vbl_hnd
 * @property {GamepadKDY} gamepad
 */


const DDRKEYMON_FIFO_LENGTH = 64;


function ddrkeymon_init(/**@type {GamepadKDY}*/gamepad, strum_binds, strum_binds_size) {
    /** @type {DDRKeymon} */
    let ddrkeymon = {
        ddrkeys_fifo: {
            queue: new Array(DDRKEYMON_FIFO_LENGTH),
            available: 0,
        },

        start_timestamp: 0,

        strum_binds: malloc_for_array(strum_binds_size),
        strum_binds_size: strum_binds_size,

        vbl_hnd: -1,
        gamepad: gamepad
    }

    for (let i = 0; i < strum_binds_size; i++) {
        let count = math2d_bitcount(strum_binds[i]);
        ddrkeymon.strum_binds[i] = {
            is_visible: true,
            button_flags: strum_binds[i],
            button_count: count,
            button_array: malloc_for_array(count)
        };

        for (let j = 0, k = 0; k < count; j++) {
            let flag = strum_binds[i] & (0x01 << j);
            if (flag == 0x00) continue;
            ddrkeymon.strum_binds[i].button_array[k++] = flag;
        }
    }

    // (JS & C# only) populate queue
    clone_struct_as_array_items(
        ddrkeymon.ddrkeys_fifo.queue, DDRKEYMON_FIFO_LENGTH,
        { in_song_timestamp: -1, strum_id: -1, holding: 0, discard: 0, button: 0x00 }
    );

    return ddrkeymon;
}

function ddrkeymon_destroy(/**@type {DDRKeymon}*/ddrkeymon) {
    // note: "ddrkeys_fifo.queue" is a fixed array do not dispose

    // stop the monitor thread if running
    ddrkeymon_stop(ddrkeymon);

    for (let i = 0; i < ddrkeymon.strum_binds_size; i++) {
        ddrkeymon.strum_binds[i].button_array = undefined;
    }

    ddrkeymon.strum_binds = undefined;
    ddrkeymon = undefined;
}


function ddrkeymon_get_fifo(/**@type {DDRKeymon}*/ddrkeymon) {
    return ddrkeymon.ddrkeys_fifo;
}

function ddrkeymon_purge(/**@type {DDRKeysFIFO}*/ddrkeys_fifo) {
    ddrkeymon_purge2(ddrkeys_fifo, false);
}

function ddrkeymon_purge2(/**@type {DDRKeysFIFO}*/ddrkeys_fifo, /**@type {boolean}*/force_drop_first) {
    if (ddrkeys_fifo.available < 1) return;

    let available = 0;

    for (let i = force_drop_first ? 1 : 0; i < ddrkeys_fifo.available; i++) {
        if (ddrkeys_fifo.queue[i].discard) continue;

        if (i == available) {
            available++;
            continue;
        }

        clone_struct_to(ddrkeys_fifo.queue[i], ddrkeys_fifo.queue[available]);
        available++;

        // debugging only
        ddrkeys_fifo.queue[i].in_song_timestamp = -2.0;
        ddrkeys_fifo.queue[i].strum_id = -2;
        ddrkeys_fifo.queue[i].button = 0x00;
    }

    ddrkeys_fifo.available = available;
}

function ddrkeymon_clear(/**@type {DDRKeymon}*/ddrkeymon) {
    for (let i = 0; i < ddrkeymon.ddrkeys_fifo.available; i++)
        ddrkeymon.ddrkeys_fifo.queue[i].discard = true;
    ddrkeymon.ddrkeys_fifo.available = 0;
}

function ddrkeymon_start(/**@type {DDRKeymon}*/ddrkeymon, /**@type {number}*/offset_timestamp) {
    if (ddrkeymon.vbl_hnd >= 0) return;

    ddrkeymon.start_timestamp = timer_ms_gettime64() + offset_timestamp;
    ddrkeymon.vbl_hnd = vblank_handler_add(ddrkeymon_internal_vbl, ddrkeymon);

    if (DEBUG) {
        console.assert(ddrkeymon.vbl_hnd >= 0);
    }
}

function ddrkeymon_stop(/**@type {DDRKeymon}*/ddrkeymon) {
    if (ddrkeymon.vbl_hnd < 0) return;

    vblank_handler_remove(ddrkeymon.vbl_hnd);
    ddrkeymon.vbl_hnd = -1;

    ddrkeymon_clear(ddrkeymon);
}

function ddrkeymon_peek_timestamp(/**@type {DDRKeymon}*/ddrkeymon) {
    return timer_ms_gettime64() - ddrkeymon.start_timestamp;
}

function ddrkeymon_resync(/**@type {DDRKeymon}*/ddrkeymon, /**@type {number}*/offset_timestamp) {
    ddrkeymon.start_timestamp = timer_ms_gettime64() + offset_timestamp;
}

function ddrkeymon_internal_append_key(ddrkeymon, timestamp, strum_id, button_id, holding) {
    const queue = ddrkeymon.ddrkeys_fifo.queue;
    let available = ddrkeymon.ddrkeys_fifo.available;

    // offset correction
    timestamp -= ddrkeymon.start_timestamp;

    for (let i = available - 1; i >= 0; i--) {
        if (queue[i].discard) continue;
        if (queue[i].strum_id != strum_id) continue;
        if (queue[i].button != button_id) continue;

        // can not exists repeated events
        if (queue[i].holding != holding) break;

        // non-monotomous key event, this happens if the queue overflows
        // which under normal conditions this never should happen
        queue[i].in_song_timestamp = timestamp;
        return;
    }

    if (available >= DDRKEYMON_FIFO_LENGTH) {
        // imminent overflow, drop first key
        console.warn("ddrkeymon_append_key() queue overflow, ¿are you checking the queue?");
        ddrkeymon_purge2(ddrkeymon.ddrkeys_fifo, true);

        if (available >= DDRKEYMON_FIFO_LENGTH) {
            throw new Error("ddrkeymon_append_key() queue overflow");
        }
    }

    // the queue is healthy, append the key
    queue[available].in_song_timestamp = timestamp;
    queue[available].strum_id = strum_id;
    queue[available].holding = holding;
    queue[available].discard = false;
    queue[available].strum_invisible = !ddrkeymon.strum_binds[strum_id].is_visible;
    queue[available].button = button_id;

    ddrkeymon.ddrkeys_fifo.available = available + 1;
}

function ddrkeymon_internal_process_key(ddrkeymon, timestamp, old_buttons, new_buttons) {
    for (let i = 0; i < ddrkeymon.strum_binds_size; i++) {
        let buttons_flags = ddrkeymon.strum_binds[i].button_flags;
        let old_state = old_buttons & buttons_flags;
        let new_state = new_buttons & buttons_flags;

        if (old_state == new_state) continue;

        for (let j = 0; j < ddrkeymon.strum_binds[i].button_count; j++) {
            let button_flag = ddrkeymon.strum_binds[i].button_array[j];
            let button_state_old = old_state & button_flag;
            let button_state_new = new_state & button_flag;

            if (button_state_old == button_state_new) continue;

            ddrkeymon_internal_append_key(ddrkeymon, timestamp, i, button_flag, button_state_new);
        }
    }
}

function ddrkeymon_internal_vbl(code, /**@type {DDRKeymon}*/ddrkeymon) {
    void code;

    let timestamp = timer_ms_gettime64();
    let ret = gamepad_direct_state_update(ddrkeymon.gamepad);

    if (ret.previous == UINT32_MAX || ret.current == UINT32_MAX) {
        // invalid state or dettached controller
        return;
    }

    ddrkeymon_internal_process_key(ddrkeymon, timestamp, ret.previous, ret.current);
}

