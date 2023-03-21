
declare namespace KDMYEngine {
    /**
     * @file
     * @summary This is a cheatset of functions to write in your lua script
     * This contains all functions called by the engine.
     * All these functions are optional, so you can implement (write in your own lua script) only the functions you need
     *
     * also.... ¡¡¡¡DO NOT IMPORT THIS FILE!!!!
     */

    /**
     * Called when the week is loaded. If the week is initialized from the freeplay menu.
     * an index of the choosen song is passed. Note: Attempting to adquire any object from the engine can fail
     * because this function is called before loading week's resources. 
     *
     * Suggestion: In "freeplay mode" you should avoid/skip any cutscenes, distractions or dialogues if necessary.
     * @param freeplay_index index of the song in "about.json" file or "freeplaySongIndexInGameplayManifest" field 
     * value, otherwise, -1.
     */
    export function f_weekinit(freeplay_index: number): void;

    /**
     * @summary Called before the "__Ready?__" message is shown.
     * In this function you should do any init tasks. If the player looses and changes the difficult (during the
     * gameover screen), _from_retry_ will be false.
     * @param from_retry {@link true} if the user restarted or is attempting again, otherwise, {@link false} for
     * first attempt or difficult changed.
     */
    export function f_beforeready(from_retry: boolean): void;

    /**
     * Called when the user is ready and before the countdown begins
     */
    export function f_ready(): void;

    /**
     * Called when the gameplay itself is about to start (before scroll strums and play the song)
     */
    export function f_aftercountdown(): void;

    /**
     * @summary Called before animate and rendering.
     * Strums already are scrolled at this point and UI elements are updated with the player(s) progress.
     * If a player dies (not opponent) this function is called after {@link f_roundend} or a player pauses the
     * game is called after {@link f_pause}.
     * @param elapsed Amount of milliseconds since last frame (also known as delta-time).
     */
    export function f_frame(elapsed: number): void;

    /**
     * Called when the current round (song of the week) has ended.  
     * Note: when the player loses the gameover screen is shown.
     * @param loose {@link true} If a player was defeated, otherwise, {@link false} if the song was cleared.
     */
    export function f_roundend(loose: boolean): void;

    /**
     * Called when the week ends, this means the game is over.
     * @param giveup {@link true} If a player has loose and gives up, otherwise, {@link false} the week was completed.
     */
    export function f_weekend(giveup: boolean): void;

    /**
     * @summary Notifies the die player's animation is completed and before:
     * - looped dead animation starts
     * - play the background music if present
     * - the gameover UI becomes visible  
     * the function is called after {@link f_roundend}
     */
    export function f_gameoverloop(): void;

    /**
     * @summary Notifies the dead player's decision, this function is called before playing the dead player's 
     * "retry" or "giveup" animations.
     * @param retry_or_giveup {@link true} if the player wants to retry the
     *                                     round (current song), otherwhise, {@link false} if gives up.
     * @param changed_difficult {@link null} if the player does not change the difficult, otherwise, the 
     *                                       difficult name is provided.
     * Note: if the difficult was changed the {@link f_beforeready} function is called with {@link false} argument
     */
    export function f_gameoverdecision(retry_or_giveup: boolean, changed_difficult: string): void;

    /**
     * @summary Called when the gameover screen ends
     */
    export function f_gameoverended(): void;

    /**
     * Called before the pause menu is shown, this suspends the stage layout (includes animations, triggers, sounds, etc)
     * @param pause_or_resume {@link true} if game was paused, otherwhise, {@link false} if returns to the gameplay.
     */
    export function f_pause(pause_or_resume: boolean): void;

    /**
     * Called when the player leaves the game, this means one of
     * "_Return to the main menu_" or "_Return to the week selector_" options was selected from the pause menu.
     * Important: on this state, {@link f_roundend} and {@link f_weekend} functions never are called.
     */
    export function f_weekleave(): void;

    /**
     * @summary Called after the results are shown and before the player returns to the menus
     * Results are only shown once the week is completed, if the player has changed the difficult
     * in one or more songs (excluding first week's song) the player is notified with the legend
     * "__WAIT, YOU CHANGED THE DIFFICULT!__" and without saving any progress.
     * This function is called whatever {@link week_disable_week_end_results} was used.
     * @param {number} total_attempts total attempts taken to clear all songs
     * @param {number} songs_count the amount of songs cleared, its always 1 in freeplay mode (see {@link f_weekinit})
     * @param {number} reject_completed if the difficult was changed, otherwise, false
     */
    export function f_afterresults(total_attempts: number, songs_count: number, reject_completed: boolean): void;

    /**
     * @summary Called when the script is disposed and replaced by another script (or none at all).
     */
    export function f_scriptchange(): void;

