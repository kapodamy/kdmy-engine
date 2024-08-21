using System;
using System.Runtime.CompilerServices;
using Engine.Utils;
using KallistiOS.MAPLE;
using KallistiOS.TIMER;

namespace Engine.Platform;

[Flags]
public enum GamepadButtons : uint {
    NOTHING = 0 << 0,

    A = 1 << 0,
    B = 1 << 1,
    X = 1 << 2,
    Y = 1 << 3,

    DPAD_UP = 1 << 4,
    DPAD_DOWN = 1 << 5,
    DPAD_RIGHT = 1 << 6,
    DPAD_LEFT = 1 << 7,

    START = 1 << 8,
    SELECT = 1 << 9,

    TRIGGER_LEFT = 1 << 10,
    TRIGGER_RIGHT = 1 << 11,
    BUMPER_LEFT = 1 << 12,
    BUMPER_RIGHT = 1 << 13,

    APAD_UP = 1 << 14,
    APAD_DOWN = 1 << 15,
    APAD_RIGHT = 1 << 16,
    APAD_LEFT = 1 << 17,

    DPAD2_UP = 1 << 18,
    DPAD2_DOWN = 1 << 19,
    DPAD2_RIGHT = 1 << 20,
    DPAD2_LEFT = 1 << 21,

    DPAD3_UP = 1 << 22,
    DPAD3_DOWN = 1 << 23,
    DPAD3_RIGHT = 1 << 24,
    DPAD3_LEFT = 1 << 25,

    DPAD4_UP = 1 << 26,
    DPAD4_DOWN = 1 << 27,
    DPAD4_RIGHT = 1 << 28,
    DPAD4_LEFT = 1 << 29,

    BACK = 1 << 30,


    // combinations
    AD_UP = DPAD_UP | APAD_UP,
    AD_DOWN = DPAD_DOWN | APAD_DOWN,
    AD_RIGHT = DPAD_RIGHT | APAD_RIGHT,
    AD_LEFT = DPAD_LEFT | APAD_LEFT,
    AD = AD_DOWN | AD_UP | AD_LEFT | AD_RIGHT,
    T_LR = TRIGGER_LEFT | TRIGGER_RIGHT,
    B_LR = BUMPER_LEFT | BUMPER_RIGHT,
    DALL_UP = DPAD_UP | DPAD2_UP | DPAD3_UP | DPAD4_UP,
    DALL_DOWN = DPAD_DOWN | DPAD2_DOWN | DPAD3_DOWN | DPAD4_DOWN,
    DALL_RIGHT = DPAD_RIGHT | DPAD2_RIGHT | DPAD3_RIGHT | DPAD4_RIGHT,
    DALL_LEFT = DPAD_LEFT | DPAD2_LEFT | DPAD3_LEFT | DPAD4_LEFT,

    // invalid value
    INVALID = UInt32.MaxValue
}

public struct GamepadButtonsChanges {
    public GamepadButtons previous;
    public GamepadButtons current;
}


public class Gamepad {

    public const int ANALOG_DEAD_ZONE = 0x40;// considered "pressed" any analog input if 25% or more

    private GamepadButtons buttons;
    private GamepadButtons last_buttons;
    private GamepadButtons clear_mask;
    private int delay_duration;
    private double delay_timestamp;
    private int controller_index;
    private maple_device_t device;
    private bool mix_keyboard;
    private int id;


    private static int IDS = 0;
    public static readonly Map<Gamepad> POOL = new Map<Gamepad>();
    public static readonly CONT[] BUTTONS_MAP_FROM = {
        CONT.A, CONT.B, CONT.X, CONT.Y,
        CONT.DPAD_UP, CONT.DPAD_DOWN, CONT.DPAD_LEFT, CONT.DPAD_RIGHT,
        CONT.START, CONT.D,
        CONT.BACK_OR_SELECT,
        CONT.C, CONT.Z,
        CONT.DPAD2_UP, CONT.DPAD2_DOWN, CONT.DPAD2_LEFT, CONT.DPAD2_RIGHT,
        CONT.DPAD3_UP, CONT.DPAD3_DOWN, CONT.DPAD3_LEFT, CONT.DPAD3_RIGHT,
        CONT.DPAD4_UP, CONT.DPAD4_DOWN, CONT.DPAD4_LEFT, CONT.DPAD4_RIGHT,
    };
    public static readonly GamepadButtons[] BUTTONS_MAP_TO = {
        GamepadButtons.A, GamepadButtons.B, GamepadButtons.X, GamepadButtons.Y,
        GamepadButtons.DPAD_UP, GamepadButtons.DPAD_DOWN, GamepadButtons.DPAD_LEFT, GamepadButtons.DPAD_RIGHT,
        GamepadButtons.START, GamepadButtons.SELECT,
        GamepadButtons.BACK,
        GamepadButtons.BUMPER_LEFT, GamepadButtons.BUMPER_RIGHT,
        GamepadButtons.DPAD2_UP, GamepadButtons.DPAD2_DOWN, GamepadButtons.DPAD2_LEFT, GamepadButtons.DPAD2_RIGHT,
        GamepadButtons.DPAD3_UP, GamepadButtons.DPAD3_DOWN, GamepadButtons.DPAD3_LEFT, GamepadButtons.DPAD3_RIGHT,
        GamepadButtons.DPAD4_UP, GamepadButtons.DPAD4_DOWN, GamepadButtons.DPAD4_LEFT, GamepadButtons.DPAD4_RIGHT,
    };


