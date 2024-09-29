#include "game/common/funkinsave.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dc/maple.h>
#include <dc/vmu_pkg.h>
#include <kos/fs.h>

#include "dataview.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "stringutils.h"


typedef struct __attribute__((__packed__)) {
    uint16_t id;
    SettingValue value;
} Setting;
typedef struct __attribute__((__packed__)) {
    uint8_t type;
    char* name;
    float64 value;
} Directive;
typedef struct __attribute__((__packed__)) {
    uint16_t week_name_index_in_table;
    uint16_t difficulty_name_index_in_table;
    int64_t score;
} Progress;
typedef struct __attribute__((__packed__)) {
    char* name;
    uint16_t week_name_index_in_table;
    uint data_size;
    uint8_t* data;
} Storage;
typedef struct __attribute__((__packed__)) {
    uint16_t week_name_index_in_table;
    uint16_t difficulty_name_index_in_table;
    char* song_name;
    int64_t score;
} FreeplayProgress;

typedef struct {
    int8_t maple_port;
    int8_t maple_unit;

    LinkedList settings;
    LinkedList directives;
    LinkedList weeks_names;
    LinkedList difficulty_names;
    LinkedList progress;
    LinkedList storages;
    LinkedList freeplay_progress;

    int32_t last_played_week_index;
    int32_t last_played_difficulty_index;
} FunkinSave;


static const uint8_t FUNKINSAVE_MAX_STRING_SIZE = 32;
static const uint8_t FUNKINSAVE_SAVEDATA_VERSION = 0x02;
static uint8_t FUNKINSAVE_VMS_HEADER[] = {
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
};

static const uint8_t FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK = 0x00;
static const uint16_t FUNKINSAVE_MAX_INDEXED_NAME = 0xFFFF - 1;

static char* FUNKINSAVE_VMU_PATH = "/vmu/PU";
static char* FUNKINSAVE_VMU_SAVEFILE_NAME = "FNF_KDY_SAVE";
static char* FUNKINSAVE_VMU_SAVEFILE_PATH = "/vmu/PU/FNF_KDY_SAVE";


bool funkinsave_has_changes = false;


/**
 * This hold all information loaded from the savefile stored in the Sega Dreamcast VMU
 */
FunkinSave funkinsave = {
    .maple_port = 0,
    .maple_unit = 1,

    .settings = NULL,
    .directives = NULL,
    .weeks_names = NULL,
    .difficulty_names = NULL,
    .progress = NULL,
    .storages = NULL,
    .freeplay_progress = NULL,

    .last_played_week_index = -1,
    .last_played_difficulty_index = -1
};


static size_t funkinsave_internal_string_bytelength(const char* str);
static void funkinsave_internal_dump_string(DataView* dataview, const char* str);
static void funkinsave_internal_clear_savedata();
const char* funkinsave_internal_get_vmu_path(char* static_path, int8_t port, int8_t unit);
static const char* funkinsave_internal_read_string(DataView* dataview);
static uint32_t funkinsave_internal_calc_crc16(uint8_t* buffer, size_t size, uint32_t crc);
static int32_t funkinsave_internal_name_index(LinkedList linkedlist, const char* name);
static void* funkinsave_internal_alloc(size_t val_size, const void* val, ssize_t str_offst, ssize_t xtr_offst, size_t xtr_size);


void __attribute__((constructor)) __ctor_funkinsave() {
    funkinsave.settings = linkedlist_init();
    funkinsave.directives = linkedlist_init();
    funkinsave.weeks_names = linkedlist_init();
    funkinsave.difficulty_names = linkedlist_init();
    funkinsave.progress = linkedlist_init();
    funkinsave.storages = linkedlist_init();
    funkinsave.freeplay_progress = linkedlist_init();
}


