"use strict";

const FUNKIN_DIFFICULT_EASY = "EASY";
const FUNKIN_DIFFICULT_NORMAL = "NORMAL";
const FUNKIN_DIFFICULT_HARD = "HARD";

const FUNKINSAVE_MAX_STRING_SIZE = 32;
const FUNKINSAVE_SAVEDATA_VERSION = 0x02;
const FUNKINSAVE_VMS_HEADER_OFFSET_CRC16 = 0x0046;
const FUNKINSAVE_VMS_HEADER_OFFSET_LENGTH = 0x0048;
const FUNKINSAVE_VMS_HEADER = new Uint8Array([
    // space-padded short description (FnF savedata    )
    0x46, 0x6E, 0x46, 0x20, 0x73, 0x61, 0x76, 0x65, 0x64, 0x61, 0x74, 0x61, 0x20, 0x20, 0x20, 0x20,
    // space-padded long description (Friday Night Funkin' kdmy-engine)
    0x46, 0x72, 0x69, 0x64, 0x61, 0x79, 0x20, 0x4E, 0x69, 0x67, 0x68, 0x74, 0x20, 0x46, 0x75, 0x6E,
    0x6B, 0x69, 0x6E, 0x27, 0x20, 0x6B, 0x64, 0x6D, 0x79, 0x2D, 0x65, 0x6E, 0x67, 0x69, 0x6E, 0x65,
    // null-padded application id (ninjamuffin99FNF)
    0x6E, 0x69, 0x6E, 0x6A, 0x61, 0x6D, 0x75, 0x66, 0x66, 0x69, 0x6E, 0x39, 0x39, 0x46, 0x4E, 0x46,
    // number of icons + icon animation speed + eyecatch type + CRC16 + payload size
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // reserved (unused space, all zeros)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // color palette (argb4444)
    0x00, 0x00, 0x83, 0xF3, 0x6C, 0xF1, 0x11, 0xFA, 0xB2, 0xF3, 0x44, 0xFE, 0x9E, 0xF0, 0x25, 0xF6,
    0x18, 0xFB, 0x22, 0xF2, 0x00, 0xF0, 0x14, 0xFF, 0xC8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    // bitmap (32x32 4bit)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x11, 0x41, 0x10, 0x00, 0x00, 0x35, 0x33, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x01, 0x14, 0x44, 0x11, 0x03, 0x33, 0x35, 0x53, 0x30,
    0x00, 0x00, 0x00, 0x22, 0x26, 0x22, 0x20, 0x01, 0x44, 0x44, 0x41, 0x03, 0x33, 0x55, 0x55, 0x33,
    0x00, 0x00, 0x00, 0x26, 0x66, 0x66, 0x20, 0x01, 0x11, 0x41, 0x11, 0x03, 0x33, 0x35, 0x53, 0x30,
    0x00, 0x00, 0x00, 0x22, 0x66, 0x62, 0x20, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x35, 0x33, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x26, 0x22, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x07, 0x77, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x77, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0x78, 0x87, 0x77, 0x70, 0x00, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xA0,
    0x77, 0x88, 0x88, 0x77, 0x70, 0x00, 0x95, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xAA,
    0x07, 0x78, 0x87, 0x77, 0x70, 0x0A, 0xAB, 0xAB, 0xBA, 0x2B, 0xBB, 0xBB, 0x55, 0x55, 0x53, 0x3A,
    0x00, 0x77, 0x87, 0x00, 0x00, 0xA6, 0xA2, 0x6A, 0xB6, 0x2A, 0xBB, 0xBB, 0xB5, 0x55, 0x33, 0x3A,
    0x00, 0x07, 0x77, 0x00, 0x0A, 0x66, 0x26, 0x62, 0xA2, 0x62, 0xAB, 0xBB, 0xBB, 0x53, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0x00, 0xA6, 0x66, 0x66, 0x66, 0x26, 0x66, 0x2A, 0xBB, 0xBB, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0x0A, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x22, 0x9B, 0xBB, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0xA6, 0x62, 0x22, 0xA6, 0x66, 0x66, 0x66, 0x62, 0xAB, 0xBB, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x0A, 0x22, 0x20, 0x0A, 0x66, 0x66, 0x66, 0x66, 0x66, 0x29, 0xBB, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA6, 0x66, 0x66, 0xA6, 0x66, 0x66, 0x2A, 0x9B, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0x00, 0x9A, 0x66, 0x66, 0x66, 0x6A, 0x66, 0x66, 0xA6, 0x9B, 0x33, 0x33, 0x3A,
    0x00, 0x00, 0x00, 0x0A, 0x66, 0x66, 0x66, 0x99, 0x99, 0xA9, 0x9A, 0x99, 0xC9, 0xBA, 0xAA, 0xAA,
    0x00, 0x00, 0x00, 0xA2, 0x22, 0x22, 0x69, 0xCC, 0xCA, 0xCA, 0xAC, 0xAC, 0xCC, 0x26, 0x66, 0x62,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xC2, 0x66, 0x62,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xCC, 0x26, 0x2A,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCC, 0xCA, 0xCC, 0xCC, 0xAC, 0xCC, 0xCC, 0xC2, 0xA0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCA, 0xCC, 0xC2, 0xA0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xAC, 0xCC, 0xCA, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCC, 0xCC, 0xAA, 0xAA, 0xA9, 0xCC, 0xCC, 0xA0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00
]);

const FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK = 0x00;
const FUNKINSAVE_MAX_INDEXED_NAME = 0xFFFF - 1;



/**
 * This hold all information loaded from the savefile stored in the Sega Dreamcast VMU
 */
const funkinsave = {
    maple_port: 0,
    maple_unit: 1,

    settings: linkedlist_init(),
    directives: linkedlist_init(),
    weeks_names: linkedlist_init(),
    difficulty_names: linkedlist_init(),
    progress: linkedlist_init(),
    storages: linkedlist_init(),
    freeplay_progress: linkedlist_init(),

    last_played_week_index: -1,
    last_played_difficulty_index: -1
};


async function funkinsave_read_from_vmu() {
    const ENDIANESS = true;

    // step 1: check if the VMU is inserted at the given port+slot
    if (funkinsave_is_vmu_missing()) return 1;

    // step 2: open savedata file
    let vmu_path = funkinsave_internal_get_vmu_path(funkinsave.maple_port, funkinsave.maple_unit);
    let vms_file = await fs_open(vmu_path, O_RDONLY);

    if (vms_file == -1) {
        console.info("funkinsave_read_from_vmu() no savedata present in " + vmu_path);
        vmu_path = undefined;
        return 2;
    }

    // step 3:  read the whole VMS file
    let readed = 0;
    let total = fs_total(vms_file);
    let vms_data = new ArrayBuffer(total);

    if (total < 0) {
        // something bad happen
        console.error("fs_total() on " + vmu_path + " failed");
        vmu_path = undefined;
        vms_data = undefined;
        return 3;
    }

    while (readed < total) {
        let read = fs_read(vms_file, new Uint8Array(vms_data, readed), total);
        if (read < 0) {
            // something bad happen
            vmu_path = undefined;
            vms_data = undefined;
            await fs_close(vms_file);
            return 3;
        }
        if (read == 0) break;// EOF was reached
        readed += read;
    }

    await fs_close(vms_file);

    if (readed != total) {
        vmu_path = undefined;
        vms_data = undefined;
        console.error("failed to read the VMS file at " + vmu_path);
        return 3;
    }

    // step 4: parse VMS header using vmu_pkg_parse() function from KallistiOS (required for CRC16 check)
    const vmu_pkg = new vmu_pkg_t();

    if (vmu_pkg_parse(vms_data, vmu_pkg) != 0) {
        console.warn("vmu_pkg_parse() returned failed on: " + vmu_path);

        vmu_path = undefined;
        vms_data = undefined;
        return 4;
    }

    // step 5: parse savedata
    let offset = 0;
    let savedata = new DataView(vmu_pkg.data.buffer, vmu_pkg.data.byteOffset, vmu_pkg.data_len);
    let version = savedata.getUint8(0); offset += 1;
    offset++;// reserved space

    // version check
    if (version > FUNKINSAVE_SAVEDATA_VERSION) {
        vmu_path = undefined;
        vms_data = undefined;
        return 5;
    }

    // clear in-memory savedata
    vmu_path = undefined;
    funkinsave_internal_clear_savedata();

    let settings_count = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let directives_count = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let week_names_table_size = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let difficulty_names_table_size = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let progress_count = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let storages_count = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let last_played_week_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
    let last_played_difficulty_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;

    for (let i = 0; i < settings_count; i++) {
        let id = savedata.getUint16(offset, ENDIANESS); offset += 2;
        let is_integer = funkinsave_internal_is_setting_integer(id);
        let value;

        if (is_integer) value = Number(savedata.getBigInt64(offset, ENDIANESS));
        else value = savedata.getFloat64(offset, ENDIANESS);
        offset += 8;

        linkedlist_add_item(funkinsave.settings, { id, value });
    }

    for (let i = 0; i < directives_count; i++) {
        let type = savedata.getUint8(offset); offset++;
        let name = []; offset += funkinsave_internal_read_string(savedata, offset, name);
        let value = savedata.getFloat64(offset, ENDIANESS); offset += 8;

        console.assert(name && name.length > 0);
        linkedlist_add_item(funkinsave.directives, { type, value, name: name[0] });
    }

    for (let i = 0; i < week_names_table_size; i++) {
        let name = []; offset += funkinsave_internal_read_string(savedata, offset, name);

        console.assert(name && name.length > 0);
        linkedlist_add_item(funkinsave.weeks_names, name[0]);
    }

    for (let i = 0; i < difficulty_names_table_size; i++) {
        let name = []; offset += funkinsave_internal_read_string(savedata, offset, name);

        console.assert(name && name.length > 0);
        linkedlist_add_item(funkinsave.difficulty_names, name[0]);
    }

    for (let i = 0; i < progress_count; i++) {
        let week_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
        let difficulty_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
        let score = Number(savedata.getBigInt64(offset, ENDIANESS)); offset += 8;

        console.assert(week_name_index_in_table < week_names_table_size);
        console.assert(difficulty_name_index_in_table < difficulty_names_table_size);

        linkedlist_add_item(funkinsave.progress, {
            week_id: week_name_index_in_table,
            difficulty_id: difficulty_name_index_in_table,
            score
        });
    }

    for (let i = 0; i < storages_count; i++) {
        let week_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
        let name = []; offset += funkinsave_internal_read_string(savedata, offset, name);
        let data_size = savedata.getUint32(offset, ENDIANESS); offset += 4;

        console.assert(week_name_index_in_table < week_names_table_size);
        console.assert(data_size > 0);
        console.assert((offset + data_size) <= total);

        let data = new ArrayBuffer(data_size);
        (new Uint8Array(data)).set(new Uint8Array(savedata.buffer, savedata.byteOffset + offset, data_size), 0);
        offset += data_size;

        linkedlist_add_item(funkinsave.storages, {
            week_id: week_name_index_in_table,
            name: name[0],
            data_size,
            data
        });
    }

    if (version >= 2) {
        let freeplay_progress_count = savedata.getUint32(offset, ENDIANESS); offset += 4;

        for (let i = 0; i < freeplay_progress_count; i++) {
            let week_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
            let difficulty_name_index_in_table = savedata.getUint16(offset, ENDIANESS); offset += 2;
            let score = Number(savedata.getBigInt64(offset, ENDIANESS)); offset += 8;
            let song_name = []; offset += funkinsave_internal_read_string(savedata, offset, song_name);

            console.assert(week_name_index_in_table < week_names_table_size);
            console.assert(difficulty_name_index_in_table < difficulty_names_table_size);

            linkedlist_add_item(funkinsave.freeplay_progress, {
                week_id: week_name_index_in_table,
                difficulty_id: difficulty_name_index_in_table,
                song_name: song_name[0],
                score: score
            });
        }
    }

    if (last_played_week_name_index_in_table == 0xFFFF)
        funkinsave.last_played_week_index = -1;
    else
        funkinsave.last_played_week_index = last_played_week_name_index_in_table;

    if (last_played_difficulty_name_index_in_table == 0xFFFF)
        funkinsave.last_played_difficulty_index = -1;
    else
        funkinsave.last_played_difficulty_index = last_played_difficulty_name_index_in_table;

    // done
    savedata = undefined;
    return 0;
}

