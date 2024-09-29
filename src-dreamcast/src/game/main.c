#include "game/main.h"

#include <assert.h>

#include <arch/arch.h>
#include <arch/timer.h>
#include <dc/biosfont.h>
#include <dc/fs_iso9660.h>
#include <dc/maple/controller.h>
#include <kos/init.h>
#include <kos/thread.h>

#include "expansions.h"
#include "float64.h"
#include "fontholder.h"
#include "fs.h"
#include "imgutils.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "preloadcache.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringutils.h"

#include "externals/ffgraph.h"
#include "externals/fontatlas.h"
#include "externals/luascript.h"
#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/sndbridge_sfx.h"

#include "game/introscreen.h"
#include "game/mainmenu.h"
#include "game/savemanager.h"
#include "game/startscreen.h"
#include "game/weekselector.h"

#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/common/weekenumerator.h"

#include "game/gameplay/helpers/menumanifest.h"


/* These macros tell KOS how to initialize itself. All of this initialization
   happens before main() gets called, and the shutdown happens afterwards. So
   you need to set any flags you want here. Here are some possibilities:

   INIT_NONE         -- don't do any auto init
   INIT_IRQ          -- Enable IRQs (DO NOT ENABLE, breaks video playback)
   INIT_THD_PREEMPT  -- Enable pre-emptive threading (already in default mode)
   INIT_NET          -- Enable networking (doesn't imply lwIP!)
   INIT_MALLOCSTATS  -- Enable a call to malloc_stats() right before shutdown
   INIT_NO_DCLOAD    -- Disable dcload

   You can OR any or all of those together. If you want to start out with
   the current KOS defaults, use INIT_DEFAULT (or leave void * __kos_romdiske it out entirely). */
KOS_INIT_FLAGS(
    /*INIT_IRQ | */ INIT_CDROM | INIT_CONTROLLER | INIT_VMU | INIT_PURUPURU | INIT_NO_DCLOAD | INIT_NO_SHUTDOWN
);

/* Declaration of the romdisk
   You can access the files inside it by using the "/rd" mounting point. */
const void* __kos_romdisk = KOS_INIT_ROMDISK_NONE;


typedef struct {
    void* (*routine)(void* param);
    void* param;
} ThreadInitData;


//
// this file contains all shared data across the game
//

const char* ENGINE_NAME = "kdmy-engine";
const char* ENGINE_VERSION = "0.60";

/**
 * The background music used in all menus, inherited from introscreen
 */
SoundPlayer background_menu_music = NULL;
int main_argc;
char** main_argv;


static void main_check_for_expansion();

static void main_print_thread_list(uint8_t addr, uint32_t btns) {
    (void)addr;
    (void)btns;

    logger_info("main_print_thread_list() thread list:");
    thd_pslist(printf);
    thd_pslist_queue(printf);
    logger_info("-------------------------------------");
    fflush(stdout);
}