int32_t funkinsave_read_from_vmu() {
    // step 1: check if the VMU is inserted at the given port+slot
    if (funkinsave_is_vmu_missing()) return 1;

    // step 2: open savedata file
    funkinsave_internal_get_vmu_path(FUNKINSAVE_VMU_SAVEFILE_PATH, funkinsave.maple_port, funkinsave.maple_unit);
    file_t vms_file = fs_open(FUNKINSAVE_VMU_SAVEFILE_PATH, O_RDONLY);

    if (vms_file == FILEHND_INVALID) {
        logger_info("funkinsave_read_from_vmu() no savedata present in %s", FUNKINSAVE_VMU_SAVEFILE_PATH);
        return 2;
    }

    // step 3:  read the whole VMS file
    size_t readed = 0;
    size_t total = fs_total(vms_file);
    uint8_t* vms_data = malloc_for_array(uint8_t, total);

    if (total == (size_t)-1) {
        // something bad happen
        logger_error("funkinsave_read_from_vmu() call to fs_total() on %s failed", FUNKINSAVE_VMU_SAVEFILE_PATH);
        free_chk(vms_data);
        return 3;
    }

    while (readed < total) {
        ssize_t read = fs_read(vms_file, vms_data + readed, total);
        if (read < 0) {
            // something bad happen
            free_chk(vms_data);
            fs_close(vms_file);
            return 3;
        }
        if (read == 0) break; // EOF was reached
        readed += (size_t)read;
    }

    fs_close(vms_file);

    if (readed != total) {
        free_chk(vms_data);
        logger_error("funkinsave_read_from_vmu() failed to read the VMS file at %s", FUNKINSAVE_VMU_SAVEFILE_PATH);
        return 3;
    }

    // step 4: parse VMS header using vmu_pkg_parse() function from KallistiOS (required for CRC16 check)
    vmu_pkg_t vmu_pkg;

    if (vmu_pkg_parse(vms_data, &vmu_pkg) != 0) {
        logger_warn("funkinsave_read_from_vmu() call to vmu_pkg_parse() failed on: %s", FUNKINSAVE_VMU_SAVEFILE_PATH);

        free_chk(vms_data);
        return 4;
    }

    // step 5: parse savedata
    DataView savedata = dataview_from(vmu_pkg.data, 0, vmu_pkg.data_len);
    uint8_t version = dataview_read_u8(savedata);
    dataview_skip(savedata, sizeof(uint8_t)); // reserved space

    // version check
    if (version > FUNKINSAVE_SAVEDATA_VERSION) {
        free_chk(vms_data);
        return 5;
    }

    // clear in-memory savedata
    funkinsave_internal_clear_savedata();

    uint16_t settings_count = dataview_read_u16(savedata);
    uint16_t directives_count = dataview_read_u16(savedata);
    uint16_t week_names_table_size = dataview_read_u16(savedata);
    uint16_t difficulty_names_table_size = dataview_read_u16(savedata);
    uint16_t progress_count = dataview_read_u16(savedata);
    uint16_t storages_count = dataview_read_u16(savedata);
    uint16_t last_played_week_name_index_in_table = dataview_read_u16(savedata);
    uint16_t last_played_difficulty_name_index_in_table = dataview_read_u16(savedata);

    for (uint16_t i = 0; i < settings_count; i++) {
        Setting entry;
        entry.id = dataview_read_u16(savedata);
        entry.value = dataview_read_pack4(savedata);

        void* entry_ptr = funkinsave_internal_alloc(sizeof(Setting), &entry, -1, -1, 0);
        linkedlist_add_item(funkinsave.settings, entry_ptr);
    }

    for (uint16_t i = 0; i < directives_count; i++) {
        Directive entry;
        entry.type = dataview_read_u8(savedata);
        entry.name = (char*)funkinsave_internal_read_string(&savedata);
        entry.value = dataview_read_f64(savedata);

        void* entry_ptr = funkinsave_internal_alloc(sizeof(Directive), &entry, offsetof(Directive, name), -1, 0);
        linkedlist_add_item(funkinsave.directives, entry_ptr);
    }

    for (uint16_t i = 0; i < week_names_table_size; i++) {
        const char* name = funkinsave_internal_read_string(&savedata);

        char* entry = string_duplicate(name);
        linkedlist_add_item(funkinsave.weeks_names, entry);
    }

    for (uint16_t i = 0; i < difficulty_names_table_size; i++) {
        const char* name = funkinsave_internal_read_string(&savedata);

        char* entry = string_duplicate(name);
        linkedlist_add_item(funkinsave.difficulty_names, entry);
    }

    for (uint16_t i = 0; i < progress_count; i++) {
        Progress entry;
        entry.week_name_index_in_table = dataview_read_u16(savedata);
        entry.difficulty_name_index_in_table = dataview_read_u16(savedata);
        entry.score = dataview_read_i64(savedata);

        assert(entry.week_name_index_in_table < week_names_table_size);
        assert(entry.difficulty_name_index_in_table < difficulty_names_table_size);

        void* entry_ptr = funkinsave_internal_alloc(sizeof(Progress), &entry, -1, -1, 0);
        linkedlist_add_item(funkinsave.progress, entry_ptr);
    }

    for (uint16_t i = 0; i < storages_count; i++) {
        Storage entry;
        entry.week_name_index_in_table = dataview_read_u16(savedata);
        entry.name = (char*)funkinsave_internal_read_string(&savedata);
        entry.data_size = dataview_read_u32(savedata);
        entry.data = dataview_pointer(savedata);

        dataview_skip(savedata, entry.data_size);

        void* entry_ptr = funkinsave_internal_alloc(
            sizeof(Storage), &entry,
            offsetof(Storage, name),
            offsetof(Storage, data), entry.data_size
        );
        linkedlist_add_item(funkinsave.storages, entry_ptr);
    }

    if (version >= 2) {
        uint32_t freeplay_progress_count = dataview_read_u32(savedata);

        for (uint32_t i = 0; i < freeplay_progress_count; i++) {
            FreeplayProgress entry;
            entry.week_name_index_in_table = dataview_read_u16(savedata);
            entry.difficulty_name_index_in_table = dataview_read_u16(savedata);
            entry.score = dataview_read_i64(savedata);
            entry.song_name = (char*)funkinsave_internal_read_string(&savedata);

            assert(entry.week_name_index_in_table < week_names_table_size);
            assert(entry.difficulty_name_index_in_table < difficulty_names_table_size);

            void* entry_ptr = funkinsave_internal_alloc(sizeof(FreeplayProgress), &entry, offsetof(FreeplayProgress, song_name), -1, 0);
            linkedlist_add_item(funkinsave.freeplay_progress, entry_ptr);
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
    dataview_assert(savedata);
    free_chk(vms_data);
    return 0;
}

int32_t funkinsave_write_to_vmu() {
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

    size_t savedata_length = 0;
    uint16_t week_names_table_size = 0;
    uint16_t difficulty_names_table_size = 0;

    uint16_t last_played_week_index = 0xFFFF;
    uint16_t last_played_difficulty_index = 0xFFFF;

    if (funkinsave.last_played_week_index > 0)
        last_played_week_index = funkinsave.last_played_week_index;
    if (funkinsave.last_played_difficulty_index >= 0)
        last_played_difficulty_index = funkinsave.last_played_difficulty_index;

    if (funkinsave_is_vmu_missing()) return 1;

    // step 1: count amount bytes needed
    savedata_length += (size_t)linkedlist_count(funkinsave.settings) * sizeof(Setting);
    savedata_length += (size_t)linkedlist_count(funkinsave.progress) * sizeof(Progress);

    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        savedata_length += funkinsave_internal_string_bytelength(directive->name);
        savedata_length += sizeof(Directive) - sizeof(char*);
    }

    foreach (char*, week_name, LINKEDLIST_ITERATOR, funkinsave.weeks_names) {
        savedata_length += funkinsave_internal_string_bytelength(week_name);
        week_names_table_size++;
    }

    foreach (char*, difficulty_name, LINKEDLIST_ITERATOR, funkinsave.difficulty_names) {
        savedata_length += funkinsave_internal_string_bytelength(difficulty_name);
        difficulty_names_table_size++;
    }

    foreach (Storage*, week_storage, LINKEDLIST_ITERATOR, funkinsave.storages) {
        savedata_length += funkinsave_internal_string_bytelength(week_storage->name);
        savedata_length += week_storage->data_size;
        savedata_length += sizeof(Storage) - sizeof(char*) - sizeof(uint8_t*);
    }

    foreach (FreeplayProgress*, freeplay_progress, LINKEDLIST_ITERATOR, funkinsave.freeplay_progress) {
        savedata_length += funkinsave_internal_string_bytelength(freeplay_progress->song_name);
        savedata_length += sizeof(FreeplayProgress) - sizeof(char*);
    }

    savedata_length += 10; // version + reserved + settings_count + directives_count + progress_count + storages_count
    savedata_length += 4;  // last_played_week_name_index_in_table + last_played_difficulty_name_index_in_table
    savedata_length += 4;  // week_names_table_size + difficulty_names_table_size
    savedata_length += 4;  // freeplay_progress_count (in funkinsave v2 or newer)

    // step 2: prepare buffer
    assert(savedata_length < INT32_MAX);

    uint8_t* vms_data = malloc_for_array(uint8_t, savedata_length);
    DataView savedata = dataview_from(vms_data, 0, savedata_length);

    // step 3: build funkinsave header
    dataview_write_u8(savedata, FUNKINSAVE_SAVEDATA_VERSION);
    dataview_write_u8(savedata, 0x00);
    dataview_write_u16(savedata, (uint16_t)linkedlist_count(funkinsave.settings));
    dataview_write_u16(savedata, (uint16_t)linkedlist_count(funkinsave.directives));
    dataview_write_u16(savedata, week_names_table_size);
    dataview_write_u16(savedata, difficulty_names_table_size);
    dataview_write_u16(savedata, (uint16_t)linkedlist_count(funkinsave.progress));
    dataview_write_u16(savedata, (uint16_t)linkedlist_count(funkinsave.storages));
    dataview_write_u16(savedata, last_played_week_index);
    dataview_write_u16(savedata, last_played_difficulty_index);

    // dump settings
    foreach (Setting*, setting, LINKEDLIST_ITERATOR, funkinsave.settings) {
        dataview_write_u16(savedata, setting->id);
        dataview_write_pack4(savedata, setting->value);
    }

    // dump directives
    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        dataview_write_u8(savedata, directive->type);
        funkinsave_internal_dump_string(&savedata, directive->name);
        dataview_write_f64(savedata, directive->value);
    }

    // dump week names
    foreach (char*, week_name, LINKEDLIST_ITERATOR, funkinsave.weeks_names) {
        funkinsave_internal_dump_string(&savedata, week_name);
    }

    // dump difficulty names
    foreach (char*, difficulty_name, LINKEDLIST_ITERATOR, funkinsave.difficulty_names) {
        funkinsave_internal_dump_string(&savedata, difficulty_name);
    }

    // dump week progress
    foreach (Progress*, week_progress, LINKEDLIST_ITERATOR, funkinsave.progress) {
        dataview_write_u16(savedata, week_progress->week_name_index_in_table);
        dataview_write_u16(savedata, week_progress->difficulty_name_index_in_table);
        dataview_write_i64(savedata, week_progress->score);
    }

    // dump week storages
    foreach (Storage*, week_storage, LINKEDLIST_ITERATOR, funkinsave.storages) {
        dataview_write_u16(savedata, week_storage->week_name_index_in_table);
        funkinsave_internal_dump_string(&savedata, week_storage->name);
        dataview_write_u32(savedata, week_storage->data_size);

        memcpy(dataview_pointer(savedata), week_storage->data, week_storage->data_size);
        dataview_skip(savedata, week_storage->data_size);
    }

    // dump freeplay progress
    dataview_write_u32(savedata, (uint32_t)linkedlist_count(funkinsave.freeplay_progress));
    foreach (FreeplayProgress*, freeplay_progress, LINKEDLIST_ITERATOR, funkinsave.freeplay_progress) {
        dataview_write_u16(savedata, freeplay_progress->week_name_index_in_table);
        dataview_write_u16(savedata, freeplay_progress->difficulty_name_index_in_table);
        dataview_write_i64(savedata, freeplay_progress->score);
        funkinsave_internal_dump_string(&savedata, freeplay_progress->song_name);
    }

    // step 4: check overflows
    dataview_assert(savedata);

    // step 5: check available space and delete previous savedata
    file_t vms_file;
    size_t available_space = INT32_MAX;
    funkinsave_internal_get_vmu_path(FUNKINSAVE_VMU_SAVEFILE_PATH, funkinsave.maple_port, funkinsave.maple_unit);
    struct stat vmu_stat = {.st_blksize = 0, .st_size = 0};

    funkinsave_internal_get_vmu_path(FUNKINSAVE_VMU_PATH, funkinsave.maple_port, funkinsave.maple_unit);
    if (fs_stat(FUNKINSAVE_VMU_PATH, &vmu_stat, 0x00) == 0) {
        // calculate in bytes the space available
        available_space = (size_t)(vmu_stat.st_blksize * vmu_stat.st_size);
    }

    // read the old savedata size (if exists)
    vms_file = fs_open(FUNKINSAVE_VMU_SAVEFILE_PATH, O_RDONLY);
    if (vms_file != FILEHND_INVALID) {
        size_t existing_size = fs_total(vms_file);
        if (existing_size != (size_t)-1) available_space += existing_size;
        fs_close(vms_file);
    }

    // the VMU stores data in blocks of 512 bytes, calculates how many blocks is required
    size_t required_size = ((sizeof(FUNKINSAVE_VMS_HEADER) + savedata_length + 511) / 512) * 512;
    if (required_size > available_space) return 2; // no space left

    // delete savedata
    if (vms_file != FILEHND_INVALID && fs_unlink(FUNKINSAVE_VMU_SAVEFILE_PATH)) {
        free_chk(vms_data);
        return 5;
    }

    // create a new savedata
    vms_file = fs_open(FUNKINSAVE_VMU_SAVEFILE_PATH, O_WRONLY);

    if (vms_file == FILEHND_INVALID) {
        logger_error("funkinsave_flush_to_vmu() failed to write to %s, also the old savedata is lost :(", FUNKINSAVE_VMU_SAVEFILE_PATH);
        free_chk(vms_data);
        return 3;
    }

    // step 6: write funkinsave in the VMU
    vmu_hdr_t* headerView = (vmu_hdr_t*)FUNKINSAVE_VMS_HEADER;
    headerView->data_len = (uint32_t)savedata_length;
    headerView->crc = 0x00;

    // step 7: calculate checksum
    uint32_t crc16 = funkinsave_internal_calc_crc16(FUNKINSAVE_VMS_HEADER, sizeof(FUNKINSAVE_VMS_HEADER), 0x0000);
    crc16 = funkinsave_internal_calc_crc16(vms_data, savedata_length, crc16);
    headerView->crc = crc16 & 0xffff;

    // step 9: write in the VMU
    if (fs_write(vms_file, FUNKINSAVE_VMS_HEADER, sizeof(FUNKINSAVE_VMS_HEADER)) == -1) return 4;
    if (fs_write(vms_file, vms_data, savedata_length) == -1) return 4;

    // done
    free_chk(vms_data);
    return fs_close(vms_file) == 0 ? 0 : 4;
}