async function funkinsave_write_to_vmu() {
    const ENDIANESS = true;

    /*
        savedata layout:
                {
                    version,                        // 1 byte
                    reserved,                       // 1 byte
                    settings_count,                 // 2 bytes
                    directives_count,               // 2 bytes
                    progress_count,                 // 2 bytes
                    storages_count,                 // 2 bytes
                    week_names_table_size,          // 2 bytes
                    difficulty_names_table_size,    // 2 bytes
                    settings: [{id, value}, ...],                       // 10 bytes
                    directives: [{type, name, value}, ...],             // 1 + 8 + n bytes
                    week_names_table: [week_name, ...],                 // n bytes
                    difficulty_names_table: [difficulty_name, ...],     // n bytes
                    progress: [
                        {
                            week_name_index_in_table,// 2 bytes (index to week_names_table array)
                            difficulty_name_index_in_table,// 2 bytes (index to difficulty_names_table array)
                            score,// 8 bytes
                        },
                        ...
                    ],
                    storage: [
                        {
                            week_name_index_in_table,// 2 bytes
                            data_size,// 4 bytes
                            data,// n bytes
                        },
                        ...
                    ],
                    freeplay_progress_count, // 4 bytes
                    freeplay_progress: [
                        {
                            week_name_index_in_table,// 2 bytes (index to week_names_table array)
                            difficulty_name_index_in_table,// 2 bytes (index to difficulty_names_table array)
                            song_name,// zero-terminated string (max chars 32)
                            score,// 8 bytes
                        }
                    ]
                }
    */

    let offset = 0;
    let length = 0;
    let week_names_table_size = 0;
    let difficulty_names_table_size = 0;

    let last_played_week_index = 0xFFFF;
    let last_played_difficulty_index = 0xFFFF;

    if (funkinsave.last_played_week_index > 0)
        last_played_week_index = funkinsave.last_played_week_index;
    if (funkinsave.last_played_difficulty_index >= 0)
        last_played_difficulty_index = funkinsave.last_played_difficulty_index;

    if (funkinsave_is_vmu_missing()) return 1;

    // step 1: count amount bytes needed
    length += linkedlist_count(funkinsave.settings) * (/*sizeof(uint16))*/2 +/*sizeof(int64|double))*/8);
    length += linkedlist_count(funkinsave.progress) * (/*sizeof(uint16))*/2 +/*sizeof(uint16))*/2 + /*sizeof(int64))*/8);

    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        let string_length = funkinsave_internal_string_bytelength(directive.name);
        length += string_length + /*sizeof(int64_t)*/8 + /*sizeof(uint8_t)*/1;
    }

    for (let week_name of linkedlist_iterate4(funkinsave.weeks_names)) {
        length += funkinsave_internal_string_bytelength(week_name);
        week_names_table_size++;
    }

    for (let difficulty_name of linkedlist_iterate4(funkinsave.difficulty_names)) {
        length += funkinsave_internal_string_bytelength(difficulty_name);
        difficulty_names_table_size++;
    }

    for (let week_storage of linkedlist_iterate4(funkinsave.storages)) {
        length += funkinsave_internal_string_bytelength(week_storage.name);
        length += week_storage.data_size + /*sizeof(int32)*/4 + /*sizeof(int16)*/2;
    }

    for (let freeplay_progress of linkedlist_iterate4(funkinsave.freeplay_progress)) {
        length += funkinsave_internal_string_bytelength(freeplay_progress.song_name);
        length += /*sizeof(uint16))*/2 +/*sizeof(uint16))*/2 + /*sizeof(int64))*/8;
    }

    length += 10;//version + reserved + settings_count + directives_count + progress_count + storages_count
    length += 4;// last_played_week_name_index_in_table + last_played_difficulty_name_index_in_table
    length += 4;// week_names_table_size + difficulty_names_table_size
    length += 4;// freeplay_progress_count (in funkinsave v2 or newer)

    // step 2: prepare buffer
    console.assert(length < MATH2D_MAX_INT32);
    let savedata = new DataView(new ArrayBuffer(length));

    // step 3: build funkinsave header
    savedata.setUint8(offset, FUNKINSAVE_SAVEDATA_VERSION); offset++;
    savedata.setUint8(offset, 0x00); offset++;
    savedata.setUint16(offset, linkedlist_count(funkinsave.settings), ENDIANESS); offset += 2;
    savedata.setUint16(offset, linkedlist_count(funkinsave.directives), ENDIANESS); offset += 2;
    savedata.setUint16(offset, week_names_table_size, ENDIANESS); offset += 2;
    savedata.setUint16(offset, difficulty_names_table_size, ENDIANESS); offset += 2;
    savedata.setUint16(offset, linkedlist_count(funkinsave.progress), ENDIANESS); offset += 2;
    savedata.setUint16(offset, linkedlist_count(funkinsave.storages), ENDIANESS); offset += 2;
    savedata.setUint16(offset, last_played_week_index, ENDIANESS); offset += 2;
    savedata.setUint16(offset, last_played_difficulty_index, ENDIANESS); offset += 2;

    // dump settings
    for (let setting of linkedlist_iterate4(funkinsave.settings)) {
        let is_integer = funkinsave_internal_is_setting_integer(setting.id);
        savedata.setUint16(offset, setting.id, ENDIANESS); offset += 2;
        if (is_integer) savedata.setBigInt64(offset, BigInt(setting.value), ENDIANESS);
        else savedata.setFloat64(offset, setting.value, ENDIANESS);
        offset += 8;
    }

    // dump directives
    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        savedata.setUint8(offset, directive.type); offset++;
        offset += funkinsave_internal_dump_string(directive.name, savedata, offset);
        savedata.setFloat64(offset, directive.value, ENDIANESS); offset += 8;
    }

    // dump week names
    for (let week_name of linkedlist_iterate4(funkinsave.weeks_names)) {
        offset += funkinsave_internal_dump_string(week_name, savedata, offset);
    }

    // dump difficulty names
    for (let difficulty_name of linkedlist_iterate4(funkinsave.difficulty_names)) {
        offset += funkinsave_internal_dump_string(difficulty_name, savedata, offset);
    }

    // dump week progress
    for (let week_progress of linkedlist_iterate4(funkinsave.progress)) {
        savedata.setUint16(offset, week_progress.week_id, ENDIANESS); offset += 2;
        savedata.setUint16(offset, week_progress.difficulty_id, ENDIANESS); offset += 2;
        savedata.setBigInt64(offset, BigInt(week_progress.score), ENDIANESS); offset += 8;
    }

    // dump week storages
    for (let week_storage of linkedlist_iterate4(funkinsave.storages)) {
        savedata.setUint16(offset, week_storage.week_id, ENDIANESS); offset += 2;
        offset += funkinsave_internal_dump_string(week_storage.name, savedata, offset);
        savedata.setUint32(offset, week_storage.data_size, ENDIANESS); offset += 4;
        (new Uint8Array(savedata.buffer)).set(week_storage.data, offset); offset += week_storage.data_size;
    }

    // dump freeplay progress
    savedata.setUint32(offset, linkedlist_count(funkinsave.freeplay_progress), ENDIANESS); offset += 4;
    for (let freeplay_progress of linkedlist_iterate4(funkinsave.freeplay_progress)) {
        savedata.setUint16(offset, freeplay_progress.week_id, ENDIANESS); offset += 2;
        savedata.setUint16(offset, freeplay_progress.difficulty_id, ENDIANESS); offset += 2;
        savedata.setBigInt64(offset, BigInt(freeplay_progress.score), ENDIANESS); offset += 8;
        offset += funkinsave_internal_dump_string(freeplay_progress.song_name, savedata, offset);
    }

    // step 4: check overflows
    console.assert(offset == length, "offset != length");

    // step 5: check available space and delete previous savedata
    let vms_file;
    let available_space = MATH2D_MAX_INT32;
    let vmu_path = funkinsave_internal_get_vmu_path(funkinsave.maple_port, funkinsave.maple_unit);
    const vmu_stat = { st_blksize: 0, st_size: 0 };

    if (fs_stat("/vmu", vmu_stat, 0x00) == 0) {
        // calculate in bytes the space available
        available_space = vmu_stat.st_blksize * vmu_stat.st_size;
    }

    // read the old savedata size (if exists)
    vms_file = await fs_open(vmu_path, O_RDONLY);
    if (vms_file != -1) {
        let existing_size = fs_total(vms_file);
        if (existing_size > 0) available_space += existing_size;
        await fs_close(vms_file);
    }

    // the VMU stores data in blocks of 512 bytes, calculates how many blocks is required
    let required_size = Math.ceil((FUNKINSAVE_VMS_HEADER.length + length) / 512) * 512;
    if (required_size > available_space) return 2;// no space left

    // delete savedata
    if (vms_file != -1 && await fs_unlink(vmu_path)) {
        vmu_path = undefined;
        savedata = undefined;
        return 5;
    }

    // create a new savedata
    vms_file = await fs_open(vmu_path, O_WRONLY);

    if (vms_file == -1) {
        console.error("funkinsave_flush_to_vmu() failed to write to " + vmu_path + ", also the old savedata is lost :(");
        vmu_path = undefined;
        savedata = undefined;
        return 3;
    }

    // step 6: write funkinsave in the VMU
    let headerView = new DataView(FUNKINSAVE_VMS_HEADER.buffer);
    headerView.setInt32(FUNKINSAVE_VMS_HEADER_OFFSET_LENGTH, length, ENDIANESS);
    headerView.setUint16(FUNKINSAVE_VMS_HEADER_OFFSET_CRC16, 0x00, ENDIANESS);

    // step 7: calculate checksum
    let crc16 = funkinsave_internal_calc_crc16(headerView.buffer, headerView.byteLength, 0x0000);
    crc16 = funkinsave_internal_calc_crc16(savedata.buffer, savedata.byteLength, crc16);
    headerView.setUint16(FUNKINSAVE_VMS_HEADER_OFFSET_CRC16, crc16 & 0xffff, ENDIANESS);

    // step 9: write in the VMU
    if (fs_write(vms_file, FUNKINSAVE_VMS_HEADER, FUNKINSAVE_VMS_HEADER.byteLength) == -1) return 4;
    if (fs_write(vms_file, savedata, length) == -1) return 4;

    // done
    savedata = undefined;
    return await fs_close(vms_file) == 0 ? 0 : 4;
}


