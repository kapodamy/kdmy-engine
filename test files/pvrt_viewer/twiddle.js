"use strict";

const TWIDDLE_MASK_TABLE = [0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF];
const TWIDDLE_TABLE = new Uint32Array(1024);

/** 
 * @typedef TwiddleLookup
 * @type {object}
 * @property {number} mask
 * @property {number} shift
 * @property {number} reverse_mask
 * 
 * 
*/

function twiddle_lookup_init(/**@type{TwiddleLookup}*/twiddle, /**@type{number}*/width, /**@type{number}*/height) {
    let mask = Math.min(width, height) - 1;
    let bit = 1;
    let shift = 0;

    while (mask & bit) {
        bit <<= 1;
        shift++;
    }

    twiddle.shift = shift;
    twiddle.mask = mask;
    twiddle.reverse_mask = ~mask << shift;
}

function twiddle_index(/**@type{number}*/index) {
    let bit = 1;
    let acc = 0;
    let mask = 0;

    for (let i = 0; i < TWIDDLE_MASK_TABLE.length; i++) {
        let tmp = (index >>> bit) ^ acc;

        tmp &= TWIDDLE_MASK_TABLE[i];
        acc ^= tmp;
        index ^= tmp << bit;
        mask = (mask << 4) | 0x0f
        bit <<= 1;
    }

    return (acc << bit) | (index & mask);
}

function twiddle_coords(/**@type{TwiddleLookup}*/twiddle, /**@type{number}*/x, /**@type{number}*/y) {
    let twx = x & twiddle.mask;
    let twy = y & twiddle.mask;
    let twc = (x | y) << twiddle.shift;

    twx = x < TWIDDLE_TABLE.length ? TWIDDLE_TABLE[twx] : twiddle_index(twx);
    twy = y < TWIDDLE_TABLE.length ? TWIDDLE_TABLE[twy] : twiddle_index(twy);

    let pos = (twx << 1) | twy | (twc & twiddle.reverse_mask);

    return pos;
}

function twiddle_table_init() {
    for (let i = 0; i < TWIDDLE_TABLE.length; i++) {
        TWIDDLE_TABLE[i] = twiddle_index(i);
    }
}

