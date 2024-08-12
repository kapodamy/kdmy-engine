"use strict";

//
// JS STUB for KallistiOS
//

const STATE_ZOMBIE = 0x0000;
const STATE_RUNNING = 0x0001;
const STATE_READY = 0x0002;
const STATE_WAIT = 0x0003;
const STATE_FINISHED = 0x0004;

/** 
 * @summary  Create a new thread.
    This function creates a new kernel thread with default parameters to run the
    given routine. The thread will terminate and clean up resources when the
    routine completes if the thread is created detached, otherwise you must
    join the thread with thd_join() to clean up after it.
    @param {number|boolean} detach    Set to 1 to create a detached thread. Set to 0 to create a joinable thread.
    @param {function(any):any} routine     The function to call in the new thread.
    @param {any} param      A parameter to pass to the function called.
    @returns {object}       The new thread on success, NULL on failure.
*/
function thd_create(detach, routine, param) {

    const kthread_context = {
        // these properties are for internal use only
        kthread_id: kos_thd_id++,
        detached: !!detach,
        completed: false,
        error: null,
        result: null,

        // public properties (present in "kthread_t" struct)
        state: STATE_READY,
        rv: null
    };

    function cleanup_tls(kthread_id) {
        for (let [key, data] of kos_tls_keys) {
            if (data.destructor) {
                let value = data.threads.get(kthread_id);
                if (value != null) {
                    data.destructor(value);
                }
            }
            data.threads.delete(kthread_id);
        }
    }

    function resolve_joins(kthread_id, result, error) {
        for (let halt of kos_thd_instances.get(kthread_id)) {
            if (error)
                halt.reject(error);
            else
                halt.resolve(result);

            delete halt.reject;
            delete halt.resolve;
        }
    }

    function handle_thread_exit(kthread, result, error, resolve, reject) {
        cleanup_tls(kthread.kthread_id);
        resolve_joins(kthread.kthread_id, result, error);
        kos_thd_instances.delete(kthread.kthread_id);

        kthread.completed = 1;
        kthread.error = error;
        kthread.result = result === undefined ? null : result;
        kthread.state = STATE_FINISHED;

        if (!error) kthread.rv = result === undefined ? null : result;

        if (error)
            reject(error);
        else
            resolve(result);
    }

    kos_thd_instances.set(kthread_context.kthread_id, new Array());

    function runner(resolve, reject) {
        let ret = undefined;

        try {
            kthread_context.state = STATE_RUNNING;
            ret = routine.call(kthread_context, param);
            kthread_context.state = STATE_ZOMBIE;
            if (!(ret instanceof Promise)) {
                handle_thread_exit(kthread_context, ret, null, resolve, reject);
                return;
            }
        } catch (error) {
            kthread_context.state = STATE_ZOMBIE;
            handle_thread_exit(kthread_context, null, error, resolve, reject);
            return;
        }

        // wait until the promise is fulfilled
        kthread_context.state = STATE_WAIT;
        ret.then(
            function (result) {
                handle_thread_exit(kthread_context, result, null, resolve, reject);
            },
            function (error) {
                handle_thread_exit(kthread_context, null, error, resolve, reject);
            }
        );
    }

    // Javascript in a nutshell: Promise --> requestIdleCallback (for async running) --> runner
    new Promise(function (resolve, reject) {
        requestIdleCallback(
            function (time) { runner(resolve, reject); },
            { timeout: 1 }
        );
    });

    return kthread_context;
}

/**
 * This function manually yields the current thread's timeslice to the system,
 * forcing a reschedule to occur.
 * @summary Throw away the current thread's timeslice.
 */
function thd_pass() {
    return new Promise(function (resolve, reject) {
        setTimeout(resolve, 1);
    });
}

/**
 * This function puts the current thread to sleep for the specified amount of
 * time. The thread will be removed from the runnable queue until the given
 * number of milliseconds passes. That is to say that the thread will sleep for
 * at least the given number of milliseconds. If another thread is running, it
 * will likely sleep longer.
 * @param {number}  ms The number of milliseconds to sleep.
 * @summary Sleep for a given number of milliseconds.
*/
function thd_sleep(ms) {
    let start = timer_ms_gettime64();
    let end = start + ms;
    return new Promise(function (resolve, reject) {
        function callback(time) {
            if (time >= end)
                resolve();
            else
                requestAnimationFrame(callback);
        }
        callback(timer_ms_gettime64());
    });
}


/** 
 * @summary brief  Wait for a thread to exit.
    This function "joins" a joinable thread. This means effectively that the
    calling thread blocks until the speified thread completes execution. It is
    invalid to join a detached thread, only joinable threads may be joined.
    @param {object} thd             The joinable thread to join.
    @param {any} value_ptr       A pointer to storage for the thread's return value,
                            or NULL if you don't care about it.
    @returns  {Promise<number>}               0 on success, or less than 0 if the thread is
                            non-existant or not joinable.
*/
async function thd_join(thd, value_ptr) {
    let promise = new Promise(function (resolve, reject) {
        if (thd.detached) {
            reject(new Error("Can not join an detached thread"));
        } else if (thd.completed) {
            if (thd.error) {
                reject(thd.error);
                return;
            }
            if (value_ptr) value_ptr[0] = thd.result;
            resolve(0);
        } else {
            let halt_array = kos_thd_instances.get(thd.kthread_id);
            if (!halt_array) reject(new Error("Invalid thread provided"));
            halt_array.push({ resolve, reject });
        }
    });

    return await promise;
}

/** 
 * @summary  Brutally kill the given thread.
    This function kills the given thread, removing it from the execution chain,
    cleaning up thread-local data and other internal structures. You should
    never call this function on the current thread. In general, you shouldn't
    call this function at all.
    @param {object} thd             The thread to destroy.
    @returns {number} 0               On success.
*/
function thd_destroy(thd) {
    return 0;
}