function funkinsave_get_week_score(week_name, week_difficult) {
    let week_id = linkedlist_index_of(funkinsave.weeks_names, week_name);
    let difficulty_id = linkedlist_index_of(funkinsave.difficulty_names, week_difficult);
    if (week_id < 0 || difficulty_id < 0) return 0;

    for (let progress of linkedlist_iterate4(funkinsave.progress)) {
        if (progress.week_id == week_id && progress.difficulty_id == difficulty_id) {
            return progress.score;
        }
    }

    return 0;
}

function funkinsave_get_completed_count() {
    return linkedlist_count(funkinsave.weeks_names);
}

function funkinsave_get_last_played_week() {
    if (funkinsave.last_played_week_index < 0) return null;
    return linkedlist_get_by_index(funkinsave.weeks_names, funkinsave.last_played_week_index);
}

function funkinsave_get_last_played_difficult() {
    if (funkinsave.last_played_difficulty_index < 0) return null;
    return linkedlist_get_by_index(funkinsave.difficulty_names, funkinsave.last_played_difficulty_index);
}


function funkinsave_create_unlock_directive(name, value) {
    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        if (directive.type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
            directive.value = value;
            return;
        }
    }

    let directive = { name: strdup(name), value, type: FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK };
    linkedlist_add_item(funkinsave.directives, directive);
    return;
}