int64_t funkinsave_get_week_score(const char* week_name, const char* week_difficult) {
    int32_t week_id = linkedlist_index_of2(funkinsave.weeks_names, week_name);
    int32_t difficulty_id = linkedlist_index_of2(funkinsave.difficulty_names, week_difficult);
    if (week_id < 0 || difficulty_id < 0) return 0;

    foreach (Progress*, progress, LINKEDLIST_ITERATOR, funkinsave.progress) {
        if (progress->week_name_index_in_table == week_id && progress->difficulty_name_index_in_table == difficulty_id) {
            return progress->score;
        }
    }

    return 0;
}

int32_t funkinsave_get_completed_count() {
    return linkedlist_count(funkinsave.weeks_names);
}

const char* funkinsave_get_last_played_week() {
    if (funkinsave.last_played_week_index < 0) return NULL;
    return linkedlist_get_by_index(funkinsave.weeks_names, funkinsave.last_played_week_index);
}

const char* funkinsave_get_last_played_difficult() {
    if (funkinsave.last_played_difficulty_index < 0) return NULL;
    return linkedlist_get_by_index(funkinsave.difficulty_names, funkinsave.last_played_difficulty_index);
}


void funkinsave_create_unlock_directive(const char* name, float64 value) {
    funkinsave_has_changes = true;

    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        if (directive->type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK) {
            if (string_equals(directive->name, name)) {
                directive->value = value;
                return;
            }
        }
    }

    Directive directive = {.name = (char*)name, .value = value, .type = FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK};

    void* entry_ptr = funkinsave_internal_alloc(sizeof(Directive), &directive, offsetof(Directive, name), -1, 0);
    linkedlist_add_item(funkinsave.directives, entry_ptr);
    return;
}