/**
 * This function retrieves the number of milliseconds since KOS was started. 
 * It is equivalent to calling timer_ms_gettime() and combining the number of 
 * seconds and milliseconds into one 64-bit value.
 * @summary Get the current uptime of the system (in milliseconds).
 * @returns {number} The number of milliseconds since KOS started.
 */
function timer_ms_gettime64() {
    return window.performance.now();
}

//
// JS STUB for C stdlib.h
//


/**
 * @callback compar
 * @param {any} elem1
 * @param {any} elem2
 * @returns {number} 
 */
/**
 * The C library function sorts an array.
 * @param {any[]} base This is the pointer to the first element of the array to be sorted.
 * @param {number} num This is the number of elements in the array pointed by base.
 * @param {number} size  This is the size in bytes of each element in the array.
 * @param {compar} compar This is the function that compares two elements.
 */
function qsort(base, num, size, compar) {
    // @ts-ignore
    base.sort(compar);
}

/**
 *  Gives the amount of storage, in bytes, required to store an object of the type 
 * @param {string|object} type the desired type
 * @returns {number}
 */
function sizeof(type) {
    if (type === undefined) return 1;
    if (type instanceof Array) return type.length;
    if (typeof (type) == "string") return type.length + 1;

    if (Object.getPrototypeOf(type) != Object.prototype)
        throw new TypeError("javacript object provided");

    let props = 0;
    for (const prop in type) props++;

    return props;
}

function malloc(type) {
    return {};
}

/**
 * The C library function deallocates the memory previously allocated by a call to calloc, malloc, or realloc.
 * @param {*} ptr This is the pointer to a memory block previously allocated with malloc, calloc or realloc to be deallocated. If a null pointer is passed as argument, no action occurs.
 */
function free(ptr) {

}

/**
 * Attempts to resize the memory block pointed to by ptr that was previously allocated with a call to malloc or calloc.
 * @param {any[]|Uint32Array} ptr This is the pointer to a memory block previously allocated with malloc, calloc or realloc to be reallocated. If this is NULL, a new block is allocated and a pointer to it is returned by the function.
 * @param {number} size This is the new size for the memory block, in bytes. If it is 0 and ptr points to an existing block of memory, the memory block pointed by ptr is deallocated and a NULL pointer is returned.
 * @returns {any} This function returns a pointer to the newly allocated memory, or NULL if the request fails.
 */
function realloc(ptr, size) {
    if (ptr instanceof Array) {
        if (size < ptr.length) {
            ptr.splice(size);
            return ptr;
        }

        let new_array = new Array(size);
        for (let i = 0; i < ptr.length && i < size; i++)
            new_array[i] = ptr[i];
        return new_array;
    }
    if (ptr instanceof Uint32Array) {
        let new_array = new Uint32Array(size);
        new_array.set(ptr, 0);
        return new_array;
    }
    return ptr;
}

function memcmp(ptr1, ptr2, num) {
    if (ptr1 == ptr2) return 0;

    if (ptr1 instanceof Array && ptr2 instanceof Array) {
        for (let i = 0; i < num; i++) {
            if (ptr1[i] != ptr2[i]) return -1;
        }
        return 0;
    }

    return 1;
}

/**
 * Returns a pointer to a null-terminated byte string, which is a duplicate of the string pointed to by str1. The returned pointer must be passed to free to avoid a memory leak.
 * If an error occurs, a null pointer
 * @param {string} strl Pointer to the null-terminated byte string to duplicate
 * @returns {string} A pointer to the newly allocated string, or a null pointer if an error occurred.
 */
function strdup(strl) {
    if (strl == null) return null;
    return strl + "";
}


//
// TLS stuff stub
//

const kos_tls_keys = new Map();
const kos_thd_instances = new Map();
var kos_thd_id = 0;
var kos_tls_key_id = 0;


/**
 * Create a new thread-local storage key.
 * @param {object} key The key to use.
 * @param {Function} destructor A destructor for use with this key. If it is non-NULL, 
 *                        and a value associated with the key is non-NULL at thread exit, 
 *                        then the destructor will be called with the value as its argument.
 * @returns {-1|0} On success. -1 On failure, and sets errno to one of the following: EPERM if called inside an 
 *                  interrupt and another call is in progress, ENOMEM if out of memory.
 */
function kthread_key_create(key, destructor) {
    for (let [k, d] of kos_tls_keys) {
        if (k == key) {
            console.warn("kthread_key_create() duplicated key creation: " + key.key_id);
            return -1;
        }
    }
    key.id = kos_tls_key_id++;
    kos_tls_keys.set(key, { threads: new Map(), destructor });
    return 0;
}

/**
 * Retrieve a value associated with a TLS key.
 * @param {object} key The key to look up data for.
 * @returns {object} The data associated with the key, or NULL if the key is not valid or
 *           no data has been set in the current thread.
*/
function kthread_getspecific(key) {
    let data = kos_tls_keys.get(key);
    if (!data) {
        return null;
    }

    let id = 0;// current thread id, imposible to implement in JS

    let value = data.threads.get(id);
    return value === undefined ? null : value;
}

/**
 * Set thread specific data for a key.
 * 
 * @param {object} key     The key to set data for.
 * @param {object} value   The thread-specific value to use.
 * @returns {-1|0}      0 On success. -1 On failure, and sets errno to one of the following: EINVAL
 *                if the key is not valid, ENOMEM if out of memory, or EPERM
 *                if called inside an interrupt and another call is in
 *                progress.
*/
function kthread_setspecific(key, value) {
    let data = kos_tls_keys.get(key);
    if (!data) {
        console.warn("kthread_setspecific() key not found: " + key.key_id);
        return -1;
    }

    let id = 0;// current thread id, imposible to implement in JS

    data.threads.set(id, value);
    return 0;
}