function funkinsave_delete_unlock_directive(name) {
    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        if (directive.type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
            directive.name = undefined;
            linkedlist_remove_item(funkinsave.directives, directive);
            return;
        }
    }
}

function funkinsave_contains_unlock_directive(name) {
    if (name == null || name.length < 1) return 1;

    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        if (directive.type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
            return 1;
        }
    }
    return 0;
}

function funkinsave_read_unlock_directive(name, value_output) {
    for (let directive of linkedlist_iterate4(funkinsave.directives)) {
        if (directive.type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK && directive.name == name) {
            value_output[0] = directive.value;
            return 1;
        }
    }
    return 0;
}


function funkinsave_set_last_played(week_name, difficulty_name) {
    let week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    let difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);
    if (week_id >= 0) funkinsave.last_played_week_index = week_id;
    if (difficulty_id >= 0) funkinsave.last_played_difficulty_index = difficulty_id;
}

function funkinsave_set_week_score(week_name, difficulty_name, score) {
    let week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    let difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);

    if (week_id < 0 || difficulty_id < 0) return;

    for (let progress of linkedlist_iterate4(funkinsave.progress)) {
        if (progress.week_id == week_id && progress.difficulty_id == difficulty_id) {
            progress.score = score;
            return;
        }
    }

    let new_progress = { week_id, difficulty_id, score };
    linkedlist_add_item(funkinsave.progress, new_progress);
}