    /**
     * Called when the week menu (an extra menu) is shown or hidden.
     * Note: only called if the week provided an aditional menu in the "gameplay.json" file.
     * @param shown_or_hidden {@link true} if the custom week menu is opened, otherwise, {@link false} if closed.
     */
    export function f_pause_menuvisible(shown_or_hidden: boolean): void;

    /**
     * Called during the pause menu, once the user enters in the "_Week menu_" option.
     * Note: only called if the week provided an aditional menu.
     * @param option_index Index (base-zero) of the custom option selected.
     * @param buttons An usigned number indicating the buttons pressed on the option. These are bit flags/fields.
     */
    export function f_pause_optionchoosen(option_index: number, buttons: GamepadButtons): void;

    /**
     * Called on a note hit or miss, also applies to penalties.
     * @param timestamp milliseconds presentation of the note (as readed from the chart file).
     * @param id ID (as readed from the chart file), this normally is the strum line index.
     * @param duration duration in milliseconds.
     * @param data {@link double} number attached (4° index in the note definition as readed from the chart). 0 by default
     * @param special {@link true} if notes of that ID are marked as special in the "notepool" (defined in gameplay.json file). Defaults to {@link false}
     * @param player_id Index (base-zero) of the character, normally 0 means the opponent and 1 the player.
     * @param state the state of the note
     */
    export function f_note(timestamp: number, id: number, duration: number, data: number, special: boolean, player_id: number, state: StrumNoteState): void;

    /**
     * Called after strums are scrolled and before {@link f_frame}. In a modding context is also called
     * @param player_id Index (base-zero) of the player controller/gamepad, in modding context this value is -1.
     * @param buttons An usigned number indicating the buttons pressed in the controller. These are bit flags/fields defined in GAMEPAD_* constants
     */
    export function f_buttons(player_id: number, buttons: GamepadButtons): void;

    /**
     * Called wheen a unknown note is found. Note: unknown notes are not processed by strum lines.
     * @param player_id Index (base-zero) of the player, normally 0 means the opponent and 1 the player.
     * @param timestamp milliseconds presentation of the note (as readed from the chart file).
     * @param id ID (as readed from the chart file), this normally is the strum line index.
     * @param duration duration in milliseconds.
     * @param data {@link double} number attached (4° index in the note definition as readed from the chart). 0 by default
     */
    export function f_unknownnote(player_id: number, timestamp: number, id: number, duration: number, data: number): void;

    /**
     * @summary Called on every beat.
     * This function is not called exactly when the beat occurs beacuse the check is done
     * on every frame (before {@link f_frame}).
     * The milliseconds spent waiting for the frame is passed in the parameter __since__.
     * @param current_beat The beat number (also known as count).
     * @param since How many milliseconds transcurred since the new beat
     */
    export function f_beat(current_beat: number, since: number): void;

    /**
     * @summary Called on every quarter (a quarter is 1/4 of a beat).
     * This function is not called exactly when the quarter occurs beacuse the check is done
     * on every frame (before {@link f_frame}).
     * The milliseconds spent waiting for the frame is passed in the parameter __since__.
     * @param current_quarter The quarter number (also known as count).
     * @param since How many milliseconds transcurred since the new quarter.
     */
    export function f_quarter(current_quarter: number, since: number): void;

    /**
     * @summary Called when the engine load and display a dialogue declared
     * in the gameplay manifest (gameplay.json file).
     * This happens if the song contains the "dialogText" property set.
     * Note 1: the engine calls this function before {@link f_beforeready}.
     * Note 2: while the dialog is shown the user can pause the game.
     * Note 3: if the user restarts the song, the dialog is ignored.
     * Note 4: if the initial states contains lua code, these runs first.
     * @param dialog_src the dialog text filename as-is was defined in the gameplay manifest.
     */
    export function f_dialogue_builtin_open(dialog_src: string): void;

    /**
     * @summary Called when the dialogue is playing the close animation
     */
    export function f_dialogue_closing(): void;

    /**
     * @summary Called when the the dialogue is closed or "\<Exit /\>" is used on a state
     */
    export function f_dialogue_exit(): void;

    /**
     * @summary Called before the dialogue prints any text
     * @param line_index the line index in the dialog text file (txt file)
     * @param state_name state of the current line (if the line starts with ":bf:" the state name is "bf")
     * @param text text content of the line (if the line is ":bf:beep!" the text is "beep!")
     */
    export function f_dialogue_line_starts(line_index: number, state_name: string, text: string): void;