/**
 * Delete a TLS key.
 *
 * @param {object} key     The key to delete.
 * @returns {-1|0}     0 On success. -1 On failure, and sets errno to one of the following: EINVAL
 *                    if the key is invalid, EPERM if unsafe to call free.
*/
function kthread_key_delete(key) {
    if (kos_tls_keys.delete(key)) return 0;
    console.warn("kthread_key_delete() key not found: " + key.key_id);
}



//
// Maple related stuff
//

/** 
 * @summary  Maple device info structure.
    This structure is used by the hardware to deliver the response to the device
    info request.
*/
class maple_devinfo_t {
    /** @type {number} Function codes supported */
    functions;
}

/**
 * One maple device.
 *   Note that we duplicate the port/unit info which is normally somewhat
 *   implicit so that we can pass around a pointer to a particular device struct.
 */
class maple_device_t {

    /** @type {boolean} Is this a valid device?*/
    valid;

    /** @type {number} Maple bus port connected to*/
    port;

    /** @type {number} Unit number, off of the port*/
    unit;

    /** @type {maple_devinfo_t} Device info struct*/
    info;


    constructor(functions, port, unit) {
        if (KOS_MAPLE_DEVS.length >= KOS_MAX_DEVICES) throw new Error("Maximum of number of devices reached");

        this.valid = functions == MAPLE_FUNC_MEMCARD;
        this.port = port;
        this.unit = unit;
        this.info = new maple_devinfo_t();
        this.info.functions = functions;
        this._gamepad_index = -1;
    }


    /** @type {Gamepad} */
    _gamepad = null;
    _gamepad_index = -1;
    _status = new cont_state_t();
    timestamp = 0;

    _peek_gamepad_data() {
        //
        // Notes:
        //    * The dreamcast controller does not have enough buttons/axes like modern controllers.
        //    * If the controller has additional buttons and/or axes, they are reported
        //      by KallistiOS using CONT_CAPABILITY_* in maple_enum_type*() functions.
        //
        const gamepad = navigator.getGamepads()[this._gamepad_index];

        if (!gamepad || !gamepad.connected) return;

        let new_timestamp = gamepad.timestamp;
        if (new_timestamp == this.timestamp) return;

        this.timestamp = new_timestamp;

        // read the gamepad buttons using the standard layout
        let length = gamepad.buttons.length;
        for (let i = 0; i < KOS_JSGAMEPAD_BUTTONS_MAPPING.length && i < length; i++) {
            const button_flag = KOS_JSGAMEPAD_BUTTONS_MAPPING[i];
            const button = gamepad.buttons[i];

            switch (button_flag) {
                case CONT_EX_TRIGGER_L:
                    this._status.ltrig = button.value * 255;
                    continue;
                case CONT_EX_TRIGGER_R:
                    this._status.rtrig = button.value * 255;
                    continue;
            }

            if (button.pressed)
                this._status.buttons |= button_flag;
            else
                this._status.buttons &= ~button_flag;
        }

        // read the gamepad axes using the standard layout
        length = gamepad.axes.length;
        for (let i = 0; i < KOS_JSGAMEPAD_AXES_MAPPING.length && i < length; i++) {
            const axis_flag = KOS_JSGAMEPAD_AXES_MAPPING[i];
            const axis = gamepad.axes[i];
            let value = axis * 0x7F;

            switch (axis_flag) {
                case CONT_EX_AXIS_LX:
                    this._status.joyx = value;
                    break;
                case CONT_EX_AXIS_LY:
                    this._status.joyy = value;
                    break;
                case CONT_EX_AXIS_RX:
                    this._status.joy2x = value;
                    break;
                case CONT_EX_AXIS_RY:
                    this._status.joy2y = value;
                    break;
                case CONT_EX_TRIGGER_L:
                    this._status.ltrig = axis * 255;
                    break;
                case CONT_EX_TRIGGER_R:
                    this._status.rtrig = axis * 255;
                    break;
            }
        }
    }

}

class maple_keyboard_device_t {
    /** 
     * @typedef {object} maple_device_keyboard_t
     * @property {number} timestamp
     * @property {number} target_button
     * @property {number} target_axis
     * @property {boolean} hold
     * @property {boolean} negative
     */
    /**
     * @callback KeyboardEventDelegate
     * @param {KeyboardEvent} evt
     * @returns {void}
     */

    status = new cont_state_t();
    timestamp = 0;
    /** @type {maple_device_keyboard_t[]} */ #queue = null;

    /**@type {KeyboardEventDelegate} */ delegate_callback = null;


    constructor() {
        document.addEventListener("keydown", evt => this._keyboard_enqueue_data(evt), false);
        document.addEventListener("keyup", evt => this._keyboard_enqueue_data(evt), false);
        this.#queue = new Array();
    }


    _keyboard_enqueue_data(/**@type {KeyboardEvent} */ evt) {
        if (evt.isComposing) return;
        if (evt.repeat) return;

        if (this.delegate_callback) this.delegate_callback(evt);

        //
        // just like "window.performance.now()", "evt.timeStamp" returns the milliseconds since
        // the page was loaded. But here since the event was created. This should give
        // better precission
        //
        let timestamp = evt.timeStamp;
        let hold = evt.type == "keydown";

        // special keys      
        switch (evt.code) {
            case "F11":
                if (hold) pvr_fullscreen(null);
                return;
            case "F12":
                //if (hold) PVRContext.TakeScreenshot();
                return;
            case "NumpadSubtract":
                if (hold) mastervolume_volume_step(false);
                return;
            case "NumpadAdd":
                if (hold) mastervolume_volume_step(true);
                return;
        }

        for (let i = 0; i < KOS_KEYBOARD_MAPPING_BUTTONS.length; i++) {
            const map = KOS_KEYBOARD_MAPPING_BUTTONS[i];
            if (map.code == evt.code) {
                this.#queue_key_event(timestamp, map.button, CONT_EX_NONE, false, hold);
            }
        }

        for (let i = 0; i < KOS_KEYBOARD_MAPPING_AXES.length; i++) {
            const map = KOS_KEYBOARD_MAPPING_AXES[i];
            if (map.key_low == evt.code) {
                this.#queue_key_event(timestamp, 0x00, map.axes, true, hold);
            }
            if (map.key_high == evt.code) {
                this.#queue_key_event(timestamp, 0x00, map.axes, false, hold);
            }
        }

        for (let i = 0; i < KOS_KEYBOARD_MAPPING_TRIGGERS.length; i++) {
            const map = KOS_KEYBOARD_MAPPING_TRIGGERS[i];
            if (map.code == evt.code) {
                this.#queue_key_event(timestamp, 0x00, map.trigger, false, hold);
            }
        }

        // reached if the key is not mapped
        return;
    }