function funkinsave_set_setting(setting_id, setting_value) {
    for (let setting of linkedlist_iterate4(funkinsave.settings)) {
        if (setting.id == setting_id) {
            setting.value = setting_value;
            return;
        }
    }

    linkedlist_add_item(funkinsave.settings, { id: setting_id, value: setting_value });
}


function funkinsave_storage_set(week_name, name, data, data_size) {
    let week_id;

    if (week_name == null) {
        week_id = FUNKINSAVE_MAX_INDEXED_NAME;
    } else {
        week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
        if (week_id < 0) return 0;
    }

    let storage = null;

    for (let s of linkedlist_iterate4(funkinsave.storages)) {
        if (s.week_id == week_id && s.name == name) {
            storage = s;
            break;
        }
    }

    if (data_size < 1) {
        if (storage) {
            linkedlist_remove_item(funkinsave.storages, storage);
            storage.data = undefined;
            storage = undefined;
        }
        return 1;
    }

    if (!storage) storage = { week_id: 0, name: null, data_size: 0, data: null };

    data = clone_array(data, data_size, NaN);

    storage.data = undefined;
    storage.data = data;
    storage.data_size = data_size;
    storage.week_id = week_id;

    return 1;
}

function funkinsave_storage_get(week_name, name, out_data) {
    let week_id = week_name == null ? FUNKINSAVE_MAX_INDEXED_NAME : funkinsave.weeks_names.indexOf(name);

    for (let storage of linkedlist_iterate4(funkinsave.storages)) {
        if (storage.week_id == week_id && storage.name == name) {
            out_data[0] = storage.data;
            return storage.data_size;
        }
    }

    out_data[0] = null;
    return 0;
}

