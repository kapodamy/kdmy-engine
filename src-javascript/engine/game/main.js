"use strict";

// this file contains all shared data across the game
const ENGINE_NAME = "kdmy-engine";
const ENGINE_VERSION = "0.58";


/**
 * The background music used in all menus, inherited from introscreen
 */
var background_menu_music = null;

/**
 * Contains all weeks definitions found
 */
var weeks_array = { array: null, size: -1 };

/**
 * Menus style (customized by a week) used across the game
 */
var custom_style_from_week = null;

var DEBUG = 0;
var SETTINGS = { // not implemented
    input_offset: 0, inverse_strum_scroll: false, penality_on_empty_strum: true, use_funkin_marker_duration: true,
    song_progressbar: true, song_progressbar_remaining: false,
    gameplay_enabled_distractions: true, gameplay_enabled_flashinglights: true, gameplay_enabled_ui_cosmetics: true,
    show_loading_screen: true
};


const FUNKIN_BACKGROUND_MUSIC = "/assets/common/music/freakyMenu.ogg";
const FUNKIN_SCREEN_RESOLUTION_WIDTH = 1280;
const FUNKIN_SCREEN_RESOLUTION_HEIGHT = 720;
const FUNKIN_LOADING_SCREEN_TEXTURE = "/assets/common/image/funkin/funkay.png";



async function main(argc, argv) {
    // vital parts
    fs_init();// intialize filesystem access for the main thread (this thread)
    luascriptplatform.InitializeCallbacks();
    await mastervolume_init();

    if (!(await io_resource_exists(FS_ASSETS_FOLDER, 0, 1))) {
        alert("Missing assets folder");
        return 1;
    }

    /**@type {utsname}*/
    let osinfo = { machine: null, nodename: null, release: null, sysname: null, version: null };
    uname(osinfo);
    console.info(`${ENGINE_NAME} ${ENGINE_VERSION}`);
    console.info(`Console: ${osinfo.machine}`);
    console.info(`OS: ${osinfo.version}\r\n`);

    //await www_autoplay();

    // (JS & CS only) preload fonts
    await Promise.all([
        fontholder_init("/assets/common/font/vcr.ttf", -1, null),
        //fontholder_init("/assets/common/font/pixel.otf", -1, null),
        fontholder_init("/assets/common/font/alphabet.xml", -1, null)
    ]);


    await weekenumerator_enumerate();

    if (weeks_array.size < 1) console.warn("main() no weeks detected");

    // load savedata
    funkinsave_pick_first_available_vmu();
    let load_result = await savemanager_should_show(0);
    if (load_result) {
        let savemanager = await savemanager_init(0, load_result);
        await savemanager_show(savemanager);
        savemanager_destroy(savemanager);
    }

    // choose a random menu style
    let try_choose_last_played = Math.random() <= 0.25;
    let visited = new Array(weeks_array.size);
    let visited_count = 0;
    for (let i = 0; i < weeks_array.size; i++) visited[i] = 0;

    while (visited_count < weeks_array.size) {
        let weekinfo;

        if (try_choose_last_played) {
            try_choose_last_played = false;
            weekinfo = null;
            const last_played = funkinsave_get_last_played_week();

            if (last_played != null) {
                for (let i = 0; i < weeks_array.size; i++) {
                    if (weeks_array.array[i].name == last_played) {
                        ;
                        visited[i] = 1;
                        visited_count++;
                        weekinfo = weeks_array.array[i];
                    }
                }
            }
            if (weekinfo == null) continue;
        } else {
            let random_index = math2d_random_int(0, weeks_array.size - 1);
            if (visited[random_index]) continue;
            visited[random_index] = 1;
            visited_count++;
            weekinfo = weeks_array.array[random_index];
        }

        if (!weekinfo.custom_folder && !weekinfo.has_greetings) continue;

        if (funkinsave_contains_unlock_directive(weekinfo.unlock_directive)) {
            custom_style_from_week = weekinfo;

            // override commons files (if possible)
            if (weekinfo.custom_folder) {
                fs_override_common_folder(weekinfo.custom_folder);
            }
            break;
        }
    }

    background_menu_music = await soundplayer_init(FUNKIN_BACKGROUND_MUSIC);
    if (background_menu_music) soundplayer_loop_enable(background_menu_music, 1);

    /*await week_main(
        weeks_array.array[7],
        0,
        "NORMAL",
        "/assets/common/data/BOYFRIEND.json",
        "/assets/common/data/GIRLFRIEND.json",
        null,
        -1
    );
    console.info("week_main() execution done, engine is terminated");
    return 0;*/

    //await weekselector_main();


    await introscreen_main();

    while (1) {
        if (await startscreen_main()) break;
        while (await mainmenu_main());
        soundplayer_replay(background_menu_music);
    }

    return 1;
}

async function main_helper_draw_loading_screen() {
    if (!SETTINGS.show_loading_screen) return;

    if (!await fs_file_exists(FUNKIN_LOADING_SCREEN_TEXTURE)) {
        return;
    }

    let texture = await texture_init(FUNKIN_LOADING_SCREEN_TEXTURE);
    let sprite = sprite_init(texture);
    sprite_set_draw_location(sprite, 0.0, 0.0);
    sprite_set_draw_size_from_source_size(sprite);

    // funkay texture aspect ratio is not 16:9 or 4:3
    imgutils_calc_resize_sprite(sprite, pvr_context.screen_width, pvr_context.screen_height, 1, 1);

    pvr_context_reset(pvr_context);
    sprite_draw(sprite, pvr_context);
    await pvrctx_wait_ready();

    sprite_destroy_full();
}