int main(int argc, char* argv[]) {
    main_argc = argc;
    main_argv = argv;

    // vital parts
    fs_init_kdy(); // intialize filesystem access for the main thread (this thread)
    pvr_context_init();
    sndbridge_init();
    sndbride_sfx_startup();
    main_check_for_expansion();
    // mastervolume_init();

    logger_info("SoundBridge: %s", sndbridge_get_runtime_info());
    logger_info("FontAtlas: FreeType %s", fontatlas_get_version());
    logger_info("LuaScript:  %s", luascript_get_version());
    logger_info("FFGraph:  %s\n", ffgraph_get_runtime_info());
    logger_info("%s %s", ENGINE_NAME, ENGINE_VERSION);

    // FIXME: on release builds there a deadlock somewhere
    cont_btn_callback(0, CONT_START | CONT_Y | CONT_A, main_print_thread_list);

    // return main_debug(argc, argv);

    main_helper_draw_loading_screen();

    logger_info("preloading fonts, this will take a while...");
    fontholder_init("/assets/common/font/vcr.ttf", NULL, false);
    if (DBL_MEM_CHECK != 0) {
        fontholder_init("/assets/common/font/alphabet.xml", "bold", false);
        fontholder_init("/assets/common/font/pixel.otf", NULL, false);
    }

    weekenumerator_enumerate();

    if (weeks_array.size < 1) logger_warn("main() no weeks detected");

    // load savedata
    funkinsave_pick_first_available_vmu();
    int32_t load_result = savemanager_should_show(false);
    if (load_result) {
        SaveManager savemanager = savemanager_init(false, load_result);
        savemanager_show(savemanager);
        savemanager_destroy(&savemanager);
    }

    // choose a random menu style
    {
        bool visited[weeks_array.size];

        bool try_choose_last_played = math2d_random_float() <= 0.15f;
        int32_t visited_count = 0;
        memset(visited, 0x00, (size_t)weeks_array.size * sizeof(bool));

        while (visited_count < weeks_array.size) {
            WeekInfo* weekinfo;

            if (try_choose_last_played) {
                try_choose_last_played = false;
                weekinfo = false;

                const char* last_played;
                /*if (SETTINGS.style_from_week_name)
                last_played = SETTINGS.style_from_week_name;
                else*/
                last_played = funkinsave_get_last_played_week();

                if (last_played != NULL) {
                    for (int32_t i = 0; i < weeks_array.size; i++) {
                        if (string_equals(weeks_array.array[i].name, last_played)) {
                            visited[i] = true;
                            visited_count++;
                            weekinfo = &weeks_array.array[i];
                        }
                    }
                }
                if (weekinfo == NULL) continue;
            } else {
                int32_t random_index = math2d_random_int(0, weeks_array.size);
                if (visited[random_index]) continue;
                visited[random_index] = true;
                visited_count++;
                weekinfo = &weeks_array.array[random_index];
            }

            if (!weekinfo->custom_folder && !weekinfo->has_greetings) continue;

            if (funkinsave_contains_unlock_directive(weekinfo->unlock_directive)) {
                custom_style_from_week = weekinfo;

                // override commons files (if possible)
                if (weekinfo->custom_folder) {
                    fs_override_common_folder(weekinfo->custom_folder);
                }
                break;
            }
        }
    }

    background_menu_music = soundplayer_init(FUNKIN_BACKGROUND_MUSIC);
    if (background_menu_music) soundplayer_loop_enable(background_menu_music, true);

#if 0
// debug only
#ifdef MALLOC_CHK_ENABLE
    mallocfilter_all_chk();
    preloadcache_malloc_chk_enable_autofilter_entries(true);
#endif
    preloadcache_add_file_list("/assets/weeks/week7/_preload.ini");
    extern int32_t week_main(WeekInfo * w, bool a, const char* d, const char* b, const char* g, const char* gm, int32_t s, const char* e);
    week_main(
        &weeks_array.array[7],
        false,
        "NORMAL",
        "/assets/common/data/BOYFRIEND.json",
        "/assets/common/data/GIRLFRIEND.json",
        NULL,
        0,
        "RETURN TO THE BIOS"
    );
    preloadcache_clear();
    // weekselector_main();
#ifdef MALLOC_CHK_ENABLE
    mallocfilter_stats();
#endif
    logger_info("week_main() execution done, engine is terminated");
    arch_menu();
    return 0;
#endif

    introscreen_main();

    while (true) {
        if (startscreen_main()) break;
        while (mainmenu_main());
        if (background_menu_music) soundplayer_replay(background_menu_music);
    }

    return 1;
}

void main_helper_draw_loading_screen() {
    // if (!SETTINGS.show_loading_screen) return;

    if (!fs_file_exists(FUNKIN_LOADING_SCREEN_TEXTURE)) {
        return;
    }

    Texture texture = texture_init(FUNKIN_LOADING_SCREEN_TEXTURE);
    Sprite sprite = sprite_init(texture);
    sprite_set_draw_location(sprite, 0.0f, 0.0f);
    sprite_set_draw_size_from_source_size(sprite);

    // funkay texture aspect ratio is not 16:9 or 4:3
    imgutils_calc_resize_sprite(sprite, pvr_context.screen_width, pvr_context.screen_height, true, true);

    pvr_context_reset(&pvr_context);
    sprite_draw(sprite, &pvr_context);
    pvrctx_wait_ready();

    sprite_destroy_full(&sprite);
}

void main_helper_trigger_action_menu(Layout layout, const char* prefix, const char* name, bool selected, bool choosen) {
    if (!layout || !name) return;

    const char* suffix;

    if (choosen)
        suffix = "choosen";
    else if (selected)
        suffix = "selected";
    else
        suffix = "unselected";

    char* target;

    if (prefix)
        target = string_concat(5, prefix, "-", name, "-", suffix);
    else
        target = string_concat(3, name, "-", suffix);

    layout_trigger_any(layout, target);
    free_chk(target);
}