function funkinsave_set_freeplay_score(week_name, difficulty_name, song_name, score) {
    if (!song_name) return;

    let week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    let difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);

    if (week_id < 0 || difficulty_id < 0) return;

    for (let progress of linkedlist_iterate4(funkinsave.freeplay_progress)) {
        if (progress.week_id == week_id && progress.difficulty_id == difficulty_id && progress.song_name == song_name) {
            progress.score = score;
            return;
        }
    }

    let new_progress = {
        week_id,
        difficulty_id,
        song_name,
        score
    };
    linkedlist_add_item(funkinsave.freeplay_progress, new_progress);
}

function funkinsave_get_freeplay_score(week_name, difficulty_name, song_name) {
    if (!song_name) return 0;

    let week_id = linkedlist_index_of(funkinsave.weeks_names, week_name);
    let difficulty_id = linkedlist_index_of(funkinsave, difficulty_name);
    if (week_id < 0 || difficulty_id < 0) return 0;

    for (let progress of linkedlist_iterate4(funkinsave.freeplay_progress)) {
        if (progress.week_id == week_id && progress.difficulty_id == difficulty_id && progress.song_name == song_name) {
            return progress.score;
        }
    }

    return 0;
}


async function funkinsave_has_savedata_in_vmu(port, unit) {
    let dev = maple_enum_dev(port, unit);
    if (!dev) return 0;

    //
    // There no such thing like fs_file_exists, try open the file
    //
    let vmu_path = funkinsave_internal_get_vmu_path(port, unit);
    let vms_file = await fs_open(vmu_path, O_RDONLY);

    let found = vms_file != -1;
    await fs_close(vms_file);

    vmu_path = undefined;
    return found;
}


function funkinsave_set_vmu(port, unit) {
    funkinsave.maple_port = port;
    funkinsave.maple_unit = unit;
}

function funkinsave_is_vmu_missing() {
    let device = maple_enum_dev(funkinsave.maple_port, funkinsave.maple_unit);
    return !device || (device.info.functions & MAPLE_FUNC_MEMCARD) == 0x00;
}