    dequeque_all() {
        if (this.#queue.length > 0) {
            for (let i = 0; i < this.#queue.length; i++) {
                this.parse_key_event(this.#queue[i]);
            }
            this.#queue = new Array();
        }
        return this.status;
    }

    #queue_key_event(timestamp, target_button, target_axis, negative, hold) {
        if (this.#queue.length > 16) {
            // parse and update the current state
            this.parse_key_event(this.#queue.shift());
        }

        // before continue check is the key state is the same but with different modifier key
        for (let i = this.#queue.length - 1; i >= 0; i--) {
            let addedKeyInfo = this.#queue[i];
            if (addedKeyInfo.target_button != target_button) continue;
            if (addedKeyInfo.hold == hold) return;

            // the hold state is different, so... add it
            break;
        }

        this.#queue.push({
            timestamp: timestamp,
            target_button: target_button,
            target_axis: target_axis,
            hold: hold,
            negative: negative
        });
    }

    parse_key_event(/**@type {maple_device_keyboard_t} */ item) {
        this.timestamp = item.timestamp;

        if (item.target_button != 0) {
            if (item.hold)
                this.status.buttons |= item.target_button;
            else
                this.status.buttons &= ~item.target_button;
            return;
        }

        switch (item.target_axis) {
            case CONT_EX_TRIGGER_L:
                this.status.ltrig = item.hold ? 255 : 0;
                return;
            case CONT_EX_TRIGGER_R:
                this.status.rtrig = item.hold ? 255 : 0;
                return;
        }

        let value = 0;
        if (item.hold) value = item.negative ? -127 : 127;

        switch (item.target_axis) {
            case CONT_EX_AXIS_LX:
                this.status.joyx = value;
                break;
            case CONT_EX_AXIS_LY:
                this.status.joyy = value;
                break;
            case CONT_EX_AXIS_RX:
                this.status.joy2x = value;
                break;
            case CONT_EX_AXIS_RY:
                this.status.joy2x = value;
                break;
        }
    }

    *poll_queue() {
        for (let i = 0; i < this.#queue.length; i++) {
            this.parse_key_event(this.#queue[i]);
            yield true;
        }
        this.#queue = new Array();
    }

    get has_queued() {
        return this.#queue.length;
    }

    reloadMappings() {
        SETTINGS.get_bind("menuAccept", KOS_KEYBOARD_MAPPING_BUTTONS[1]);
        SETTINGS.get_bind("menuAlternativeTracks", KOS_KEYBOARD_MAPPING_BUTTONS[3]);

        SETTINGS.get_bind("menuSelectorLeft", KOS_KEYBOARD_MAPPING_TRIGGERS[0]);
        SETTINGS.get_bind("menuSelectorRight", KOS_KEYBOARD_MAPPING_TRIGGERS[1]);

        //SETTINGS.get_bind("left0", KOS_KEYBOARD_MAPPING_BUTTONS[5]);
        //SETTINGS.get_bind("down0", KOS_KEYBOARD_MAPPING_BUTTONS[6]);
        //SETTINGS.get_bind("up0", KOS_KEYBOARD_MAPPING_BUTTONS[7]);
        //SETTINGS.get_bind("right0", KOS_KEYBOARD_MAPPING_BUTTONS[8]);
        SETTINGS.get_bind("left1", KOS_KEYBOARD_MAPPING_BUTTONS[9]);
        SETTINGS.get_bind("down1", KOS_KEYBOARD_MAPPING_BUTTONS[10]);
        SETTINGS.get_bind("up1", KOS_KEYBOARD_MAPPING_BUTTONS[11]);
        SETTINGS.get_bind("right1", KOS_KEYBOARD_MAPPING_BUTTONS[12]);
        SETTINGS.get_bind("left2", KOS_KEYBOARD_MAPPING_BUTTONS[13]);
        SETTINGS.get_bind("down2", KOS_KEYBOARD_MAPPING_BUTTONS[14]);
        SETTINGS.get_bind("up2", KOS_KEYBOARD_MAPPING_BUTTONS[15]);
        SETTINGS.get_bind("right2", KOS_KEYBOARD_MAPPING_BUTTONS[16]);
        SETTINGS.get_bind("left3", KOS_KEYBOARD_MAPPING_BUTTONS[17]);
        SETTINGS.get_bind("down3", KOS_KEYBOARD_MAPPING_BUTTONS[18]);
        SETTINGS.get_bind("up3", KOS_KEYBOARD_MAPPING_BUTTONS[19]);
        SETTINGS.get_bind("right3", KOS_KEYBOARD_MAPPING_BUTTONS[20]);

        SETTINGS.get_bind("diamond", KOS_KEYBOARD_MAPPING_BUTTONS[21]);
    }
}

/**
 * Controller status structure.
 * 
 * This structure contains information about the status of the controller
 * device and can be fetched with maple_dev_status().
 * 
 * A 1 bit in the buttons bitfield indicates that a button is pressed, and the
 * joyx, joyy, joy2x, joy2 values are all 0 based (0 is centered).
 */