function main_helper_trigger_action_menu(layout, prefix, name, selected, choosen) {
    if (!layout || !name) return;

    let suffix;

    if (choosen)
        suffix = "choosen";
    else if (selected)
        suffix = "selected";
    else
        suffix = "unselected";

    let target;

    if (prefix)
        target = string_concat(5, prefix, "-", name, "-", suffix);
    else
        target = string_concat(3, name, "-", suffix);

    layout_trigger_any(layout, target);
    target = undefined;
}

function main_helper_trigger_action_menu2(layout, menu_manifest, index, prefix, selected, choosen) {
    if (index < 0 || index >= menu_manifest.items_size) return;
    let name = menu_manifest.items[index].name ?? menu_manifest.items[index].text;
    main_helper_trigger_action_menu(layout, prefix, name, selected, choosen);
}

function main_get_weekinfo_from_week_name(week_name) {
    for (let i = 0; i < weeks_array.size; i++) {
        if (weeks_array.array[i].name == week_name)
            return weeks_array.array[i];
    }
    return null;
}


async function main_spawn_coroutine(background_layout, function_routine, argument_routine) {
    //
    // Important: In JS, all calls to fs_* engine functions will fail, because there no
    // TLS support.
    //
    let thd = thd_helper_spawn(function_routine, argument_routine);
    let ret;

    if (background_layout) {
        while (thd.state != STATE_FINISHED) {
            let elapsed = await pvrctx_wait_ready();
            pvr_context_reset(pvr_context);

            layout_animate(background_layout, elapsed);
            layout_draw(background_layout, pvr_context);
        }

        // aquire the return value and destroy the thread struct
        ret = thd.rv;
        thd_destroy(thd);
    } else {
        // no layout specified, wait for thread end
        let tmp = [null];
        await thd_join(thd, tmp);
        ret = tmp[0];
    }

    return ret;
}