    public Gamepad(int controller_device_index)
        : this(controller_device_index, true) { }

    public Gamepad(int controller_device_index, bool mix_keyboard) {

        this.buttons = 0x00;
        this.last_buttons = 0x00;
        this.clear_mask = GamepadButtons.NOTHING;
        this.controller_index = controller_device_index;
        this.delay_duration = 0;
        this.delay_timestamp = 0;
        this.device = null;
        this.mix_keyboard = controller_device_index < 1 && mix_keyboard;
        this.id = IDS++;

        Gamepad.POOL.Set(this.id, this);

        // check if should use any controller attached
        if (controller_device_index < 0) return;

        // pick dreamcast gamepad if connected
        InternalPickMapleDevice();

    }

    public void Destroy() {
        POOL.Delete(this);

        //free(gamepad);
    }

    public bool IsDettached() {
        if (this.mix_keyboard) return false;
        return this.device != null ? (!this.device.valid) : false;
    }

    public bool Pick(bool pick_on_start_press) {
        if (this.device != null) {
            if (this.device.valid) return true;
        } else if (this.InternalPickMapleDevice()) return true;
        if (!pick_on_start_press) return false;

        //
        // Pick another maple controller connected to the dreamcast
        // but also check if not used by any other gamepad instance
        // the user must press the START button to detect them
        //
        int maple_devices_found = maple.enum_count();
        for (int i = 0 ; i < maple_devices_found ; i++) {
            maple_device_t device = maple.enum_type(i, MAPLE_FUNC.CONTROLLER);
            if (device != null || !device.valid) continue;

            bool in_use = false;
            foreach (Gamepad j in Gamepad.POOL) {
                if (j.device == device) {
                    in_use = true;
                    break;
                }
            }

            if (in_use) continue;

            // check if the user is pressing the button START
            cont_state_t status = maple.dev_status(device);
            if ((status.buttons & CONT.START) != 0x00) {
                this.device = device;
                return true;
            }
        }


        return false;
    }

    public int GetControllerIndex() {
        return this.controller_index;
    }

    public void SetButtonsDelay(int delay_time) {
        this.delay_duration = delay_time;
        this.delay_timestamp = timer.ms_gettime64() + delay_time;
    }

    public void EnforceButtonsDelay() {
        this.delay_timestamp = timer.ms_gettime64() + this.delay_duration;
    }


    public void ClearButtons() {
        this.clear_mask = 0x00;
        this.InternalUpdateState();
        this.clear_mask = ~this.buttons;
        this.buttons = GamepadButtons.NOTHING;
    }

    public void ClearButtons2(GamepadButtons buttons_to_clear) {
        this.clear_mask = 0x00;
        this.InternalUpdateState();
        this.clear_mask = ~(this.buttons & buttons_to_clear);
        this.buttons &= ~buttons_to_clear;
    }

    public static void ClearAllGamepads() {
        foreach (Gamepad gamepad in Gamepad.POOL) {
            gamepad.clear_mask = GamepadButtons.NOTHING;
            gamepad.InternalUpdateState();
            gamepad.clear_mask = ~gamepad.buttons;
            //gamepad.last_buttons = gamepad.buttons;
            gamepad.buttons = GamepadButtons.NOTHING;
        }
    }

    public GamepadButtons GetPressed() {
        InternalUpdateState();
        return this.buttons;
    }

    public GamepadButtons GetLastPressed() {
        return this.buttons;
    }

    public bool GetManagedPresses(bool update_state, ref GamepadButtons buttons) {
        if (update_state) InternalUpdateState();
        if (this.buttons == this.last_buttons) return false;

        buttons = this.buttons;
        this.last_buttons = this.buttons;
        return true;
    }

    public GamepadButtons HasPressed(GamepadButtons button_flags) {
        InternalUpdateState();

        return (this.buttons & button_flags)/* != 0*/;
    }