function funkinsave_pick_first_available_vmu() {
    for (let i = 0, count = maple_enum_count(); i < count; i++) {
        let dev = maple_enum_type(i, MAPLE_FUNC_MEMCARD);

        if (dev && dev.valid) {
            funkinsave.maple_port = dev.port;
            funkinsave.maple_unit = dev.unit;
            return;
        }
    }
}


function funkinsave_internal_string_bytelength(str) {
    let string_length = string_bytelength(str);
    return Math.min(string_length, FUNKINSAVE_MAX_STRING_SIZE);
}

function funkinsave_internal_dump_string(str, buffer, offset) {
    /*
    // C only
    size_t size = 0;
    for (size_t i = 0; i < FUNKINSAVE_MAX_STRING_SIZE ; i++) {
        uint8_t character = str[i];
        *buffer = character;
        buffer++;
        size++;
        if (!character) return size;
    }
    return size;
    */

    let buf = new Uint8Array(buffer.buffer);
    let textEncoder = new TextEncoder();
    let temp = textEncoder.encode(str);

    if (temp.length >= FUNKINSAVE_MAX_STRING_SIZE) {
        temp = temp.subarray(0, FUNKINSAVE_MAX_STRING_SIZE);
        buf.set(temp, offset);
        return FUNKINSAVE_MAX_STRING_SIZE;
    }

    buf.set(temp, offset);
    buf[offset + temp.byteLength + 1] = 0x00;

    return temp.byteLength + 1;
}

function funkinsave_internal_clear_savedata() {
    linkedlist_clear(funkinsave.settings, free);

    for (let directive of linkedlist_iterate4(funkinsave.directives)) directive.name = undefined;
    linkedlist_clear(funkinsave.directives, free);

    linkedlist_clear(funkinsave.difficulty_names, free);

    linkedlist_clear(funkinsave.weeks_names, free);

    linkedlist_clear(funkinsave.progress, free);

    for (let week_storage of linkedlist_iterate4(funkinsave.storages)) week_storage.data = undefined;
    linkedlist_clear(funkinsave.storages, free);

    for (let freeplay_progress of linkedlist_iterate4(funkinsave.freeplay_progress)) freeplay_progress.song_name = undefined;
    linkedlist_clear(funkinsave.freeplay_progress, free);

    funkinsave.last_played_week_index = -1;
    funkinsave.last_played_difficulty_index = -1;
}

function funkinsave_internal_get_vmu_path(port, unit) {
    let port_name = String.fromCodePoint(0x61 + port);
    let slot_name = String.fromCodePoint(0x30 + unit);

    // Important: the vms filename can not be longer than 12 bytes
    return `/vmu/${port_name}${slot_name}/FNF_KDY_SAVE`;
}

function funkinsave_internal_read_string(buffer, offset, output_string) {
    let textDecoder = new TextDecoder("utf-8", { ignoreBOM: true, fatal: true });
    let buf = new Uint8Array(buffer.buffer, buffer.byteOffset + offset);
    let length = 0;
    let null_found = false;

    for (let i = 0; i < FUNKINSAVE_MAX_STRING_SIZE; i++) {
        if (buf[i] == 0x00) {
            null_found = true;
            break;
        }
        length++;
    }

    console.assert(length > 0);
    output_string[0] = textDecoder.decode(buf.subarray(0, length), { stream: false });

    if (null_found) length++;
    return length;
}

function funkinsave_internal_is_setting_integer(id) {
    if (id == 0xabcd) return 0;// debug only
    return 1;
}

function funkinsave_internal_calc_crc16(buffer, size, crc) {
    //
    // taken from vmu_pkg.c (KallistiOS sourcecode)
    //
    let buf = new Uint8Array(buffer);

    for (let i = 0; i < size; i++) {
        crc ^= (buf[i] << 8);

        for (let c = 0; c < 8; c++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 4129;
            else
                crc = (crc << 1);
        }
    }

    return crc;
}

function funkinsave_internal_name_index(linkedlist, name) {
    if (name == null) return -1;
    let index = linkedlist_index_of(linkedlist, name);
    if (index < 0) {
        index = linkedlist_count(linkedlist);
        if (index >= FUNKINSAVE_MAX_INDEXED_NAME) {
            console.error("funkinsave_internal_name_index() failed, ran out of indices");
            return -1;
        }
        linkedlist_add_item(linkedlist, strdup(name));
    }
    return index;
}

