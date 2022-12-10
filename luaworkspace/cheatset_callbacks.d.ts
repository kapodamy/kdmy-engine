
declare namespace KDMYEngine {
    /**
     * @file
     * @summary This is a cheatset of functions to write in the lua script
     * This contains all functions called by the engine.
     * All these functions are optional, so you can implement (write in your lua script) only the functions you need
     * 
     * also.... ¡¡¡¡DO NOT IMPORT THIS FILE!!!!
     */

    /**
     * Called when the week is loaded (with the first track). If the week is initialized from the freeplay menu 
     * an index of the choosen song is passed.
     * 
     * Note: In "freeplay mode" you should avoid/skip any cutscenes, distractions or dialogues if necessary.
     * @param freeplay_index index of the song in "about.json" file or "freeplayTrackIndexInGameplayManifest" field value, otherwise, -1.
     */
    export function f_weekinit(freeplay_index: number): void;

    /**
     * Called before the "_Ready?_" message is shown.
     * @param from_retry {@link true} if the user restarted the current track, otherwise, {@link false}.
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
     * Called when the current round (track of the week) has ended.
     * @param loose {@link true} If a player was defeated, otherwise, {@link false} the track is clear.
     */
    export function f_roundend(loose: boolean): void;

    /**
     * Called when the week ends, this means the game is over.
     * @param giveup {@link true} If a player has loose and gives up, otherwise, {@link false} the week was completed.
     */
    export function f_weekend(giveup: boolean): void;

    /**
     * @summary Notifies the dead player's decision, this function is called after {@link f_roundend}.
     * Depending on the choosen decision later will be called {@link f_beforeready} or {@link f_weekend}.
     * @param retry_or_giveup {@link true} if the player wants to retry the
     * round (current track), otherwhise, {@link false} if gives up.
     */
    export function f_diedecision(retry_or_giveup: boolean): void;

    /**
     * Called before the pause menu is shown, this suspends the stage and UI layouts (animations, triggers, sounds, etc)
     * @param pause_or_resume {@link true} if game was paused, otherwhise, {@link false} if returns to the gameplay.
     */
    export function f_pause(pause_or_resume: boolean): void;

    /**
     * Called when the player leaves the game, this means one of
     * "_Return to the main menu_" or "_Return to the week selector_" options was selected from the pause menu.
     * Important: on this state, {@link f_roundend} and {@link f_weekend} functions never will be called.
     */
    export function f_weekleave(): void;

    /**
     * @summary Called after the results are shown and before the player returns to the menus
     * Results are only shown once the week is completed and clear (without changing the difficult)
     */
    export function f_afterresults(): void;

    /**
     * Called when the script is disposed before starting another round, this may mean that it is being
     * replaced by another script or by none at all.
     */
    export function f_scriptchange(): void;

    /**
     * Called when the week menu (an extra menu) is shown or hidden.
     * Note: only called if the week provided an aditional menu.
     * @param shown_or_hidden {@link true} if the custom week menu is opened, otherwise, {@link false} if closed.
     */
    export function f_pause_menuvisible(shown_or_hidden: boolean): void;

    /**
     * Called during the pause menu, once the user enters in the "_Week menu_" option.
     * Note: only called if the week provided an aditional menu.
     * @param option_index Index (base-zero) of the custom option selected.
     * @param buttons An usigned number indicating the buttons pressed on the option. These are bit flags/fields.
     */
    export function f_pause_optionselected(option_index: number, buttons: GamepadButtons): void;

    /**
     * Called on a note hit or miss, also applies to penalties.
     * @param timestamp milliseconds presentation of the note (as readed from the chart file).
     * @param id ID (as readed from the chart file), this normally is the strum line index.
     * @param duration duration in milliseconds.
     * @param data {@link double} number attached (4° index in the note definition as readed from the chart). 0 by default
     * @param special {@link true} if notes of that ID are marked as special in the "notepool" (defined in gameplay.json file). Defaults to {@link false}
     * @param player_id Index (base-zero) of the player, normally 0 means the opponent and 1 the player.
     * @param state the state of the note
     */
    export function f_note(timestamp: number, id: number, duration: number, data: number, special: boolean, player_id: number, state: NoteState): void;

    /**
     * Called after strums are scrolled and before {@link f_frame}
     * @param player_id Index (base-zero) of the player controller/gamepad.
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
     * This function is not called exactly when an event occurs beacuse the check is done on every frame (before {@link f_frame}).
     * The milliseconds spent waiting for the frame is passed in the parameter __since__.
     * @param current_beat The beat number (also known as count).
     * @param since How many milliseconds transcurred since the new beat
     */
    export function f_beat(current_beat: number, since: number): void;

    /**
     * @summary Called on every quarter (a quarter is 1/4 of a beat).
     * This function is not called exactly when an event occurs beacuse the check is done on every frame (before {@link f_frame}).
     * The milliseconds spent waiting for the frame is passed in the parameter __since__.
     * @param current_quarter The quarter number (also known as count).
     * @param since How many milliseconds transcurred since the new quarter.
     */
    export function f_quarter(current_quarter: number, since: number): void;

    /**
     * @summary Called when the dialogue is playing the close animation
     */
    export function f_dialogue_closing(): void;
    
    /**
     * @summary Called when the the dialogue is closed or "<Exit />" is used on a state
     */
    export function f_dialogue_exit(): void;
    
    /**
     * @summary Called before the dialogue prints any text
     * @param line_index the line index in the dialog text file (txt file)
     * @param state_name state of the current line (if the line starts with ":dad:" the state name is "dad")
     */
    export function f_dialogue_line_starts(line_index: number, state_name: string): void;
    
    /**
     * @summary Called after the dialogue ends printing the current dialog line
     * @param line_index the line index in the dialog text file (txt file)
     * @param state_name state of the current line (if the line starts with ":dad:" the state name is "dad")
     */
    export function f_dialogue_line_ends(line_index: number, state_name: string): void;

    /**
     * @summary Called after the strum scrolling is done, at this point all key inputs are processed by all
     * strum lines and all {@link PlayerStats} are updated.
     */
    export function f_after_strum_scroll(): void;
}