    public GamepadButtons HasPressedDelayed(GamepadButtons button_flags) {
        InternalUpdateState();

        GamepadButtons pressed = this.buttons & button_flags;
        if (pressed == GamepadButtons.NOTHING || this.delay_duration < 1) return 0x00;

        double now = timer.ms_gettime64();
        if (now < this.delay_timestamp) return 0x00;

        this.delay_timestamp = now + this.delay_duration;
        return pressed;
    }

    public GamepadButtonsChanges DirectStateUpdate() {
        if (this.device == null || this.device.valid) {
            if (this.mix_keyboard) {
                return new GamepadButtonsChanges() {
                    previous = this.GetLastPressed(),
                    current = this.GetPressed()
                };
            }

            return new GamepadButtonsChanges() {
                previous = GamepadButtons.INVALID,
                current = GamepadButtons.INVALID
            };
        }

        GamepadButtons old = this.buttons;
        this.buttons = GamepadButtons.NOTHING;

        cont_state_t controller_status = maple.dev_status(this.device);
        InternalReadDevice(controller_status);

        // (JS & C# only) read the keyboard keys pressed (if required)
        if (this.mix_keyboard) {
            InternalReadDevice(maple.STUB_KEYBOARD.status);
        }

        return new GamepadButtonsChanges() {
            previous = old,
            current = this.buttons
        };
    }


    private bool InternalPickMapleDevice() {
        int index = 0;
        int maple_devices_found = maple.enum_count();

        // find the desired controller number/index/position
        for (int i = 0 ; i < maple_devices_found ; i++) {
            maple_device_t device = maple.enum_type(i, MAPLE_FUNC.CONTROLLER);
            if (device == null) continue;

            if (index == this.controller_index) {
                this.device = device;
                return true;
            }

            index++;
        }

        return false;
    }

    private void InternalReadDevice(cont_state_t controller_status) {
        // digital buttons
        for (int i = 0 ; i < Gamepad.BUTTONS_MAP_FROM.Length ; i++) {
            CONT source = Gamepad.BUTTONS_MAP_FROM[i];
            GamepadButtons target = Gamepad.BUTTONS_MAP_TO[i];
            if ((controller_status.buttons & source) != 0x00) this.buttons |= target;
        }

        // analog left trigger
        if (controller_status.ltrig >= Gamepad.ANALOG_DEAD_ZONE) this.buttons |= GamepadButtons.TRIGGER_LEFT;
        // analog right trigger
        if (controller_status.rtrig >= Gamepad.ANALOG_DEAD_ZONE) this.buttons |= GamepadButtons.TRIGGER_RIGHT;

        // analog left stick
        if (controller_status.joyx <= -Gamepad.ANALOG_DEAD_ZONE)
            this.buttons |= GamepadButtons.APAD_LEFT;
        else if (controller_status.joyx >= Gamepad.ANALOG_DEAD_ZONE)
            this.buttons |= GamepadButtons.APAD_RIGHT;
        // analog right stick
        if (controller_status.joyy <= -Gamepad.ANALOG_DEAD_ZONE)
            this.buttons |= GamepadButtons.APAD_DOWN;
        else if (controller_status.joyy >= Gamepad.ANALOG_DEAD_ZONE)
            this.buttons |= GamepadButtons.APAD_UP;
    }

    private void InternalUpdateState() {
        this.buttons = 0x00;

        if (this.device != null) {
            // read controller status
            cont_state_t controller_status = maple.dev_status(this.device);
            // map the buttons in a form the engine can understand
            InternalReadDevice(controller_status);
        } else if (this.controller_index < 0) {
            int count = maple.enum_count();
            for (int i = 0 ; i < count ; i++) {
                maple_device_t device = maple.enum_type(i, MAPLE_FUNC.CONTROLLER);
                if (device != null) {
                    cont_state_t controller_status = maple.dev_status(device);
                    InternalReadDevice(controller_status);
                }
            }
        }

        // (JS & C# only) read the keyboard keys pressed (if required)
        if (this.mix_keyboard) {
            InternalReadDevice(maple.STUB_KEYBOARD.status);
        }

        // apply clear mask if necessary
        InternalApplyClearMask();
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void InternalApplyClearMask() {
        if (this.clear_mask != GamepadButtons.NOTHING) {
            // forget released buttons present in the mask
            this.clear_mask = ~(~this.clear_mask & this.buttons);

            // clear pressed buttons
            this.buttons &= this.clear_mask;
        }
    }

}

public static class GamepadButtonsExtensions {

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool Bool(this GamepadButtons buttons) {
        return buttons != GamepadButtons.NOTHING;
    }
}