class cont_state_t {
    /**@type{number} Buttons bitfield.*/
    buttons = 0;

    /**@type{number} Left trigger value. */
    ltrig = 0;
    /**@type{number} Right trigger value. */
    rtrig = 0;

    /**@type{number} Main joystick x-axis value. */
    joyx = 0;
    /**@type{number} Main joystick y-axis value. */
    joyy = 0;

    /**@type{number} Secondary joystick x-axis value (if applicable). */
    joy2x = 0;
    /**@type{number} Secondary joystick y-axis value (if applicable). */
    joy2y = 0;
}


const MAPLE_PORT_COUNT = 4;
const MAPLE_UNIT_COUNT = 6;
const MAPLE_FUNC_CONTROLLER = 0x01000000;
const MAPLE_FUNC_MEMCARD = 0x02000000;
const CONT_C = (1 << 0);
const CONT_B = (1 << 1);
const CONT_A = (1 << 2);
const CONT_START = (1 << 3);
const CONT_DPAD_UP = (1 << 4);
const CONT_DPAD_DOWN = (1 << 5);
const CONT_DPAD_LEFT = (1 << 6);
const CONT_DPAD_RIGHT = (1 << 7);
const CONT_Z = (1 << 8);
const CONT_Y = (1 << 9);
const CONT_X = (1 << 10);
const CONT_D = (1 << 11);
const CONT_DPAD2_UP = (1 << 12);
const CONT_DPAD2_DOWN = (1 << 13);
const CONT_DPAD2_LEFT = (1 << 14);
const CONT_DPAD2_RIGHT = (1 << 15);
// extra keys (not available in the dreamcast)
const CONT_EX_BACK_OR_SELECT = (1 << 16);
const CONT_HOME_OR_GUIDE = (1 << 17);
const CONT_EX_DPAD3_UP = (1 << 18);
const CONT_EX_DPAD3_DOWN = (1 << 19);
const CONT_EX_DPAD3_LEFT = (1 << 20);
const CONT_EX_DPAD3_RIGHT = (1 << 21);
const CONT_EX_DPAD4_UP = (1 << 22);
const CONT_EX_DPAD4_DOWN = (1 << 23);
const CONT_EX_DPAD4_LEFT = (1 << 24);
const CONT_EX_DPAD4_RIGHT = (1 << 25);
// axis placeholders
const CONT_EX_NONE = 0x00000000;
const CONT_EX_TRIGGER_L = (1 << 26);
const CONT_EX_TRIGGER_R = (1 << 27);
const CONT_EX_AXIS_LX = (1 << 28);
const CONT_EX_AXIS_LY = (1 << 29);
const CONT_EX_AXIS_RX = (1 << 30);
const CONT_EX_AXIS_RY = (1 << 31);


const /**@type {maple_device_t[][]}*/KOS_MAPLE_DEVS = new Array(MAPLE_PORT_COUNT);
const KOS_MAPLE_KEYBOARD = new maple_keyboard_device_t();// -1 is the keyboard (JS & C# only)
const KOS_JSGAMEPAD_BUTTONS_MAPPING = [
    CONT_A | CONT_EX_DPAD3_DOWN, CONT_B | CONT_EX_DPAD3_RIGHT, CONT_X | CONT_EX_DPAD3_LEFT, CONT_Y | CONT_EX_DPAD3_UP,
    CONT_C, CONT_Z,
    CONT_EX_TRIGGER_L, CONT_EX_TRIGGER_R,
    CONT_EX_BACK_OR_SELECT, CONT_START,
    CONT_EX_NONE, CONT_EX_NONE,
    CONT_DPAD_UP, CONT_DPAD_DOWN, CONT_DPAD_LEFT, CONT_DPAD_RIGHT,
    CONT_HOME_OR_GUIDE
];
const KOS_JSGAMEPAD_AXES_MAPPING = [
    CONT_EX_AXIS_LX, CONT_EX_AXIS_LY,
    CONT_EX_AXIS_RX, CONT_EX_AXIS_RY
];
const KOS_CONT_PLCHLDR = (1 << 16);// all "not supported" buttons in the dreamcast
const KOS_MAX_DEVICES = MAPLE_PORT_COUNT * MAPLE_UNIT_COUNT;
const KOS_KEYBOARD_MAPPING_BUTTONS = [
    { code: "Enter", button: CONT_START },
    { code: "KeyA", button: CONT_A },
    { code: null, button: CONT_B },
    { code: "KeyX", button: CONT_X },
    { code: null, button: CONT_Y },
    { code: "ArrowUp", button: CONT_DPAD_UP },
    { code: "ArrowDown", button: CONT_DPAD_DOWN },
    { code: "ArrowRight", button: CONT_DPAD_RIGHT },
    { code: "ArrowLeft", button: CONT_DPAD_LEFT },
    { code: "KeyF", button: CONT_DPAD2_LEFT },
    { code: "KeyG", button: CONT_DPAD2_DOWN },
    { code: "KeyJ", button: CONT_DPAD2_UP },
    { code: "KeyK", button: CONT_DPAD2_RIGHT },
    { code: null, button: CONT_EX_DPAD3_LEFT },
    { code: null, button: CONT_EX_DPAD3_DOWN },
    { code: null, button: CONT_EX_DPAD3_UP },
    { code: null, button: CONT_EX_DPAD3_RIGHT },
    { code: null, button: CONT_EX_DPAD4_LEFT },
    { code: null, button: CONT_EX_DPAD4_DOWN },
    { code: null, button: CONT_EX_DPAD4_UP },
    { code: null, button: CONT_EX_DPAD4_RIGHT },
    { code: "Space", button: CONT_D },
    { code: "Escape", button: CONT_EX_BACK_OR_SELECT },
    { code: "Home", button: CONT_HOME_OR_GUIDE }
];
const KOS_KEYBOARD_MAPPING_AXES = [
    { key_low: null, key_high: null, axes: CONT_EX_TRIGGER_L },
    { key_low: null, key_high: null, axes: CONT_EX_TRIGGER_R }
];
const KOS_KEYBOARD_MAPPING_TRIGGERS = [
    { code: "Digit1", trigger: CONT_EX_TRIGGER_L },
    { code: "Digit2", trigger: CONT_EX_TRIGGER_R }
];
const KOS_GAMEPAD_TO_MAPLE_DEVICE = [
    { index: 0, port: 0, unit: 0 },
    { index: 1, port: 1, unit: 0 },
    { index: 2, port: 2, unit: 0 },
    { index: 3, port: 3, unit: 0 },
    { index: 4, port: 0, unit: 2 },
    { index: 5, port: 1, unit: 2 },
    { index: 6, port: 2, unit: 2 },
    { index: 7, port: 3, unit: 2 },
    { index: 8, port: 0, unit: 3 },
    { index: 9, port: 1, unit: 3 },
    { index: 10, port: 2, unit: 3 },
    { index: 11, port: 3, unit: 3 },
    { index: 12, port: 0, unit: 4 },
    { index: 13, port: 1, unit: 4 },
    { index: 14, port: 2, unit: 4 },
    { index: 15, port: 3, unit: 4 }
];
var KOS_MAPLE_slots_allocated = false;