    /**
     * @summary Called after the dialogue ends printing the current dialog line
     * @param line_index the line index in the dialog text file (txt file)
     * @param state_name state of the current line (if the line starts with ":bf:" the state name is "bf")
     * @param text text content of the line (if the line is ":bf:beep!" the text is "beep!")
     */
    export function f_dialogue_line_ends(line_index: number, state_name: string, text: string): void;

    /**
     * @summary Called after the strum scrolling is done and all key inputs are processedm, also any {@link PlayerStats} is updated.
     */
    export function f_after_strum_scroll(): void;


    /**
     * @summary (modding context only) Called when a menu option is selected
     * @param menu the target menu
     * @param index (base zero) selected option index
     * @param name name of the option or text label
     */
    export function f_modding_menu_option_selected(menu: Menu, index: number, name: string): void;

    /**
     * @summary (modding context only) Called when a menu option is choosen, this leaves the current
     * screen and loads another (example from main screen to freeplay screen).
     * @param menu the target menu
     * @param index (base zero) selected option index
     * @param name name of the option or text label
     * @return {boolean} true to cancel, otherwise, false.
     */
    export function f_modding_menu_option_choosen(menu: Menu, index: number, name: string): boolean;

    /**
     * @summary (modding context only) Called when something happens in the UI, events can be different
     * in all engine screen menus.
     * @param name name of the event
     */
    export function f_modding_event(name: string): void;

    /**
     * @summary (modding context only) Called when a menu option is choosen but is not recognized by
     * the engine. This function is only called if native menus contains custom option/entries.
     * @param option_name name of the option, which is the same as declared in {@link MenuManifestItem.name}
     */
    export function f_modding_handle_custom_option(option_name: string): void;

    /**
     * @summary (modding context only) Called when the user is attempting go to the previous screen.
     * @return {boolean} true to cancel, otherwise, false.
     */
    export function f_modding_back(): boolean;

    /**
     * @summary (modding context only) Called before the script is disposed. From here is possible return
     * a value to the previous screen menu script (only if waiting on {@link modding_spawn_screen}).
     * @return {BasicValue} the value to return.
     */
    export function f_modding_exit(): BasicValue;

    /**
     * @summary (modding context only) Called when the script is loaded from a another modding context or
     * screen menu. The parameter _arg_ value is the same used in the call to {@link modding_spawn_screen}.
     * @param arg value passed by {@link modding_spawn_screen} or "NATIVE_SCREEN" if called by the engine.
     */
    export function f_modding_init(arg: BasicValue): void;

    /**
     * @summary (modding context only) Called when the game window looses or gain focus.
     * @param focused true if focused, otherwise, false.
     */
    export function f_modding_window_focus(focused: boolean): void;

    /**
     * @summary (modding context only) Called when the game window is minimized in the OS taskbar.
     * @param minimized true if minimized, otherwise, false.
     */
    export function f_modding_window_minimized(minimized: boolean): void;

    /**
     * @summary called when the game window was resized. Note: the size always honours the aspect ratio
     * @param screen_width the new window width.
     * @param screen_height the new window height.
     */
    export function f_window_size_changed(screen_width: number, screen_height: number): void;

    /**
     * @summary (desktop version only) called when a key is pressed or released
     * @param key The keyboard key that was pressed or released. Can be {@link Keys.Unknown} if unknown
     * @param scancode The system-specific scancode of the key. 
     * @param is_pressed true on press, false if was released.
     * @param mods Bit field describing which modifier keys were held down.
     */
    export function f_input_keyboard(key: Keys, scancode: number, is_pressed: boolean, mods: ModKeys): void;

    /**
     * @summary (desktop version only) called when the mouse is moved, the reported position is relative
     * to the window viewport, the position can be negative or greater than viewport width/height if the cursor
     * moves outside of the window. 
     * @param x The new cursor x-coordinate, relative to the left edge of the window
     * @param y The new cursor y-coordinate, relative to the top edge of the window
     */
    export function f_input_mouse_position(x: number, y: number): void;

    /**
     * @summary (desktop version only) called when the mouse cursor enters or leave the window area
     * @param entering true if the cursor entered the window, otherwise, false if leaving.
     */
    export function f_input_mouse_enter(entering: boolean): void;

    /**
     * @summary (desktop version only) Called when a mouse button is pressed or released
     * @param button which button was pressed
     * @param is_pressed true on press, otherwise, false if was released.
     * @param mods 	Bit field describing which keyboard modifier keys were held down.
     */
    export function f_input_mouse_button(button: MouseButton, is_pressed: boolean, mods: ModKeys): void;

    /**
     * @summary (desktop version only) called when the vertical and/or horizontal wheel is moved. The scroll amounts
     * are system-specific
     * @param x The scroll offset along the x-axis.
     * @param y The scroll offset along the y-axis.
     */
    export function f_input_mouse_scroll(x: number, y: number): void;


}

