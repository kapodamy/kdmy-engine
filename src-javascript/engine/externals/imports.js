
/**
 * @typedef {object} ModuleLuaScript
 * @property {function(number, number, number, number):ModuleLuaScript} _luascript_init
 * @property {function(ModuleLuaScript, number):void} _luascript_drop_shared
 * @property {function(ModuleLuaScript):void} _luascript_destroy_JS
 * @property {function(number, number):void} _luascript_set_engine_globals_JS
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_weekinit
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_beforeready
 * @property {function(ModuleLuaScript):void} _luascript_notify_ready
 * @property {function(ModuleLuaScript):void} _luascript_notify_aftercountdown
 * @property {function(ModuleLuaScript,number):void} _luascript_notify_frame
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_roundend
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_weekend
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_diedecision
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_pause
 * @property {function(ModuleLuaScript):void} _luascript_notify_weekleave
 * @property {function(ModuleLuaScript):void} _luascript_notify_afterresults
 * @property {function(ModuleLuaScript):void} _luascript_notify_scriptchange
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_pause_optionselected
 * @property {function(ModuleLuaScript,bool):void} _luascript_notify_pause_menuvisible
 * @property {function(ModuleLuaScript,number,number,number,number,bool,number,number):void} _luascript_notify_note
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_buttons
 * @property {function(ModuleLuaScript,number,number,number,number,number):void} _luascript_notify_unknownnote
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_beat
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_quarter
 * @property {function(ModuleLuaScript,number):void} _luascript_notify_timer_run
 * @property {function(ModuleLuaScript,number):void} _luascript_notify_timersong_run
 * @property {function(ModuleLuaScript,number,string):void} _luascript_notify_dialogue_builtin_open
 * @property {function(ModuleLuaScript,number,number, number):void} _luascript_notify_dialogue_line_starts
 * @property {function(ModuleLuaScript,number,number, number):void} _luascript_notify_dialogue_line_ends
 * @property {function(ModuleLuaScript):void} _luascript_notify_after_strum_scroll
 * @property {function(ModuleLuaScript,number,number,number):void} _luascript_notify_modding_menu_option_selected;
 * @property {function(ModuleLuaScript,number,number,number):boolean} _luascript_notify_modding_menu_option_choosen;
 * @property {function(ModuleLuaScript):boolean} _luascript_notify_modding_back;
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_modding_exit;
 * @property {function(ModuleLuaScript,number,number):void} _luascript_notify_modding_init;
 * @property {function(ModuleLuaScript,number):void} _luascript_call_function
 * @property {function(ModuleLuaScript,number):number} _luascript_eval
 * @property {function(string):number} kdmyEngine_stringToPtr
 * @property {function(number):string} kdmyEngine_ptrToString
 * @property {function(number):void} kdmyEngine_deallocate
 * @property {function(object):number} kdmyEngine_obtain
 * @property {function():Promise<any>|undefined} kdmyEngine_yieldAsync
 * @property {function():boolean} kdmyEngine_hasAsyncPending
 * @property {function(object):void} kdmyEngine_drop_shared_object
 * @property {function():DataView} kdmyEngine_get_ram
 * 
 * @property {bool} isRuntimeInitialized
 * @property {bool} hasGlobalsSet
 * @property {boolean} kdmyEngine_endianess
 * 
 */
 

/** @type {ModuleLuaScript} */
const ModuleLuaScript = {
    // @ts-ignore
    print: (text) => {
        console.log("luascript-stdout", text);
    },

    // @ts-ignore
    printErr: (text) => {
        console.error("luascript-stderr", text);
    },

    // @ts-ignore
    onRuntimeInitialized: () => {
        console.info("luascript is ready to use")
        ModuleLuaScript.isRuntimeInitialized = true;
    },

    isRuntimeInitialized: false,
    hasGlobalsSet: false
};


/**
* @typedef {object} ModuleFontAtlas
* @property {function(number, number):ModuleFontAtlas} _fontatlas_init
* @property {function(ModuleFontAtlas):void} _fontatlas_destroy_JS
* @property {function(ModuleFontAtlas, number, number, number):number} _fontatlas_atlas_build
* @property {function(number):void} _fontatlas_atlas_destroy_texture_only
* @property {function(number):void} _fontatlas_atlas_destroy_JS
* @property {function(ModuleFontAtlas, number, number):number} _fontatlas_atlas_build_complete
* @property {function(number):void} _fontatlas_enable_sdf
* @property {function(Uint8Array|Int32Array|ArrayBuffer):number} kdmyEngine_allocate
* @property {function(number):void} kdmyEngine_deallocate
* @property {function(number):FontCharMap} kdmyEngine_parseFontCharMap
* 
* @property {bool} hasInstanceActive
* @property {bool} isRuntimeInitialized
* 
*/

/**
 * @typedef {object} FontCharMap
 * @property {FontCharData[]} char_array
 * @property {number} char_array_size
 * @property {Uint8Array} texture 8bpp grayscale texture
 * @property {number} texture_width
 * @property {number} texture_height
 * @property {number} texture_byte_size
 */

/**
 * @typedef {object} FontCharData
 * @property {number} codepoint
 * @property {number} offset_y
 * @property {number} offset_x
 * @property {number} advancex
 * @property {number} width
 * @property {number} height
 * @property {FontCharDataKerning[]} kernings
 * @property {number} kernings_size
 * @property {FontCharDataAtlasEntry} atlas_entry
 * @property {bool} has_entry
 */

/**
 * @typedef {object} FontCharDataAtlasEntry
 * @property {number} x
 * @property {number} y
*/

/**
 * @typedef {object} FontCharDataKerning
 * @property {number} codepoint
 * @property {number} x
 */

/** @type {ModuleFontAtlas} */
const ModuleFontAtlas = {
    // @ts-ignore
    print: (text) => {
        console.log("fontatlas-stdout", text);
    },


    printErr: (text) => {
        console.error("fontatlas-stderr", text);
    },


    onRuntimeInitialized: () => {
        console.info("fontatlas is ready to use")
        ModuleFontAtlas.isRuntimeInitialized = true;
    },

    isRuntimeInitialized: false,
    hasInstanceActive: false
};