/**
 * Return the number of connected devices.
 * @returns {number} The number of devices connected.
 */
function maple_enum_count() {
    let count = 0;
    for (let port of KOS_MAPLE_DEVS) {
        for (let device of port) {
            if (device != null) count++;
        }
    }
    return count;
}

/**
 * Get the Nth device of the requested type (where N is zero-indexed).
 * @param {*} n The index to look up.
 * @param {*} func The function code to look for.
 * @returns {maple_device_t} The device found, if any. NULL otherwise.
 */
function maple_enum_type(n, func) {
    let i = 0;
    for (let port of KOS_MAPLE_DEVS) {
        for (let device of port) {
            if (device && device.info.functions & func) {
                if (i == n) return device;
                i++;
            }
        }
    }
    return null;
}

/**
 * Get the status struct for the requested maple device.
 * This function will wait until the status is valid before returning. 
 * You should cast to the appropriate type you're expecting.
 * @param {maple_device_t} dev The device to look up
 * @returns {cont_state_t} The device's status data.
 */
function maple_dev_status(dev) {
    dev._peek_gamepad_data();
    return dev._status;
}

/** 
 *  @summary  Get a raw device info struct for the given device.
 *  @param {number} p               The port to look up.
 *  @param {number} u               The unit to look up.
 *  @return  {maple_device_t}       The device at that address, or NULL if no device is there.
*/
function maple_enum_dev(p, u) {
    for (let port of KOS_MAPLE_DEVS) {
        for (let device of port) {
            if (device && device.port == p && device.unit == u) {
                return device.valid ? device : null;
            }
        }
    }
    return null;
}


window.addEventListener("gamepadconnected", KOS_handle_gamepad_connection, false);
window.addEventListener("gamepaddisconnected", KOS_handle_gamepad_connection, false);
window.addEventListener("DOMContentLoaded", function () {
    // create virtual slots
    for (let p = 0; p < MAPLE_PORT_COUNT; p++) {
        KOS_MAPLE_DEVS[p] = new Array(MAPLE_UNIT_COUNT);
        for (let u = 0; u < MAPLE_UNIT_COUNT; u++) {
            KOS_MAPLE_DEVS[p][u] = null;
        }
    }

    // allocate VMUs
    let saveslots = SETTINGS.saveslots;
    for (let i = 0; i < saveslots; i++) {
        if (i < 4) {
            KOS_MAPLE_DEVS[i][1] = new maple_device_t(MAPLE_FUNC_MEMCARD, i, 1);
            continue;
        }

        L_find_upper_available_slot:
        for (let port = 0; port < MAPLE_PORT_COUNT; port++) {
            for (let unit = 0; unit < MAPLE_UNIT_COUNT; unit++) {
                for (let mapping of KOS_GAMEPAD_TO_MAPLE_DEVICE) {
                    if (mapping.port != port && mapping.unit != unit) {
                        // unused space, assign as VMU
                        KOS_MAPLE_DEVS[port][unit] = new maple_device_t(
                            MAPLE_FUNC_MEMCARD, port, unit
                        );
                        break L_find_upper_available_slot;
                    }
                }
            }
        }
    }

    KOS_MAPLE_slots_allocated = true;

    // allocate existing gamepads
    let gamepads = window.navigator.getGamepads();
    for (let i = 0; i < gamepads.length; i++) {
        if (gamepads[i] == null) continue;

        KOS_handle_gamepad_connection(new GamepadEvent(
            gamepads[i].connected ? "gamepadconnected" : "gamepaddisconnected",
            { gamepad: gamepads[i] }
        ));
    }

    // load keyboard mappings
    KOS_MAPLE_KEYBOARD.reloadMappings();
}, false);

function KOS_handle_gamepad_connection(/**@type {GamepadEvent}*/ evt) {
    if (!KOS_MAPLE_slots_allocated) return;

    for (let mapping of KOS_GAMEPAD_TO_MAPLE_DEVICE) {
        if (mapping.index != evt.gamepad.index) continue;

        let device = KOS_MAPLE_DEVS[mapping.port][mapping.unit];

        if (!device) {
            device = new maple_device_t(MAPLE_FUNC_CONTROLLER, mapping.port, mapping.unit);
            device._gamepad_index = evt.gamepad.index;
            KOS_MAPLE_DEVS[mapping.port][mapping.unit] = device;
        }

        device.valid = evt.type == "gamepadconnected";
        device._status.buttons = 0x00;
        device._status.joyx = device._status.joyy = 0;
        device._status.joy2x = device._status.joy2y = 0;
        device._status.ltrig = device._status.rtrig = 0;

        console.log(
            `gamepad id=${evt.gamepad.id} timestamp=${evt.gamepad.timestamp} status=${evt.type}`
        );
    }
}