void main_helper_trigger_action_menu2(Layout layout, MenuManifest menu_manifest, int32_t index, const char* prefix, bool selected, bool choosen) {
    if (index < 0 || index >= menu_manifest->items_size) return;
    const char* name = (menu_manifest->items[index].name != NULL) ? menu_manifest->items[index].name : menu_manifest->items[index].text;
    main_helper_trigger_action_menu(layout, prefix, name, selected, choosen);
}

Layout main_helper_init_layout_suffixed(const char* src, bool check_exists) {
    int32_t idx = string_last_index_of_char(src, '.');
    if (idx < 0) idx = (int32_t)strlen(src);

    Layout layout;
    char* dreamcast_src = string_copy_and_insert(src, idx, "~dreamcast");

    if (fs_file_exists(dreamcast_src))
        layout = layout_init(dreamcast_src);
    else if (!check_exists || fs_file_exists(src))
        layout = layout_init(src);
    else
        layout = NULL;

    free_chk(dreamcast_src);
    return layout;
}

MenuManifest main_helper_init_menumanifest_suffixed(const char* src, bool check_exists) {
    int32_t idx = string_last_index_of_char(src, '.');
    if (idx < 0) idx = (int32_t)strlen(src);

    MenuManifest menumanifest;
    char* dreamcast_src = string_copy_and_insert(src, idx, "~dreamcast");

    if (fs_file_exists(dreamcast_src))
        menumanifest = menumanifest_init(dreamcast_src);
    else if (!check_exists || fs_file_exists(src))
        menumanifest = menumanifest_init(src);
    else
        menumanifest = NULL;

    free_chk(dreamcast_src);
    return menumanifest;
}


static void* main_thd_helper_spawn_wrapper(void* init_data) {
    void* (*routine)(void* param) = ((ThreadInitData*)init_data)->routine;
    void* params = ((ThreadInitData*)init_data)->param;

    // dispose "init_data" because was allocated in main_thd_helper_spawn
    free_chk(init_data);

    //
    // Initialize the filesystem for this thread and later execute the routine,
    // fs_destroy() is called when the thread ends (KallistiOS does this job).
    //
    // Note: This can not be implemented in javascript, because there no TLS support
    //

    fs_init_kdy();

    // now execute the routine
    return routine(params);
}

kthread_t* main_thd_helper_spawn(bool detached, AsyncThread routine, void* param) {
    ThreadInitData* init_data = malloc_chk(sizeof(ThreadInitData));
    malloc_assert(init_data, ThreadInitData);

    *init_data = (ThreadInitData){.routine = routine, .param = param};
    kthread_t* thread = thd_create(detached, main_thd_helper_spawn_wrapper, init_data);

    assert(thread);

    thd_pass();
    return thread;
}

void* main_spawn_coroutine(Layout background_layout, AsyncThread function_routine, void* argument_routine) {
    kthread_t* thd = main_thd_helper_spawn(false, function_routine, argument_routine);
    void* ret;

    if (background_layout) {
        do {
            float elapsed = pvrctx_wait_ready();
            pvr_context_reset(&pvr_context);

            layout_animate(background_layout, elapsed);
            layout_draw(background_layout, &pvr_context);
        } while (thd->state != STATE_FINISHED);


        // aquire the return value and destroy the thread struct
        ret = thd->rv;
        thd_destroy(thd);
    } else {
        // no layout specified, wait for thread end
        thd_join(thd, &ret);
    }

    return ret;
}


static void load_settings_from_funkinsave() {
    SETTINGS.input_offset = funkinsave_get_setting_int(SETTINGS_ID_INPUT_OFFSET, SETTINGS.input_offset);
    SETTINGS.inverse_strum_scroll = funkinsave_get_setting_bool(SETTINGS_ID_INVERSE_STRUM_SCROLL, SETTINGS.inverse_strum_scroll);
    SETTINGS.penality_on_empty_strum = funkinsave_get_setting_bool(SETTINGS_ID_PENALITY_ON_EMPTY_STRUM, SETTINGS.penality_on_empty_strum);
    SETTINGS.use_funkin_marker_duration = funkinsave_get_setting_bool(SETTINGS_ID_USE_FUNKIN_MARKER_DURATION, SETTINGS.use_funkin_marker_duration);
    SETTINGS.song_progressbar = funkinsave_get_setting_bool(SETTINGS_ID_SONG_PROGRESSBAR, SETTINGS.song_progressbar);
    SETTINGS.song_progressbar_remaining = funkinsave_get_setting_bool(SETTINGS_ID_SONG_PROGRESSBAR_REMAINING, SETTINGS.song_progressbar_remaining);
    SETTINGS.gameplay_enabled_distractions = funkinsave_get_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_DISTRACTIONS, SETTINGS.gameplay_enabled_distractions);
    SETTINGS.gameplay_enabled_flashinglights = funkinsave_get_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_FLASHINGLIGHTS, SETTINGS.gameplay_enabled_flashinglights);
    SETTINGS.gameplay_enabled_uicosmetics = funkinsave_get_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_UICOSMETICS, SETTINGS.gameplay_enabled_uicosmetics);
}