void funkinsave_delete_unlock_directive(const char* name) {
    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        if (directive->type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK) {
            if (string_equals(directive->name, name)) {
                linkedlist_remove_item(funkinsave.directives, directive);
                free_chk(directive);
                funkinsave_has_changes = true;
                return;
            }
        }
    }
}

bool funkinsave_contains_unlock_directive(const char* name) {
    if (string_is_empty(name)) return true;

    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        if (directive->type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK) {
            if (string_equals(directive->name, name)) {
                return true;
            }
        }
    }
    return false;
}

bool funkinsave_read_unlock_directive(const char* name, float64* value_output) {
    foreach (Directive*, directive, LINKEDLIST_ITERATOR, funkinsave.directives) {
        if (directive->type == FUNKINSAVE_DIRECTIVE_TYPE_UNLOCK) {
            if (string_equals(directive->name, name)) {
                *value_output = directive->value;
                return true;
            }
        }
    }
    return false;
}


void funkinsave_set_last_played(const char* week_name, const char* difficulty_name) {
    int32_t week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    int32_t difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);
    if (week_id >= 0) funkinsave.last_played_week_index = week_id;
    if (difficulty_id >= 0) funkinsave.last_played_difficulty_index = difficulty_id;
    funkinsave_has_changes = true;
}