//
// FS stuff (do not confuse with engine fs_* functions)
//

const O_WRONLY = 1;
const O_RDONLY = 2;
const SEEK_SET = 0;

var kos_localforaje = new LocalForaje("VMU");
var kos_file_handles_id = 0;
/** @type {Map<number, object>} */
var kos_file_handles = new Map();


/** 
 * @summary  Delete the specified file.
 *   This function deletes the specified file from the filesystem. This should
 *   only be used for files, not for directories. For directories, use fs_rmdir()
 *   instead of this function.
 * @param {string}  fn              The path to remove.
 * @return {Promise<number>} 0 on success, -1 on failure.
*/
async function fs_unlink(fn) {
    try {
        await kos_localforaje.delete(fn);
        return 0;
    } catch (e) {
        console.error("fs_unlink() " + fn, e);
        return -1;
    }
}

/** 
 * @summary  Open a file on the VFS.
 *   This function opens the specified file, returning a new file descriptor to
 *   access the file.
 *   @param {string} fn              The path to open.
 *   @param {number}  mode            The mode to use with opening the file. This may
                            include the standard open modes (O_RDONLY, O_WRONLY,
                            etc), as well as values from the \ref open_modes
                            list. Multiple values can be ORed together.
    @return {Promise<number>}                 The new file descriptor on success, -1 on error.
*/
async function fs_open(fn, mode) {
    try {
        let data = await kos_localforaje.get(fn);
        if (data === undefined && mode == O_RDONLY) return -1;

        let hnd = kos_file_handles_id++;
        let obj = { fn, mode, mmap: new Array(), offset: 0, has_writes: false };

        if (data != null) obj.mmap.push({ offset: 0, data });
        kos_file_handles.set(hnd, obj);
        return hnd;
    } catch (e) {
        console.error("fs_open() " + fn, e);
        return -1;
    }
}

/** 
 * @summary  Close an opened file.
 *   This function closes the specified file descriptor, releasing all resources
 *   associated with the descriptor.
 *   @param {number} hnd             The file descriptor to close.
 *   @return {Promise<number>}                0 for success, -1 for error
*/
async function fs_close(hnd) {
    if (hnd < 0) return -1;

    let obj = kos_file_handles.get(hnd);

    if (obj.has_writes && obj.mode == O_WRONLY) {
        let length = 0;
        for (let chunk of obj.mmap) {
            let len = chunk.data.byteLength + chunk.offset;
            if (len > length) length = len;
        }

        let data = new Uint8Array(length);
        for (let chunk of obj.mmap) data.set(new Uint8Array(chunk.data), chunk.offset);

        try {
            await kos_localforaje.set(obj.fn, data.buffer);
        } catch (e) {
            delete obj.mmap;
            console.error("fs_close() " + obj.fn, e);
            return -1;
        }
    }

    return kos_file_handles.delete(hnd) ? 0 : -1;
}

/**
 * @summary  Write to an opened file.
 *   This function writes the specfied buffer into the file at the current file
 *   pointer.
 *   @param {number} hnd             The file descriptor to write into.
 *   @param {DataView|DataViewEx|ArrayBuffer|Uint8Array} buffer          The data to write into the file.
 *   @param {number} cnt             The size of the buffer, in bytes.
 *   @return {number}                 The number of bytes written, or -1 on failure. Note
 *                           that the number of bytes written may be less than
 *                           what was requested.
*/
function fs_write(hnd, buffer, cnt) {
    if (hnd < 0) return -1;

    let obj = kos_file_handles.get(hnd);

    if (obj.mode != O_WRONLY) throw new Error("opened for read-only");

    /**@type {ArrayBuffer} */
    let data = null;

    if (buffer instanceof ArrayBuffer)
        data = buffer.slice(0, cnt);
    else if (buffer instanceof DataView)
        data = buffer.buffer.slice(buffer.byteOffset, cnt);
    else if (buffer instanceof Uint8Array)
        data = buffer.buffer.slice(buffer.byteOffset, cnt);
    else if (buffer instanceof DataViewEx)
        data = buffer.Buffer.buffer.slice(buffer.Buffer.byteOffset, cnt);
    else
        return -1;

    obj.mmap.push({ offset: obj.offset, data });
    obj.offset += cnt;
    obj.has_writes = true;

    return cnt;
}

/** 
 * @summary  Retrieve the length of an opened file.
    This file retrieves the length of the file associated with the given file
    descriptor.
    @param  {number}  hnd             The file descriptor to retrieve the size from.
    @return  {number}                The length of the file on success, -1 on failure.
*/
function fs_total(hnd) {
    if (hnd < 0) return -1;

    let obj = kos_file_handles.get(hnd);
    if (!obj) return -1;

    let length = 0;
    for (let chunk of obj.mmap) {
        let len = chunk.offset + chunk.data.byteLength;
        if (len > length) length = len;
    }

    return length;
}

/** 
 * @summary  Seek to a new position within a file.
 *  This function moves the file pointer to the specified position within the
 *  file (the base of this position is determined by the whence parameter).
 *  @param {number} hnd             The file descriptor to move the pointer for.
 *  @param {number} offset          The offset in bytes from the specified base.
 *  @param {number} whence          The base of the pointer move. This should be one of
 *                          the \ref seek_modes values.
 *  @return {number}                The new position of the file pointer.
*/
function fs_seek(hnd, offset, whence) {
    if (hnd < 0) return -1;

    let obj = kos_file_handles.get(hnd);
    if (!obj) return -1;

    if (whence == SEEK_SET) {
        obj.offset = offset;
        return offset;
    }

    throw new Error("whence not implemented:" + whence);
}