static void save_settings_from_funkinsave() {
    funkinsave_set_setting_int(SETTINGS_ID_INPUT_OFFSET, SETTINGS.input_offset);
    funkinsave_set_setting_bool(SETTINGS_ID_INVERSE_STRUM_SCROLL, SETTINGS.inverse_strum_scroll);
    funkinsave_set_setting_bool(SETTINGS_ID_PENALITY_ON_EMPTY_STRUM, SETTINGS.penality_on_empty_strum);
    funkinsave_set_setting_bool(SETTINGS_ID_USE_FUNKIN_MARKER_DURATION, SETTINGS.use_funkin_marker_duration);
    funkinsave_set_setting_bool(SETTINGS_ID_SONG_PROGRESSBAR, SETTINGS.song_progressbar);
    funkinsave_set_setting_bool(SETTINGS_ID_SONG_PROGRESSBAR_REMAINING, SETTINGS.song_progressbar_remaining);
    funkinsave_set_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_DISTRACTIONS, SETTINGS.gameplay_enabled_distractions);
    funkinsave_set_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_FLASHINGLIGHTS, SETTINGS.gameplay_enabled_flashinglights);
    funkinsave_set_setting_bool(SETTINGS_ID_GAMEPLAY_ENABLED_UICOSMETICS, SETTINGS.gameplay_enabled_uicosmetics);
}

EngineSettings SETTINGS = {
    .input_offset = 0,
    .inverse_strum_scroll = false,
    .penality_on_empty_strum = true,
#ifdef _arch_dreamcast
    .use_funkin_marker_duration = true,
#else
    .use_funkin_marker_duration = false,
#endif
    .song_progressbar = true,
    .song_progressbar_remaining = false,
    .gameplay_enabled_distractions = true,
    .gameplay_enabled_flashinglights = true,
    .gameplay_enabled_uicosmetics = true,
    //.style_from_week_name = NULL

    .reload_settings = load_settings_from_funkinsave,
    .save_settings = save_settings_from_funkinsave
};

static void main_check_for_expansion() {
    const char* EXPANSION_TXT = "/~expansions/expansion.txt";
    if (!fs_file_exists(EXPANSION_TXT)) return;

    char* expansion = fs_readtext("/~expansions/expansion.txt");
    if (!expansion) goto L_return;

    // just in case
    int32_t idx = string_index_of_any_char(expansion, "\r\n");
    if (idx >= 0) expansion[idx] = '\0';

    if (fs_is_invalid_filename(expansion)) goto L_return;

    logger_info("loading expansion: %s", expansion);
    expansions_load(expansion);

L_return:
    free_chk(expansion);
}


#ifdef FS_ISO9660_DMA_SUPPORT

#define READ_ATA_REGISTER(REG) *((vuint8*)(0xA05F7000 + REG))
#define WRITE_ATA_REGISTER(REG, VALUE) *((vuint8*)(0xA05F7000 + REG)) = VALUE

static bool main_check_ata_devices_in_g1_bus() {
    // backup DEVICE_SELECT register value
    uint8_t orig_device_selected = READ_ATA_REGISTER(0x98);

    // write DEVICE_SELECT register
    WRITE_ATA_REGISTER(0x98, 0xF0); // slave device
    timer_spin_sleep(1);

    // clear unnecessary registers
    WRITE_ATA_REGISTER(0x88, 0); // sector count
    WRITE_ATA_REGISTER(0x8C, 0); // LBA low
    WRITE_ATA_REGISTER(0x90, 0); // LBA middle
    WRITE_ATA_REGISTER(0x94, 0); // LBA high

    // write IDENTIFY into COMMAND register
    WRITE_ATA_REGISTER(0x9C, 0xEC);
    timer_spin_sleep(1);

    // read STATUS register
    uint8_t status = READ_ATA_REGISTER(0x9C);

    // restore DEVICE_SELECT register value
    WRITE_ATA_REGISTER(0x98, orig_device_selected);

    return status == 0 || status == 0xFF;
}
#endif