void funkinsave_set_week_score(const char* week_name, const char* difficulty_name, bool only_if_best, int64_t score) {
    int32_t week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    int32_t difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);

    if (week_id < 0 || difficulty_id < 0) return;

    foreach (Progress*, progress, LINKEDLIST_ITERATOR, funkinsave.progress) {
        if (progress->week_name_index_in_table == week_id && progress->difficulty_name_index_in_table == difficulty_id) {
            if (!only_if_best || score > progress->score) {
                progress->score = score;
            }
            return;
        }
    }

    Progress new_progress = (Progress){
        .week_name_index_in_table = week_id,
        .difficulty_name_index_in_table = difficulty_id,
        .score = score
    };

    void* entry_ptr = funkinsave_internal_alloc(sizeof(Progress), &new_progress, -1, -1, 0);
    linkedlist_add_item(funkinsave.progress, entry_ptr);
    funkinsave_has_changes = true;
}


void funkinsave_set_setting(uint16_t setting_id, SettingValue setting_value) {
    foreach (Setting*, setting, LINKEDLIST_ITERATOR, funkinsave.settings) {
        if (setting->id == setting_id) {
            setting->value = setting_value;
            funkinsave_has_changes = true;
            return;
        }
    }

    Setting new_setting = (Setting){.id = setting_id, .value = setting_value};

    void* entry_ptr = funkinsave_internal_alloc(sizeof(Setting), &new_setting, -1, -1, 0);
    linkedlist_add_item(funkinsave.settings, entry_ptr);
    funkinsave_has_changes = true;
}