/** 
 * @summary  Read from an opened file.
 *   This function reads into the specified buffer from the file at its current
 *   file pointer.
 *   @param {number} hnd             The file descriptor to read from.
 *   @param {Uint8Array} buffer          The buffer to read into.
 *   @param {number} cnt             The size of the buffer (or the number of bytes
 *                           requested).
 *   @return {number}                 The number of bytes read, or -1 on error. Note that
 *                           this may not be the full number of bytes requested.
*/
function fs_read(hnd, buffer, cnt) {
    if (hnd < 0) return -1;
    if (!buffer) return -1;

    let obj = kos_file_handles.get(hnd);
    if (!obj) return -1;

    // warning: this just a STUB and will not iterate all chunks to read the required data
    for (let i = 0; i < obj.mmap.length; i++) {
        let chunk;
        if (obj.mmap[i].offset == obj.offset)
            chunk = obj.mmap[i];
        else if (obj.mmap[i].offset >= obj.offset)
            chunk = obj.mmap[Math.max(i - 1, 0)];
        else
            continue;

        let read = Math.min(cnt, chunk.data.byteLength);
        buffer.set(new Uint8Array(chunk.data, obj.offset - chunk.offset, read), 0);
        obj.offset += read;
        return read;
    }

    return -1;
}

/** 
 * @summary  Retrieve information about the specified path.
    This function retrieves status information on the given path. This function
    now returns the normal POSIX-style struct stat, rather than the old KOS
    stat_t structure. In addition, you can specify whether or not this function
    should resolve symbolic links on filesystems that support symlinks.
    @param {string} path            The path to retrieve information about.
    @param {object} buf             The buffer to store stat information in.
    @param {number} flag            Specifies whether or not to resolve a symbolic link.
                            If you don't want to resolve any symbolic links at
                            the end of the path, pass AT_SYMLINK_NOFOLLOW,
                            otherwise pass 0.
    @returns {number}                 0 on success, -1 on failure.
*/
function fs_stat(path, buf, flag) {
    if (path != "/vmu/") return -1;

    void flag;

    // STUB
    buf.st_size = 800 * 16;
    buf.st_blksize = 512;
    return 0;
}


//
// uname stuff
//
const UNAME_KERNEL = "KallistiOS";
const UNAME_MACHINE = "Dreamcast";

/**
 * @typedef {object} utsname
 * @property {string} sysname  OS Name ("KallistiOS").
 * @property {string} nodename Name on network, if any.
 * @property {string} release  Kernel release ("2.1.0").
 * @property {string} version  Kernel version string.
 * @property {string} machine  Hardware identifier.
 */

/**
 * Retrieve version and other similar information about the kernel.
 * This function retrieves information about the current version of the kernel
 * that is running, storing it in the provided buffer.
 * @param {utsname} n   The buffer to store version information in.
 * @returns {number}    0 on success, -1 on error (setting errno appropriately).
*/
function uname(/**@type {utsname}*/ n) {
    if (!n) return -1;
    const kern_version = "2.0.0-503-gc3e7be3";// STUB

    n.nodename = null;
    n.sysname = UNAME_KERNEL;
    n.release = kern_version;
    n.version = `${UNAME_KERNEL} ${kern_version}`;
    n.machine = UNAME_MACHINE + " (abstracted on javascript)";
    return 0;
}

//
// VMU stuff
//

/**
 * @summary  VMU Package type.
 *   Anyone wanting to package a VMU file should create one of these somewhere;
 *   eventually it will be turned into a flat file that you can save using
 *   fs_vmu.
*/
class vmu_pkg_t {
    /** @type {number} Number of data (payload) bytes */
    data_len;
    /** @type {Uint8Array} Payload data */
    data;
}

/**
 * @summary  Parse an array of uint8s into a vmu_pkg_t.
 *   This function does the opposite of vmu_pkg_build and is used to parse VMU
 *   files read in.
 *   @param {ArrayBuffer} data            The buffer to parse.
 *   @param {vmu_pkg_t} pkg             Where to store the vmu_pkg_t.
 *   @returns {number} -1 On invalid CRC in the data. 0 On success.
*/
function vmu_pkg_parse(data, pkg) {
    let dataView = new DataViewEx(data);
    if (data.byteLength < 128) return -1;

    let icon_count = dataView.getUint16At(0x40);
    let eyecatch_type = dataView.getUint16At(0x44);
    let payload_size = dataView.getUint16At(0x48);
    let eyecatch_size = -1;

    switch (eyecatch_type) {
        case 0:
            eyecatch_size = 0;
            break;
        case 1:
            eyecatch_size = 72 * 56 * 2;
            break;
        case 2:
            eyecatch_size = 512 + 72 * 56;
            break;
        case 3:
            eyecatch_size = 32 + 72 * 56 / 2;
            break;
        default:
            console.error("unknown eyecatch type");
            return -1;
    }

    let payload_offset = (icon_count * 512) + eyecatch_size + 0x80;

    if ((payload_offset + payload_size) > data.byteLength) return -1;

    pkg.data_len = payload_size;
    pkg.data = new Uint8Array(data, payload_offset, payload_size);
    return 0;
}

/**
 * Kernel "exit to menu" call.
 */
function arch_menu() {
    if (window.history.length > 1) window.history.back();
    window.close();
    window.location.href = "about:blank";
}


//
// mutex stuff
//
const MUTEX_TYPE_NORMAL = 1;

function mutex_init(m, mtype) {
    if (mtype != MUTEX_TYPE_NORMAL) return -1;
    // STUB
    return 0;
}

function mutex_destroy(m) {
    return 0;
}

function mutex_lock(m) {
    return 0;
}

function mutex_unlock(m) {
    return 0;
}

function MUTEX_INITIALIZER() {
    // STUB
    return 0;
}


// sound interface stuff

function snd_sh4_to_aica_start() {
    // STUB
}