async function main_tests(argc, argv) {
    console.log("FNF tests mode");
    //    await www_autoplay();

    fs_init();

    /*let layout = await layout_init("/assets/test/layout_test.xml");
    window["layout"] = layout;
    window["group"] = layout.group_list[4];

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
    }*/


    /*let texture = await texture_init("/assets/common/image/week-selector/dreamcast_buttons.png");
    let sprite = sprite_init(texture);
    sprite_set_draw_size_from_source_size(sprite);
    sprite_set_draw_location(sprite, 100, 100);
    let sprite2 = sprite_init(texture);
    sprite_set_draw_size_from_source_size(sprite2);
    sprite_set_draw_location(sprite2, -100, -100);
    let sprite3 = sprite_init(texture);
    sprite_set_draw_size_from_source_size(sprite3);
    sprite_set_draw_location(sprite3, 300, 300);

    PSFramebuffer.ResizeQuadScreen(pvr_context);

    let a = `#pragma header
        //a
        void main() { 
            vec4 color = texture(Texture, TexCoord);
            FragColor = vec4(color.r, color.g*0.5, color.b, color.a);
        }
    `;
    let b = `#pragma header
        //b
        void main() { 
            vec4 color = texture(Texture, TexCoord);
            FragColor = vec4(color.b, color.g/0.5, color.r, color.a);
        }
`;
    let c = `#pragma header
        // c
        void main() { 
            vec4 color = texture(Texture, TexCoord);
            FragColor = vec4(color.b, color.g, color.r, color.a);
        }
`;
let d = `#pragma header
    // d
    void main() { 
        vec4 color = texture(Texture, TexCoord);
        FragColor = vec4(vec3(1.0) - color.rgb, color.a);
    }
`;

    let framebuffer1 = new PSFramebuffer(pvr_context);
    let framebuffer2 = new PSFramebuffer(pvr_context);
    let ps_a = PSShader.BuildFromSource(pvr_context, null, a);
    let ps_b = PSShader.BuildFromSource(pvr_context, null, b);
    let ps_c = PSShader.BuildFromSource(pvr_context, null, c);
    let ps_d = PSShader.BuildFromSource(pvr_context, null, d);
    if (!ps_a || !ps_b || !ps_c || !ps_d) return;

    let frame_count = 0;
    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);
        sprite_animate(sprite, elapsed);

        
        sprite_draw(sprite2, pvr_context);
        pvr_context_save(pvr_context);
        pvr_context_add_shader(pvr_context, ps_a);
        //pvr_context_add_shader(pvr_context, ps_b);
        //pvr_context_add_shader(pvr_context, ps_c);

        //pvr_context_draw_solid_color(pvr_context, [1, 0, 0], 0, 0, pvr_context.screen_width, pvr_context.screen_height);
        //pvr_context_draw_solid_color(pvr_context, PVR_CLEAR_COLOR, 1, 1, 638, 478);
        sprite_draw(sprite, pvr_context);

        pvr_context_restore(pvr_context);

        
        pvr_context_add_shader(pvr_context, ps_d);
        
        sprite_draw(sprite3, pvr_context);
    }

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);
        sprite_animate(sprite, elapsed);

        framebuffer1.Use();
        pvr_context_draw_solid_color(pvr_context, [1, 0, 0], 0, 0, pvr_context.screen_width, pvr_context.screen_height);
        pvr_context_draw_solid_color(pvr_context, PVR_CLEAR_COLOR, 1, 1, 638, 478);
        sprite_draw(sprite, pvr_context);

        framebuffer2.Use();
        ps_a.Draw(framebuffer1);

        framebuffer1.Use();
        ps_b.Draw(framebuffer2);

        PSFramebuffer.UseScreenFramebuffer(pvr_context);
        ps_c.Draw(framebuffer1);

        frame_count++;
    }*/


    //await weekenumerator_enumerate();

    /*let fontholder = await fontholder_init("/assets/test/Lato-Regular.ttf");
    let textsprite = textsprite_init2(fontholder, 24, 0x000000);
    textsprite_set_draw_location(textsprite, 150, 100);
    textsprite_set_max_draw_size(textsprite, 293, 172);
    textsprite_set_wordbreak(textsprite, FONT_WORDBREAK_LOOSE);
    textsprite_set_text(textsprite,
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean accumsan porta efficitur. " +
        "Quisque a nisi scelerisque.\n" +
        "Aenean accumsan porta efficitur."
    );

    pvr_context_draw_solid_color(pvr_context, [0.9, 0.9, 0.9], 0, 0, pvr_context.screen_width, pvr_context.screen_height);
    pvr_context_draw_solid_color(pvr_context, [1, 1, 1], 150, 100, 293, 172);
    textsprite_draw(textsprite, pvr_context);
    return;*/


    /*
        let layout_lua_test = await layout_init("/assets/weeks/week5/christmas/layout.xml");
        let rndctx = { layout: layout_lua_test };
        let luascript = await weekscript_init("/assets/test/test_placeholder.lua", rndctx, 1);
        return;
    
    
        let manifest_boyfriend = await charactermanifest_init("/assets/common/data/BOYFRIEND.json", 1);
        let character_boyfriend = await character_init(manifest_boyfriend);
    
        character_set_bpm(character_boyfriend, 100);
        character_set_draw_location(character_boyfriend, 200, pvr_context.screen_height);
        character_set_z_index(character_boyfriend, 200);
        //character_set_draw_align(character_boyfriend, ALIGN_CENTER, ALIGN_BOTH);
        //character_set_layout_resolution(character_boyfriend, pvr_context.screen_width, pvr_context.screen_height);
        //character_set_offset(character_boyfriend, manifest_boyfriend.offset_x, manifest_boyfriend.offset_y);
        //character_set_idle_speed(character_boyfriend, 1.0);
        //character_use_alternate_sing_animations(character_boyfriend, 0);
        //character_face_as_opponent(character_boyfriend, 0);
        //character_set_draw_align(character_boyfriend, ALIGN_START, ALIGN_START);
    
        window["character"] = character_boyfriend;
        let character_timestamp = 0;
    
        let test_sing_case_offset = 137461;
        let test_sing_case = [
            [139461.59999990463, 0, ""],
            [139477.89999997616, 0, ""],
            [140361, 1, "up"],
            [140660.79999995232, 1, "left"],
            [140877.19999992847, 0, ""],
            [140960.5, 2, "down"],
            [141176.89999997616, 0, ""],
            [141859.09999990463, 0, ""],
            [141875.89999997616, 0, ""],
        ];
        let test_sing_case_index = 0;
    
        while (1) {
            let elapsed = await pvr_wait_ready();
            pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);
    
            if (test_sing_case_index < test_sing_case.length) {
                let sing = test_sing_case[test_sing_case_index];
                if ((character_timestamp + test_sing_case_offset) >= sing[0]) {
                    switch (sing[1]) {
                        case STRUM_PRESS_STATE_NONE:
                            character_play_idle(character_boyfriend);
                            break;
                        case STRUM_PRESS_STATE_HIT:
                            character_play_sing(character_boyfriend, sing[2], 0);
                            break;
                        case STRUM_PRESS_STATE_HIT_SUSTAIN:
                            character_play_sing(character_boyfriend, sing[2], 1);
                            break;
                    }
                    test_sing_case_index++;
                }
            }
    
            beatwatcher_global_set_timestamp(character_timestamp);
            character_timestamp += elapsed;
    
    
    
            character_animate(character_boyfriend, elapsed);
            character_draw(character_boyfriend, pvr_context);
        }
    
        return;
    
        let save_error = await main_spawn_coroutine(
            await layout_init("/assets/common/image/start-screen/layout.xml"),
            async function (arg) {
                console.log("arg was", arg);
                await thd_sleep(2500);
                return "test completed";
            },
            123
        );
        console.log(save_error);
        return;
    
        let savemanager = await savemanager_init(0, -1);
        await savemanager_show(savemanager);
        savemanager_destroy(savemanager);
        return;
    
    
        linkedlist_add_item(funkinsave.weeks_names, "week1");
        linkedlist_add_item(funkinsave.weeks_names, "week2");
        linkedlist_add_item(funkinsave.weeks_names, "week3");
        linkedlist_add_item(funkinsave.weeks_names, "week4");
    
        linkedlist_add_item(funkinsave.difficulty_names, "EASY");
        linkedlist_add_item(funkinsave.difficulty_names, "NORMAL");
        linkedlist_add_item(funkinsave.difficulty_names, "HARD");
    
        linkedlist_add_item(funkinsave.progress, { week_id: 2, difficulty_id: 0, score: 0x12345678 });
        linkedlist_add_item(funkinsave.progress, { week_id: 2, difficulty_id: 1, score: 0xabcdef });
        linkedlist_add_item(funkinsave.progress, { week_id: 0, difficulty_id: 1, score: 0xaabbccdd });
        linkedlist_add_item(funkinsave.progress, { week_id: 2, difficulty_id: 2, score: 0x11223344 });
    
        linkedlist_add_item(funkinsave.storages, { week_id: 3, name: "ten", data_size: 10, data: new Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]) });
        linkedlist_add_item(funkinsave.storages, { week_id: 3, name: "one", data_size: 1, data: new Uint8Array([10]) });
        linkedlist_add_item(funkinsave.storages, { week_id: 1, name: "abc", data_size: 3, data: new Uint8Array([0x61, 0x62, 0x63]) });
    
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "TTL", value: 0x00 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "WEEK1!", value: 0x1234 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "WEEK2!", value: 123.4567 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "WEEK2!", value: 123.4567 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "ABCDEFGHIJKLMNÑOPQ", value: 1 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "ABC0123456789abcdefghijk01234abc", value: 456 });
        linkedlist_add_item(funkinsave.directives, { type: 0, name: "ABC0123456789abcdefghijk01234abc¡¡CUT!!", value: 999 });
    
        linkedlist_add_item(funkinsave.settings, { id: 0x789, value: 0 });
        linkedlist_add_item(funkinsave.settings, { id: 0x123, value: 0x456 });
        linkedlist_add_item(funkinsave.settings, { id: 0xabcd, value: 789.456 });
    
        await funkinsave_write_to_vmu();
        await funkinsave_read_from_vmu();
        return;
    */

    /*
    let layout_test = await layout_init("/assets/common/image/week-selector/layout.xml");
    while (1) {
        pvr_context_reset(pvr_context);
        let elapsed = await pvr_wait_ready();
        //layout_animate(layout_test, elapsed*0.1);
        layout_draw(layout_test, pvr_context);
        pvr_context.render_alpha = 0.8;
        pvr_context_draw_solid_color(pvr_context, [0, 0, 0], 0, 0, pvr_context.screen_width, pvr_context.screen_height);
    }
    return;
    */

    /*window["sfcp"] = String.fromCodePoint;
    let test_case_string = "ὖ ŉ Åström ḱṷṓn ab12BC!#$%○▀È╚|♥ÊôF►♫@";
    console.log("string_to_lowercase", test_case_string, string_to_lowercase(test_case_string));
    console.log("string_to_upercasecase", test_case_string, string_to_uppercase(test_case_string));

    return;*/
    /*
        pvr_context_save(pvr_context);
    
        sh4matrix_rotate_by_degs(pvr_context.current_matrix, -15);
        sh4matrix_translate(pvr_context.current_matrix, 100, 100)
        sh4matrix_scale(pvr_context.current_matrix, 1, 2);
        sh4matrix_skew(pvr_context.current_matrix, -0.2, 0);
    
        let test_string = "ABC EFHGJIK\nRTYUIOPVBNM";
        let test_height = 48;
        let fontglyph = await fontglyph_init("/assets/common/font/alphabet.xml", "bold", 1);
        fontglyph_enable_color_by_difference(fontglyph, 0);
        fontglyph_enable_border(fontglyph, 0);
        fontglyph_set_border_size(fontglyph, 4);
        fontglyph_set_border_color(fontglyph, 1.0, 0.0, 0.0, 1.0);
        //  let string_width = fontglyph_measure(fontglyph, test_height, test_string, 0, test_string.length);
        //  let string_height = fontglyph_draw_text(fontglyph, pvr_context, test_height, 10, 10, 0, test_string.length, test_string);
    
        let fonttype = await fonttype_init("/assets/common/font/vcr.ttf");
        let string_width = fonttype_measure(fonttype, test_height, test_string, 0, test_string.length);
        let string_height = fonttype_draw_text(fonttype, pvr_context, test_height, 0, 0, 0, test_string.length, test_string);
    
    
        //webopengl_draw_solid(pvr_context, pvr_context.webopengl, [0.5, 0.0, 0.25], 0, 0, 500, 100);
    
    
        return;
        */

    /*
    let ui_layout = await layout_init("/assets/common/image/week-round/ui~dreamcast.xml");
    let weekresult = week_result_init();
    week_result_helper_show_summary(weekresult, { layout: ui_layout }, 123, -1);
    return;
    */

    // @ts-ignore
    let messagebox = await messagebox_init();
    /*messagebox_set_buttons_text(messagebox, "Retry again", "Keep");
    //messagebox_set_button_single(messagebox, "Close");
    messagebox_set_title(messagebox, "Insert title");
    messagebox_set_message(messagebox, "test message 123abc");
    await messagebox_set_image_from_texture(messagebox, "/assets/common/image/week-round/no_controller.png");
    messagebox_show(messagebox, 1);*/

    /*await messagebox_set_image_from_texture(messagebox, "/assets/common/image/week-round/no_controller.png");
    messagebox_set_button_single(messagebox, "(Waiting controller)");
    messagebox_set_title(messagebox, "Controller disconnected");
    messagebox_set_message(messagebox, "abc123");
    messagebox_show_buttons_icons(messagebox, false);
    messagebox_use_small_size(messagebox, false);
    messagebox_hide_image(messagebox, false);
    messagebox_show(messagebox, true);*/

    messagebox_hide_image(messagebox, true);
    messagebox_set_buttons_icons(messagebox, "a", "b");
    messagebox_set_buttons_text(messagebox, "Ok", "Cancel");
    messagebox_set_title(messagebox, "Confirm");
    messagebox_use_small_size(messagebox, true);
    messagebox_set_message(messagebox, "¿leave?");
    messagebox_show(messagebox, false);

    window["messagebox"] = messagebox;

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        messagebox_animate(messagebox, elapsed);
        messagebox_draw(messagebox, pvr_context);
    }
    return;


    /*
    let layout_msg = await layout_init("/assets/common/image/messagebox.xml");
    while (1) {
        let elapsed = await pvr_wait_ready();
        pvr_context_reset(pvr_context);
        layout_animate(layout_msg, elapsed);
        layout_draw(layout_msg, pvr_context);
    }
    return;

    let gamepad = gamepad_init2(0, 1);
    let binds = [GAMEPAD_DPAD_UP, GAMEPAD_DPAD_DOWN, GAMEPAD_DPAD_LEFT, GAMEPAD_DPAD_RIGHT];
    let ddrkeysmon = ddrkeymon_init(gamepad, binds, binds.length);
    let ddrkeys_fifo = ddrkeymon_get_fifo(ddrkeysmon);
    ddrkeymon_start(ddrkeysmon, 0);
    while (1) {
        await pvr_wait_ready();
        if (ddrkeys_fifo.available > 0) {
            for (let i = 0; i < ddrkeys_fifo.available; i++) {
                console.log(`holding=${ddrkeys_fifo.queue[i].holding} button=${ddrkeys_fifo.queue[i].button.toString(16)} timestamp==${ddrkeys_fifo.queue[i].in_song_timestamp}`);
                ddrkeys_fifo.queue[i].discard = 1;
            }
            ddrkeymon_purge(ddrkeys_fifo);
        }
    }
    return;
    const MENU_MANIFEST = {
        parameters: {
            suffix_selected: null,// unused
            suffix_idle: null,// unused
            suffix_choosen: null,// unused
            suffix_discarded: null,// unused
            suffix_rollback: null,// unused
            suffix_in: null,// unused
            suffix_out: null,// unused

            atlas: null,
            animlist: "/assets/common/anims/week-round-menu.xml",

            anim_selected: "selected",
            anim_idle: "idle",
            anim_choosen: null,
            anim_discarded: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused

            anim_transition_in_delay: 0,
            anim_transition_out_delay: -100,// negative means start delays from the bottom

            font: "/assets/common/font/alphabet.xml",// unused
            font_glyph_suffix: "bold",// unused
            font_color_by_difference: 0,// unused
            font_size: 48,// unused
            font_color: 0x00,// unused
            font_border_color: 0x00,// unused
            font_border_size: NaN,// unused

            is_sparse: 0,// unused
            is_vertical: 1,// readed from the layout
            static_index: 1,

            items_align: ALIGN_START,// readed from the layout
            items_gap: 48,// readed from the layout
            items_dimmen: 0,// readed from the layout
            per_page: 0,
            texture_scale: NaN// unused
        },
        items: [
            {
                name: null,
                text: "RESUME",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
                anim_selected: null,// unused
                anim_choosen: null,// unused
                anim_discarded: null,// unused
                anim_idle: null,// unused
                anim_rollback: null,// unused
                anim_in: null,// unused
                anim_out: null,// unused
            },
            {
                name: null,
                text: "RESTART SONG",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
                anim_selected: null,// unused
                anim_choosen: null,// unused
                anim_discarded: null,// unused
                anim_idle: null,// unused
                anim_rollback: null,// unused
                anim_in: null,// unused
                anim_out: null,// unused
            },
            {
                name: null,
                text: "EXIT TO MENU",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
                anim_selected: null,// unused
                anim_choosen: null,// unused
                anim_discarded: null,// unused
                anim_idle: null,// unused
                anim_rollback: null,// unused
                anim_in: null,// unused
                anim_out: null,// unused
            },
            {
                text: "OPTION A",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
            {
                text: "OPTION B",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
            {
                text: "OPTION C",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
            {
                text: "OPTION D",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
            {
                text: "OPTION E",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
            {
                text: "OPTION F LAST",// unused
                placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            },
        ],
        items_size: 9
    };
    let menu = await menu_init(MENU_MANIFEST, 10, 100, 1, pvr_context.screen_width, 300);
    window["menu"] = menu;
    menu_select_index(menu, 0);

    while (1) {
        let elapsed = await pvr_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);
        pvr_context_draw_solid_color(pvr_context, "#AAAAAAAA", 10, 100, pvr_context.screen_width, 300);

        menu_animate(menu, elapsed);
        menu_draw(menu, pvr_context);
    }
    return;

    let gameover = await week_gameover_init();
    week_gameover_display(gameover, 5000, 60000, playerstats_init(), weeks_array.array[1], "NORMAL");
    await week_gameover_display_selector(gameover);
    week_gameover_display_choosen_difficult(gameover);

    while (1) {
        let elapsed = await pvr_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        week_gameover_animate(gameover, elapsed);
        week_gameover_draw(gameover, pvr_context);
    }
    return;


    let layout = await layout_init("/assets/common/image/week-round/gameover.xml");
    window["layout"] = layout;
    window["camera"] = layout_get_camera_helper(layout);

    let keys = {
        scale: 0,
        translate_x: 0,
        translate_y: 0,
        start: 0,
    }

    document.addEventListener("keydown", function (e) {
        switch (e.key) {
            case "ArrowDown":
                keys.translate_y = -2;
                return;
            case "ArrowUp":
                keys.translate_y = +2;
                return;
            case "ArrowLeft":
                keys.translate_x = +2;
                return;
            case "ArrowRight":
                keys.translate_x = -2;
                return;
            case "+":
                keys.scale = +0.005;
                return;
            case "-":
                keys.scale = -0.005;
                return;
            case "Enter":
                keys.start = 1;
                return;
        }

    }, false);
    document.addEventListener("keyup", function (e) {
        switch (e.key) {
            case "ArrowDown":
            case "ArrowUp":
                keys.translate_y = 0;
                return;
            case "ArrowLeft":
            case "ArrowRight":
                keys.translate_x = 0;
                return;
            case "+":
            case "-":
                keys.scale = 0;
                return;
            case "Enter":
                keys.start = 0;
                return;
        }

    }, false);

    while (1) {
        let elapsed = await pvr_wait_ready();
        pvr_context_reset(pvr_context);
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        window["camera"].modifier.translate_x += keys.translate_x;
        window["camera"].modifier.translate_y += keys.translate_y;
        window["camera"].modifier.scale_x += keys.scale;
        window["camera"].modifier.scale_y += keys.scale;

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
    }

    return;
        let selector_layout = await layout_init("/assets/common/image/week-selector/layout.xml");
        let mdl = await modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
        let animlist = await animlist_init("/assets/common/image/week-selector/ui_anims.xml");
        let test = await weekselector_mdlselect_init(animlist, mdl, selector_layout, 0);
    
        window["mdlselect"] = test;
        weekselector_mdlselect_enable_arrows(test, 1);
        let timeout = 100;
    
        while (1) {
            let elapsed = await pvr_wait_ready();
            if (timeout > 0) {
                timeout -= elapsed;
                continue;
            }
    
            pvr_context_reset(pvr_context);
    
            layout_animate(selector_layout, elapsed);
            layout_draw(selector_layout, pvr_context);
        }
    
        return;
    
        let test_cases = [
            ["/", "/"],
            ["/a", "/a"],
            ["/a/", "/a"],
            [".", ""],
            ["./", ""],
            [".a/", ".a"],
            [".///", ""],
            ["./a", "a"],
            [".////a", "a"],
            ["./a/", "a"],
            [".///a///", "a"],
            ["/a/./", "/a"],
            ["/a/./b", "/a/b"],
            ["../", "/"],
            ["../a", "/"],
            ["../..", "/"],
            ["/a/./b/..", "/a"],
            ["/a/./b/../..", "/"],
            ["/a/./b/../../", "/"],
            ["/a/./b/../../c", "/c"],
            ["a///////b", "a/b"],
            ["a/../a/b", "a/b"],
            ["//////", "/"],
            ["////a", "/a"],
            ["a////b///", "a/b"],
            ["a////", "a"],
            ["///a////", "/a"],
            ["///a//..///b///c/", "/b/c"]
        ];
    
        for (let i = 0; i < test_cases.length; i++) {
            let result = fs_resolve_path(test_cases[i][0]);
            if (result == test_cases[i][1])
                console.log(`PASS #${test_cases[i][0]}#  #${result}#`);
            else
                console.log(`FAIL #${test_cases[i][0]}#  #${result}#  expected=#${test_cases[i][1]}#`);
        }
    
    
        window.run = 1;
    
        const test_menu = {
            parameters: {
                suffix_selected: "selected",
                suffix_choosen: "choosen",
                suffix_discarded: "discarded",
                suffix_idle: "idle",
                suffix_rollback: "rollback",
                suffix_in: "in",
                suffix_out: "out",
    
                atlas: "/assets/common/image/main-menu/menuItems.xml",
                animlist: "/assets/common/image/main-menu/animations.xml",
    
                anim_selected: "selected",
                anim_choosen: "choosen",
                anim_discarded: null,
                anim_idle: "idle",
                anim_rollback: "rollback",
    
                anim_in: null,
                anim_out: null,
                transition_delay: 100,
    
                is_vertical: true,
    
                font: "/assets/common/font/vcr.ttf",
                font_color_by_difference: true,
                font_size: 18,
                font_color: 0xFFFFFF,
                font_border_color: 0x00000000,
                font_border_size: 2.5,
    
                align: ALIGN_CENTER,
                sparse: false,
                items_gap: 0,
                items_dimmen: 56,
                per_page: true,
                texture_scale: 0.25
            },
            items1: [
                { text: "0000", name: "storymode", placement: { x: 0, y: 10, dimmen: 0, gap: 0 } },
                { text: "1111", name: "storymode", placement: { x: 80, y: 60, dimmen: 0, gap: 0 } },
                { text: "2222", name: "freeplay", placement: { x: 120, y: 20, dimmen: 0, gap: 0 } },
                { text: "3333", name: "freeplay", placement: { x: 200, y: 70, dimmen: 0, gap: 0 } },
                { text: "4444", name: "donate", placement: { x: 280, y: 150, dimmen: 0, gap: 0 } },
                { text: "5555", name: "donate", placement: { x: 120, y: 140, dimmen: 0, gap: 0 } },
                { text: "6666", name: "options", placement: { x: 250, y: 80, dimmen: 0, gap: 0 } },
                { text: "7777", name: "options", placement: { x: 50, y: 90, dimmen: 0, gap: 0 } }
            ],
            items: [
                { text0: "0000", name: "storymode", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "1111", name: "storymode", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "2222", name: "freeplay", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "3333", name: "freeplay", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "4444", name: "donate", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "5555", name: "donate", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "6666", name: "options", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } },
                { text0: "7777", name: "options", placement: { x: 0, y: 0, dimmen: 0, gap: 0, scale: 0 } }
            ],
            items_size: 8
        }
    
        let menu = await menu_init(test_menu, 160, 120, 1, 320, 240);
        menu_select_index(menu, 0);
        let gamepad = gamepad_init(-1);
        gamepad_set_buttons_delay(gamepad, 200);
    
        while (1) {
            let elapsed = await pvr_wait_ready();
            pvr_context_clear_screen(pvr_context, "darkgray");
            pvr_context_reset(pvr_context);
    
            pvr_context_draw_solid_color(pvr_context, "#0000007F", 160, 120, 320, 240);
    
            if (gamepad_has_pressed_any_delayed(gamepad, GAMEPAD_DPAD_UP))
                menu_select_vertical(menu, 1);
            else if (gamepad_has_pressed_any_delayed(gamepad, GAMEPAD_DPAD_DOWN))
                menu_select_vertical(menu, 0);
            if (gamepad_has_pressed_any_delayed(gamepad, GAMEPAD_DPAD_LEFT))
                menu_select_horizontal(menu, 1);
            else if (gamepad_has_pressed_any_delayed(gamepad, GAMEPAD_DPAD_RIGHT))
                menu_select_horizontal(menu, 0);
            else if (gamepad_has_pressed_any_delayed(gamepad, GAMEPAD_START))
                menu_toggle_choosen(menu, !menu.item_choosen);
    
            menu_animate(menu, elapsed);
            menu_draw(menu, pvr_context);
        }
    
        let l = await layout_init("/assets/common/image/week-round/ui~dreamcast.xml")
        while (1) {
            let elapsed = await pvr_wait_ready();
            pvr_context_clear_screen(pvr_context, "darkgray");
            pvr_context_reset(pvr_context);
            layout_animate(l, elapsed);
            layout_draw(l, pvr_context);
        }
    
        let aa = await gameplaymanifest_init("/assets/weeks/tutorial/gameplay.json");
        gameplaymanifest_destroy(aa);
    
        let layout = await layout_init("/assets/test/layout_test.xml");
        window["layout"] = layout;
        if (!layout) {
            console.error("layout was null");
            return;
        }
    
        let keys = {
            scale: 0,
            translate_x: 0,
            translate_y: 0,
            start: 0,
        }
    
        document.addEventListener("keydown", function (e) {
            switch (e.key) {
                case "ArrowDown":
                    keys.translate_y = -2;
                    return;
                case "ArrowUp":
                    keys.translate_y = +2;
                    return;
                case "ArrowLeft":
                    keys.translate_x = +2;
                    return;
                case "ArrowRight":
                    keys.translate_x = -2;
                    return;
                case "+":
                    keys.scale = +0.005;
                    return;
                case "-":
                    keys.scale = -0.005;
                    return;
                case "Enter":
                    keys.start = 1;
                    return;
            }
    
        }, false);
        document.addEventListener("keyup", function (e) {
            switch (e.key) {
                case "ArrowDown":
                case "ArrowUp":
                    keys.translate_y = 0;
                    return;
                case "ArrowLeft":
                case "ArrowRight":
                    keys.translate_x = 0;
                    return;
                case "+":
                case "-":
                    keys.scale = 0;
                    return;
                case "Enter":
                    keys.start = 0;
                    return;
            }
    
        }, false);
    
        pvr_context_set_global_antialiasing(pvr_context, PVR_FLAG_DISABLE);
        //pvr_context_set_global_offsetcolor_multiply(pvr_context, PVR_FLAG_DISABLE);
        layout.modifier_camera.width = pvr_context.screen_width;
        layout.modifier_camera.height = pvr_context.screen_height;
        layout.modifier_camera.scale_direction_x = -0.5;
        layout.modifier_camera.scale_direction_y = -0.5;
    
        let wait_countdown = 0;
    
        while (window.run) {
            let elapsed = await pvr_wait_ready();
            pvr_context_clear_screen(pvr_context, "darkgray");
            let start = performance.now();
    
            layout_animate(layout, elapsed);
            layout_draw(layout, pvr_context);
    
            layout.modifier_camera.scale_x += keys.scale;
            layout.modifier_camera.scale_y += keys.scale;
            layout.modifier_camera.translate_x += keys.translate_x;
            layout.modifier_camera.translate_y += keys.translate_y;
    
            //pvr_context.html5canvasctx.lineWidth = 1;
            //pvr_context.html5canvasctx.moveTo(pvr_context.screen_width / 2, 0);
            //pvr_context.html5canvasctx.lineTo(pvr_context.screen_width / 2, pvr_context.screen_height);
            //pvr_context.html5canvasctx.stroke();
            //pvr_context.html5canvasctx.moveTo(0, pvr_context.screen_height / 2);
            //pvr_context.html5canvasctx.lineTo(pvr_context.screen_width, pvr_context.screen_height / 2);
            //pvr_context.html5canvasctx.stroke();
    
            let total = performance.now() - start;
            PVR_STATUS.value += `      ${total.toFixed(2)}ms`
        }
        return;
    */



    /*
        const chart = {
            strums_distribution: null,
            player_notes_size: 1,
            player_notes: [
                {
                    get notes_size() { return this.notes.length },
                    notes: new Array()
                }
            ]
        };
    
        const distribution = chart.strums_distribution || STRUMS_DISTRIBUTION_DEFAULT;
    
        let sections = (await fs_readjson("/assets/test/hey.json")).song.notes;
        for (let section of sections) {
            for (let note of section.sectionNotes) {
                let note_id = note[1];
                if (section.mustHitSection) {
                    if (note_id < 4) continue;
                    note_id -= 4;
                }
                chart.player_notes[0].notes.push([note[0], note_id, note[2], ""]);
            }
        }


    //if (strum_id < 0 || strum_id >= chart.strums_ids_size) return 0;
    //if (player_id < 0 || player_id >= chart.player_notes_size) return 0;

    window.run = 1;

    let boyfriend_moldelholder = await modelholder_init("/assets/common/image/funkin/icon-bf");
    let dad_moldelholder = await modelholder_init("/assets/weeks/week1/host/icon-dad");
    let warnings_modelholder = await modelholder_init("/assets/common/image/week-round/healthbar_warns");
    let ranking_modelholder = await modelholder_init("/assets/common/image/week-round/ranking");
    let streak_modelholder = await modelholder_init("/assets/common/font/numbers");
    let combo_modelholder = ranking_modelholder;
    let ui_animlist = await animlist_init("/assets/common/anims/week-round.xml");

    let healthbar = healthbar_init(
        170, 432, 999,
        300, 9, 2, 5,
        24, 16
    );
    healthbar_state_background_add2(healthbar, 0x00000, null, null);
    healthbar_state_player_add2(healthbar, boyfriend_moldelholder, 0x00FF00, null);
    healthbar_state_opponent_add2(healthbar, dad_moldelholder, 0xFF0000, null);
    healthbar_set_layout_size(healthbar, pvr_context.screen_width, pvr_context.screen_height);
    healthbar_internal_calc_dimmensions(healthbar);
    healthbar_state_toggle(healthbar, null);
    healthbar_set_bump_animation(healthbar, ui_animlist);
    healthbar_load_warnings(healthbar, warnings_modelholder, 0);
    healthbar_enable_extra_length(healthbar, 0);
    healthbar_set_bpm(healthbar, 100);
    healthbar_set_health_position(healthbar, 1.0, 0.5, 0);
    window["healthbar"] = healthbar;

    let font = await fontholder_init("/assets/common/font/vcr.ttf", 48, null);
    // @ts-ignore
    let font_tt = 1;
    let font_size = 48;

    let rankingcounter = rankingcounter_init(
        null, null, font, font_size,
        128
    );
    rankingcounter_add_state(rankingcounter, ranking_modelholder, null);
    rankingcounter_toggle_state(rankingcounter, null);
    rankingcounter_set_default_ranking_animation(rankingcounter, ui_animlist);
    rankingcounter_set_default_ranking_text_animation(rankingcounter, ui_animlist);
    rankingcounter_use_percent_instead(rankingcounter, 0);
    window["rankingcounter"] = rankingcounter;


    let streakcounter = streakcounter_init(null, 128, 256, 48);
    streakcounter_state_add(streakcounter, combo_modelholder, streak_modelholder, null);
    streakcounter_state_toggle(streakcounter, null);
    streakcounter_set_number_animation(streakcounter, ui_animlist);
    streakcounter_set_combo_animation(streakcounter, ui_animlist);


    let next_hit = 1000;
    let time = 0;
    let playerstats = playerstats_init();
    playerstats.streak = 10;


    while (window.run) {
        let elapsed = await pvr_wait_ready();
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        time += elapsed;
        if (time > next_hit) {
            next_hit = time + 800;
            let diff = math2d_random(0, 100);
            playerstats_add_hit(playerstats, 1.0, 100, diff);
        }

        rankingcounter_peek_ranking(rankingcounter, playerstats);

        // @ts-ignore
        let has_combo_break = streakcounter_peek_streak(streakcounter, playerstats);

        character_animate(character_boyfriend, elapsed);
        character_draw(character_boyfriend, pvr_context);

        rankingcounter_animate1(rankingcounter, elapsed);
        rankingcounter_animate2(rankingcounter, elapsed);
        rankingcounter_draw1(rankingcounter, pvr_context);
        rankingcounter_draw2(rankingcounter, pvr_context);

        streakcounter_animate(streakcounter, elapsed);
        streakcounter_draw(streakcounter, pvr_context);

        healthbar_animate(healthbar, elapsed);
        healthbar_draw(healthbar, pvr_context);
    }
    */
}

function main_debug_print_pools() {
    console.log("STATESPRITE_POOL", STATESPRITE_POOL);
    console.log("ANIMSPRITE_POOL", ANIMSPRITE_POOL);
    console.log("TEXTSPRITE_POOL", TEXTSPRITE_POOL);
    console.log("TEXTURE_POOL", TEXTURE_POOL);
    console.log("MODELHOLDER_POOL", MODELHOLDER_POOL);
    console.log("SPRITE_POOL", SPRITE_POOL);
    console.log("DRAWABLE_POOL", DRAWABLE_POOL);
    console.log("FONTTYPE_POOL", FONTTYPE_POOL);
}