SettingValue funkinsave_get_setting(uint16_t setting_id, SettingValue setting_value) {
    foreach (Setting*, setting, LINKEDLIST_ITERATOR, funkinsave.settings) {
        if (setting->id == setting_id) {
            return setting->value;
        }
    }

    Setting new_setting = (Setting){.id = setting_id, .value = setting_value};

    void* entry_ptr = funkinsave_internal_alloc(sizeof(Setting), &new_setting, -1, -1, 0);
    linkedlist_add_item(funkinsave.settings, entry_ptr);

    return setting_value;
}


bool funkinsave_storage_set(const char* week_name, const char* name, const uint8_t* data, size_t data_size) {
    int32_t week_id;

    if (week_name == NULL) {
        week_id = FUNKINSAVE_MAX_INDEXED_NAME;
    } else {
        week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
        if (week_id < 0) return false;
    }

    foreach (Storage*, storage, LINKEDLIST_ITERATOR, funkinsave.storages) {
        if (storage->week_name_index_in_table == week_id) {
            if (string_equals(storage->name, name)) {
                linkedlist_remove_item(funkinsave.storages, storage);
                free_chk(storage);
                break;
            }
        }
    }

    funkinsave_has_changes = true;

    if (data_size < 1) {
        return true;
    }

    Storage new_storage = (Storage){
        .week_name_index_in_table = week_id,
        .name = (char*)name,
        .data_size = data_size,
        .data = (uint8_t*)data
    };

    void* entry_ptr = funkinsave_internal_alloc(
        sizeof(Storage), &new_storage,
        offsetof(Storage, name),
        offsetof(Storage, data_size), data_size
    );
    linkedlist_add_item(funkinsave.storages, entry_ptr);

    return true;
}

size_t funkinsave_storage_get(const char* week_name, const char* name, uint8_t** out_data) {
    int32_t week_id;
    if (week_name == NULL)
        week_id = FUNKINSAVE_MAX_INDEXED_NAME;
    else
        week_id = linkedlist_index_of2(funkinsave.weeks_names, name);

    foreach (Storage*, storage, LINKEDLIST_ITERATOR, funkinsave.storages) {
        if (storage->week_name_index_in_table == week_id) {
            if (string_equals(storage->name, name)) {
                *out_data = storage->data;
                return storage->data_size;
            }
        }
    }

    *out_data = NULL;
    return 0;
}

void funkinsave_set_freeplay_score(const char* week_name, const char* difficulty_name, const char* song_name, bool only_if_best, int64_t score) {
    if (!song_name) return;

    int32_t week_id = funkinsave_internal_name_index(funkinsave.weeks_names, week_name);
    int32_t difficulty_id = funkinsave_internal_name_index(funkinsave.difficulty_names, difficulty_name);

    if (week_id < 0 || difficulty_id < 0) return;

    foreach (FreeplayProgress*, progress, LINKEDLIST_ITERATOR, funkinsave.freeplay_progress) {
        if (progress->week_name_index_in_table == week_id && progress->difficulty_name_index_in_table == difficulty_id) {
            if (string_equals(progress->song_name, song_name)) {
                if (!only_if_best || score > progress->score) {
                    progress->score = score;
                    funkinsave_has_changes = true;
                }
                return;
            }
        }
    }

    FreeplayProgress new_progress = (FreeplayProgress){
        .week_name_index_in_table = week_id,
        .difficulty_name_index_in_table = difficulty_id,
        .song_name = (char*)song_name,
        .score = score
    };

    void* entry_ptr = funkinsave_internal_alloc(sizeof(FreeplayProgress), &new_progress, offsetof(FreeplayProgress, song_name), -1, 0);
    linkedlist_add_item(funkinsave.freeplay_progress, entry_ptr);
    funkinsave_has_changes = true;
}

