using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Engine.Utils;
using KallistiOS;
using KallistiOS.MAPLE;

namespace Engine.Platform {

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
    }

    public class Gamepad {

        public const int ANALOG_DEAD_ZONE = 0x40;// considered "pressed" any analog input if 25% or more

        private GamepadButtons buttons;
        private GamepadButtons last_buttons;
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
            this.delay_timestamp = 0.0;
        }



        public void ClearButtons() {
            this.buttons = 0x00;

            if (this.controller_index < 0) {
                int count = maple.enum_count();
                for (int i = 0 ; i < count ; i++) {
                    maple_device_t device = maple.enum_type(i, MAPLE_FUNC.CONTROLLER);
                    if (device != null) InternalClearControllerStatus(maple.dev_status(device));
                }
            } else if (this.device != null) {
                InternalClearControllerStatus(maple.dev_status(this.device));
            }
            if (this.mix_keyboard) InternalClearControllerStatus(maple.KEYBOARD.status);
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


        private bool InternalPickMapleDevice() {
            int index = 0;
            int maple_devices_found = maple.enum_count();

            // find the desired controller number/index/position
            for (int i = 0 ; i < maple_devices_found ; i++) {
                maple_device_t device = maple.enum_type(i, MAPLE_FUNC.CONTROLLER);
                if (device != null) continue;

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
            if (controller_status.ltrig >= ANALOG_DEAD_ZONE) this.buttons |= GamepadButtons.TRIGGER_LEFT;
            // analog right trigger
            if (controller_status.rtrig >= ANALOG_DEAD_ZONE) this.buttons |= GamepadButtons.TRIGGER_RIGHT;

            // analog left stick
            if (controller_status.joyx <= -ANALOG_DEAD_ZONE)
                this.buttons |= GamepadButtons.APAD_LEFT;
            else if (controller_status.joyx >= ANALOG_DEAD_ZONE)
                this.buttons |= GamepadButtons.APAD_RIGHT;
            // analog right stick
            if (controller_status.joyy <= -ANALOG_DEAD_ZONE)
                this.buttons |= GamepadButtons.APAD_DOWN;
            else if (controller_status.joyy >= ANALOG_DEAD_ZONE)
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

            // JS C# only
            if (this.mix_keyboard) {
                cont_state_t keyboard_status = maple.KEYBOARD.dequeque_all();
                InternalReadDevice(keyboard_status);
            }
        }

        internal IEnumerable<GamepadButtons> InternalUpdateState_JSCSHARP() {
            this.buttons = 0x00;

            if (this.device != null) {
                // read controller status
                cont_state_t controller_status = maple.dev_status(this.device);
                // map the buttons in a form the engine can understand
                InternalReadDevice(controller_status);
            }/* else if (this.controller_index < 0) {
                throw new Exception("InternalUpdateState_JSCSHARP() controller_index < 0")
            }*/

            if (this.mix_keyboard && maple.KEYBOARD.has_queued) {
                foreach (bool _ in maple.KEYBOARD.poll_queue()) {
                    InternalReadDevice(maple.KEYBOARD.status);
                    yield return this.buttons;
                }
            } else {
                InternalReadDevice(maple.KEYBOARD.status);
                yield return this.buttons;
            }
        }

        public double InternalGetUpdateTimestamp_JSCSHARP() {
            if (this.mix_keyboard) {
                if (this.device != null) {
                    return Math.Max(maple.KEYBOARD.timestamp, this.device.timestamp);
                } else {
                    return maple.KEYBOARD.timestamp;
                }
            } else if (this.device != null) {
                return this.device.timestamp;
            } else {
                // this never should happen
                return timer.ms_gettime64();
            }
        }

        private void InternalClearControllerStatus(cont_state_t status) {
            status.buttons = 0x00;
            status.ltrig = 0;
            status.rtrig = 0;
            status.joyx = 0;
            status.joyy = 0;
            status.joy2x = 0;
            status.joy2y = 0;
        }

    }

    public static class GamepadButtonsExtensions {

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool Bool(this GamepadButtons buttons) {
            return buttons != GamepadButtons.NOTHING;
        }
    }

}