int64_t funkinsave_get_freeplay_score(const char* week_name, const char* difficulty_name, const char* song_name) {
    if (!song_name) return 0;

    int32_t week_id = linkedlist_index_of2(funkinsave.weeks_names, week_name);
    int32_t difficulty_id = linkedlist_index_of2(funkinsave.difficulty_names, difficulty_name);
    if (week_id < 0 || difficulty_id < 0) return 0;

    foreach (FreeplayProgress*, progress, LINKEDLIST_ITERATOR, funkinsave.freeplay_progress) {
        if (progress->week_name_index_in_table == week_id && progress->difficulty_name_index_in_table == difficulty_id) {
            if (string_equals(progress->song_name, song_name)) {
                return progress->score;
            }
        }
    }

    return 0;
}


bool funkinsave_has_savedata_in_vmu(int8_t port, int8_t unit) {
    maple_device_t* dev = maple_enum_dev(port, unit);
    if (!dev) return false;

    funkinsave_internal_get_vmu_path(FUNKINSAVE_VMU_PATH, port, unit);
    file_t dir_hnd = fs_open(FUNKINSAVE_VMU_PATH, O_RDONLY | O_DIR);

    if (dir_hnd == FILEHND_INVALID) {
        return false;
    }

    bool found = false;
    dirent_t* entry;
    while ((entry = fs_readdir(dir_hnd)) != NULL) {
        if (string_equals(entry->name, FUNKINSAVE_VMU_SAVEFILE_NAME)) {
            found = true;
            break;
        }
    }

    fs_close(dir_hnd);
    return found;
}


void funkinsave_set_vmu(int8_t port, int8_t unit) {
    funkinsave.maple_port = port;
    funkinsave.maple_unit = unit;
}

bool funkinsave_is_vmu_missing() {
    maple_device_t* device = maple_enum_dev(funkinsave.maple_port, funkinsave.maple_unit);
    return !device || (device->info.functions & MAPLE_FUNC_MEMCARD) == 0x00;
}

void funkinsave_pick_first_available_vmu() {
    bool pick_first_device = true;
    int32_t found = 0;

    for (int i = 0, count = maple_enum_count(); i < count; i++) {
        maple_device_t* dev = maple_enum_type(i, MAPLE_FUNC_MEMCARD);

        if (dev && dev->valid) {
            if (pick_first_device) {
                funkinsave.maple_port = (int8_t)dev->port;
                funkinsave.maple_unit = (int8_t)dev->unit;
                pick_first_device = false;
            }
            found++;
        }
    }

    if (found > 1) {
        // more than 1 virtual saveslots found, dont pick an vmu
        funkinsave.maple_port = -1;
        funkinsave.maple_unit = -1;
    }
}

bool funkinsave_get_vmu(int8_t* port, int8_t* unit) {
    *port = funkinsave.maple_port;
    *unit = funkinsave.maple_unit;

    return funkinsave.maple_port >= 0 && funkinsave.maple_unit >= 0;
}

int32_t funkinsave_delete_from_vmu(int8_t port, int8_t unit) {
    if (port < 0 || port >= MAPLE_PORT_COUNT) return 1;
    if (unit < 0 || unit >= MAPLE_UNIT_COUNT) return 1;

    // step 1: check if the VMU is inserted at the given port+slot
    maple_device_t* device = maple_enum_dev(port, unit);
    if (!device || (device->info.functions & MAPLE_FUNC_MEMCARD) == 0x00) {
        return 2;
    }

    // step 2: delete savedata file
    const char* vmu_path = funkinsave_internal_get_vmu_path(FUNKINSAVE_VMU_SAVEFILE_PATH, port, unit);
    int ret = fs_unlink(vmu_path);

    //
    // Possible return values:
    //   0 success
    //  -1 invalid vmu path
    //  -1 vms file not found
    //  -2 failed to initialize the vmu filesystem (issued by hardware or driver)
    //  -2 fat rewrite failed or already was damaged (the whole vmu is corrupted)
    //
    if (ret == 0)
        return 0;
    if (ret == -1)
        return 3;
    else if (ret == -2)
        return 4;

    // unknown error code found
    logger_error("funkinsave_delete_from_vmu() failed on %s with unknown error %i", vmu_path, ret);

    return 5;
}


static size_t funkinsave_internal_string_bytelength(const char* str) {
    size_t string_length = string_get_bytelength(str);
    return math2d_min_nint(string_length, FUNKINSAVE_MAX_STRING_SIZE);
}

static void funkinsave_internal_dump_string(DataView* dataview, const char* str) {
    uint8_t* buffer = dataview->buffer_u8;
    size_t str_len = funkinsave_internal_string_bytelength(str);

    memcpy(buffer, str, str_len);

    dataview->buffer_u8 += str_len;
}

static void funkinsave_internal_clear_savedata() {
    linkedlist_clear(funkinsave.settings, free_chk);
    linkedlist_clear(funkinsave.directives, free_chk);
    linkedlist_clear(funkinsave.difficulty_names, free_chk);
    linkedlist_clear(funkinsave.weeks_names, free_chk);
    linkedlist_clear(funkinsave.progress, free_chk);
    linkedlist_clear(funkinsave.storages, free_chk);
    linkedlist_clear(funkinsave.freeplay_progress, free_chk);

    funkinsave.last_played_week_index = -1;
    funkinsave.last_played_difficulty_index = -1;
}

const char* funkinsave_internal_get_vmu_path(char* static_path, int8_t port, int8_t unit) {
    char port_name = (char){0x61 + port};
    char slot_name = (char){0x30 + unit};

    // Important: the vms filename can not be longer than 12 bytes
    static_path[5] = port_name;
    static_path[6] = slot_name;

    return static_path;
}

static const char* funkinsave_internal_read_string(DataView* dataview) {
    uint8_t* buffer = dataview->buffer_u8;
    const char* str = (const char*)buffer;

    for (size_t i = 1; i < FUNKINSAVE_MAX_STRING_SIZE; i++) {
        if (*buffer == 0x00) {
            break;
        }
        buffer++;
    }

    *buffer++ = 0x00;
    dataview->buffer_u8 = buffer;

    return str;
}

static uint32_t funkinsave_internal_calc_crc16(uint8_t* buffer, size_t size, uint32_t crc) {
    //
    // taken from vmu_pkg.c (KallistiOS sourcecode)
    //

    for (size_t i = 0; i < size; i++) {
        crc ^= (buffer[i] << 8);

        for (size_t c = 0; c < 8; c++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 4129;
            else
                crc = (crc << 1);
        }
    }

    return crc;
}

static int32_t funkinsave_internal_name_index(LinkedList linkedlist, const char* name) {
    if (name == NULL) return -1;
    int32_t index = linkedlist_index_of2(linkedlist, name);
    if (index < 0) {
        index = linkedlist_count(linkedlist);
        if (index >= FUNKINSAVE_MAX_INDEXED_NAME) {
            logger_error("funkinsave_internal_name_index() failed, ran out of indices");
            return -1;
        }
        linkedlist_add_item(linkedlist, string_duplicate(name));
    }
    return index;
}

static void* funkinsave_internal_alloc(size_t val_size, const void* val, ssize_t str_offst, ssize_t xtr_offst, size_t xtr_size) {
    size_t str_length;
    const char* str;

    if (str_offst >= 0) {
        str = *((const char**)((uint8_t*)val + str_offst));
        str_length = strlen(str);
        if (str_length >= FUNKINSAVE_MAX_STRING_SIZE) {
            str_length = FUNKINSAVE_MAX_STRING_SIZE - 1;
        }
    } else {
        str = NULL;
        str_length = 0;
    }

    size_t entry_length = val_size + str_length + xtr_size;
    if (str) entry_length++;

    uint8_t* entry = malloc_for_array(uint8_t, entry_length);
    uint8_t* ptr = entry;

    // copy value
    memcpy(ptr, val, val_size);
    ptr += val_size;

    // copy and set string
    if (str) {
        const char** entry_str_ptr = (const char**)(entry + str_offst);
        *entry_str_ptr = (const char*)ptr;

        memcpy(ptr, str, str_length);
        ptr += str_length;
        *ptr = '\0';
        ptr++;
    }

    // copy and set extra data
    if (xtr_size > 0) {
        const void** entry_xtr_ptr = (const void**)(entry + xtr_offst);
        memcpy(ptr, *entry_xtr_ptr, xtr_size);
        *entry_xtr_ptr = (const void*)ptr;
    }

    return entry;
}
