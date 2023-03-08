using System;
using System.Collections.Generic;
using CsharpWrapper;
using Engine.Externals.GLFW;

namespace KallistiOS.MAPLE {

    [Flags]
    public enum CONT : uint {
        NOTHING = (0 << 0),

        C = (1 << 0),
        B = (1 << 1),
        A = (1 << 2),
        START = (1 << 3),
        DPAD_UP = (1 << 4),
        DPAD_DOWN = (1 << 5),
        DPAD_LEFT = (1 << 6),
        DPAD_RIGHT = (1 << 7),
        Z = (1 << 8),
        Y = (1 << 9),
        X = (1 << 10),
        D = (1 << 11),
        DPAD2_UP = (1 << 12),
        DPAD2_DOWN = (1 << 13),
        DPAD2_LEFT = (1 << 14),
        DPAD2_RIGHT = (1 << 15),

        // extra keys (not available in the dreamcast)
        BACK_OR_SELECT = (1 << 21),
        HOME_OR_GUIDE = (1 << 22),
        DPAD3_UP = (1 << 23),
        DPAD3_DOWN = (1 << 24),
        DPAD3_LEFT = (1 << 25),
        DPAD3_RIGHT = (1 << 26),
        DPAD4_UP = (1 << 27),
        DPAD4_DOWN = (1 << 28),
        DPAD4_LEFT = (1 << 29),
        DPAD4_RIGHT = (1 << 30),
    }

    // internal use only
    internal enum CONTEx {
        NONE,
        TRIGGER_L, TRIGGER_R,
        AXIS_LX, AXIS_LY,
        AXIS_RX, AXIS_RY
    }

    [Flags]
    public enum MAPLE_FUNC : uint {
        NOTHING = 0x00000000,
        CONTROLLER = 0x01000000,
        MEMCARD = 0x02000000
    }

    public static class maple {

        private const int MAPLE_PORT_COUNT = 4;
        private const int MAPLE_UNIT_COUNT = 6;
        private static readonly JoystickCallback joystickCallback;

        private static maple_device_t[,] DEVICES;
        public static maple_keyboard_device_t KEYBOARD;

        public static int enum_count() {
            int count = 0;
            for (int port = 0 ; port < MAPLE_PORT_COUNT ; port++) {
                for (int unit = 0 ; unit < MAPLE_UNIT_COUNT ; unit++) {
                    if (DEVICES[port, unit] != null) count++;
                }
            }
            return count;
        }

        public static maple_device_t enum_type(int n, MAPLE_FUNC func) {
            int i = 0;
            foreach (maple_device_t device in DEVICES) {
                if (device == null) continue;
                if ((device.info.functions & func) != MAPLE_FUNC.NOTHING) {
                    if (i == n) return device;
                    i++;
                }
            }
            return null;
        }

        public static cont_state_t dev_status(maple_device_t dev) {
            dev._peek_gamepad_data();
            return dev._status;
        }


        public static maple_device_t enum_dev(uint p, uint u) {
            foreach (maple_device_t device in DEVICES) {
                if (device != null && device.port == p && device.unit == u) {
                    return device.valid ? device : null;
                }
            }
            return null;
        }


        internal static void __initialize(Window nativeWindow) {
            maple.DEVICES = new maple_device_t[MAPLE_PORT_COUNT, MAPLE_UNIT_COUNT];

            // allocate VMUs
            int saveslots = EngineSettings.GetInt(false, "saveslots", 1);
            for (int i = 0 ; i < saveslots ; i++) {
                if (i < 4) {
                    maple.DEVICES[i, 1] = new maple_device_t(MAPLE_FUNC.MEMCARD, (uint)i, 1U);
                    continue;
                }

                for (uint port = 0 ; port < MAPLE_PORT_COUNT ; port++) {
                    for (uint unit = 0 ; unit < MAPLE_UNIT_COUNT ; unit++) {
                        foreach (var mapping in maple_mappings.GAMEPAD_TO_MAPLE_DEVICE) {
                            if (mapping.port != port && mapping.unit != unit) {
                                // unused space, assign as VMU
                                maple.DEVICES[port, unit] = new maple_device_t(
                                    MAPLE_FUNC.MEMCARD, port, unit
                                );
                                goto L_continue;
                            }
                        }
                    }
                }

L_continue:
                continue;
            }

            // STUB keyboard
            maple.KEYBOARD = new maple_keyboard_device_t();
            Glfw.SetKeyCallback(nativeWindow, maple.KEYBOARD.keyCallback);

            // gamepad connection/disconnection
            Glfw.SetJoystickCallback(maple.joystickCallback);

            for (int i = 0, total = Glfw.JOYSTICK_LAST ; i < total ; i++) {
                if (Glfw.JoystickIsGamepad(i)) InternalGamepadConnection(i, ConnectionStatus.Connected);
            }
        }

        private static void InternalGamepadConnection(int joystick, ConnectionStatus status) {
            foreach (var mapping in maple_mappings.GAMEPAD_TO_MAPLE_DEVICE) {
                if (mapping.index != joystick) continue;

                maple_device_t device = DEVICES[mapping.port, mapping.unit];

                if (device == null) {
                    device = new maple_device_t(MAPLE_FUNC.CONTROLLER, mapping.port, mapping.unit);
                    DEVICES[mapping.port, mapping.unit] = device;
                }

                device.valid = status == ConnectionStatus.Connected;
                device._status.buttons = CONT.NOTHING;
                device._status.joyx = device._status.joyy = 0;
                device._status.joy2x = device._status.joy2y = 0;
                device._status.ltrig = device._status.rtrig = 0;

                if (device.valid) {
                    Console.Error.WriteLine(
                        "[LOG] gamepad connected id={0} name={1}",
                        (int)joystick,
                        Glfw.GetJoystickName(joystick)
                    );
                }
            }
        }

        internal static void PollAllGamepads() {
            for (int i = 0 ; i < maple_mappings.GAMEPAD_TO_MAPLE_DEVICE.Length ; i++) {
                var mapping = maple_mappings.GAMEPAD_TO_MAPLE_DEVICE[i];
                maple_device_t device = DEVICES[mapping.port, mapping.unit];
                if (device != null && device.valid) device._peek_gamepad_data();
            }
        }

        static maple() {
            maple.joystickCallback = InternalGamepadConnection;
        }

    }

    /** 
     * @summary  Maple device info structure.
        This structure is used by the hardware to deliver the response to the device
        info request.
*/
    public class maple_devinfo_t {
        /** @type {number} Function codes supported */
        public MAPLE_FUNC functions;
    }

    /**
     * One maple device.
     *   Note that we duplicate the port/unit info which is normally somewhat
     *   implicit so that we can pass around a pointer to a particular device struct.
     */
    public class maple_device_t {

        /** @type {bool} Is this a valid device?*/
        public bool valid;
        /** @type {uint} Maple bus port connected to*/
        public uint port;
        /** @type {uint} Unit number, off of the port*/
        public uint unit;
        /** @type {maple_devinfo_t} Device info struct*/
        public maple_devinfo_t info;

        public double timestamp = 0;
        private int _gamepad_index;
        internal cont_state_t _status;


        internal maple_device_t(MAPLE_FUNC funcs, uint port, uint unit) {
            this.port = port;
            this.unit = unit;
            this.info = new maple_devinfo_t() { functions = funcs };
            this.valid = (funcs & MAPLE_FUNC.MEMCARD) != MAPLE_FUNC.NOTHING;
            this._status = new cont_state_t();
            this._gamepad_index = -1;
        }


        internal void _peek_gamepad_data() {
            //
            // Notes:
            //    * The dreamcast controller does not have enough buttons/axes like modern controllers.
            //    * If the controller has additional buttons and/or axes, they are reported
            //      by KallistiOS using CONT_CAPABILITY_* in maple_enum_type*() functions.
            //
            GamePadState state;
            if (!Glfw.GetGamepadState(this._gamepad_index, out state)) {
                this.valid = false;
                return;
            }

            double new_timestamp = Glfw.GetTime() * 1000.0;
            if (new_timestamp == this.timestamp) return;

            this.timestamp = new_timestamp;

            // read the gamepad buttons using the standard layout
            for (int i = 0 ; i < maple_mappings.GAMEPAD_BUTTONS_MAPPING.Length ; i++) {
                var mapping = maple_mappings.GAMEPAD_BUTTONS_MAPPING[i];
                InputState button_state = state.buttons[(int)mapping.glfw_button];

                if (button_state == InputState.Repeat) continue;

                if (button_state == InputState.Press)
                    this._status.buttons |= mapping.cont_button;
                else if (button_state == InputState.Release)
                    this._status.buttons &= ~mapping.cont_button;
            }

            // read the gamepad axes using the standard layout
            for (int i = 0 ; i < maple_mappings.GAMEPAD_AXES_MAPPING.Length ; i++) {
                var mapping = maple_mappings.GAMEPAD_AXES_MAPPING[i];
                float axis = state.axes[(int)mapping.glfw_axis];
                int value = (int)(axis * 0x7F);

                switch (mapping.axis) {
                    case CONTEx.AXIS_LX:
                        this._status.joyx = value;
                        break;
                    case CONTEx.AXIS_LY:
                        this._status.joyy = value;
                        break;
                    case CONTEx.AXIS_RX:
                        this._status.joy2x = value;
                        break;
                    case CONTEx.AXIS_RY:
                        this._status.joy2y = value;
                        break;
                    case CONTEx.TRIGGER_L:
                        this._status.ltrig = (int)(axis * 255);
                        break;
                    case CONTEx.TRIGGER_R:
                        this._status.rtrig = (int)(axis * 255);
                        break;
                }
            }
        }

    }

    public class maple_keyboard_device_t {

        private class KeyInfo {
            public double timestamp;
            public CONT target_button;
            public CONTEx target_axis;
            public bool hold;
            public bool negative;
        }

        public cont_state_t status;
        public double timestamp;
        private List<KeyInfo> queue;
        public int queue_enabled_refs;
        internal readonly KeyCallback keyCallback;

        internal maple_keyboard_device_t() {
            status = new cont_state_t();
            timestamp = 0;
            queue = new List<KeyInfo>(16);
            queue_enabled_refs = 0;
            keyCallback = _keyboard_enqueue_data;
        }


        internal void _keyboard_enqueue_data(Window window, Keys key, int scancode, InputState state, ModKeys mods) {
            if (state == InputState.Repeat) return;

            Engine.Externals.LuaScriptInterop.LuascriptPlatform.InternalCallbackKeyboard(window, key, scancode, state, mods);

            // avoid collision with not binded keys
            if (scancode == -1) return;

            // special keys
            if (key == Keys.F11) {
                if (state == InputState.Press) Engine.Platform.PVRContext.ToggleFullscreen();
                return;
            }
            if (key == Keys.F12) {
                if (state == InputState.Press) Engine.Platform.PVRContext.TakeScreenshot();
                return;
            }
            if (key == Keys.KP_SUBTRACT) {
                if (state == InputState.Press) Engine.Game.MasterVolume.VolumeStep(false);
                return;
            }
            if (key == Keys.KP_ADD) {
                if (state == InputState.Press) Engine.Game.MasterVolume.VolumeStep(true);
                return;
            }

            bool hold = state == InputState.Press;
            double timestamp = Glfw.GetTime() * 1000.0;

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_BUTTONS.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_BUTTONS[i];
                if (mapping.scancode == scancode) {
                    enqueue_key_event(timestamp, mapping.button, CONTEx.NONE, false, hold);
                }
            }

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_AXES.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_AXES[i];

                if (mapping.scancode_low == scancode) {
                    enqueue_key_event(timestamp, CONT.NOTHING, mapping.axis, true, hold);
                }
                if (mapping.scancode_high == scancode) {
                    enqueue_key_event(timestamp, CONT.NOTHING, mapping.axis, false, hold);
                }
            }

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_TRIGGERS.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_TRIGGERS[i];
                if (mapping.scancode == scancode) {
                    enqueue_key_event(timestamp, CONT.NOTHING, mapping.trigger, false, hold);
                }
            }

            // reached if the key is not mapped
            return;
        }

        private void enqueue_key_event(double timestamp, CONT target_button, CONTEx target_axis, bool negative, bool hold) {
            if (this.queue.Count > 128) {
                // parse and update the current state
                this.parse_key_event(this.queue[0]);
                this.queue.RemoveAt(0);
            }

            // before continue check is the key state is the same but with different modifier key
            for (int i = this.queue.Count - 1 ; i >= 0 ; i--) {
                KeyInfo addedKeyInfo = this.queue[i];
                if (addedKeyInfo.target_button != target_button) continue;
                if (addedKeyInfo.hold == hold) return;

                // the hold state is different, so... add it
                break;
            }

            KeyInfo keyInfo = new KeyInfo() {
                timestamp = timestamp,
                target_button = target_button,
                target_axis = target_axis,
                hold = hold,
                negative = negative
            };


            if (this.queue_enabled_refs > 0)
                this.queue.Add(keyInfo);
            else
                parse_key_event(keyInfo);
        }

        internal cont_state_t dequeque_all() {
            if (this.queue.Count > 0) {
                for (int i = 0 ; i < this.queue.Count ; i++) {
                    this.parse_key_event(this.queue[i]);
                }
                this.queue.Clear();
            }
            return this.status;
        }

        private void parse_key_event(KeyInfo item) {
            this.timestamp = item.timestamp;

            if (item.target_button != CONT.NOTHING) {
                if (item.hold)
                    this.status.buttons |= item.target_button;
                else
                    this.status.buttons &= ~item.target_button;
                return;
            }

            switch (item.target_axis) {
                case CONTEx.TRIGGER_L:
                    this.status.ltrig = item.hold ? 255 : 0;
                    return;
                case CONTEx.TRIGGER_R:
                    this.status.rtrig = item.hold ? 255 : 0;
                    return;
            }

            int value = 0;
            if (item.hold) value = item.negative ? -127 : 127;

            switch (item.target_axis) {
                case CONTEx.AXIS_LX:
                    this.status.joyx = value;
                    break;
                case CONTEx.AXIS_LY:
                    this.status.joyy = value;
                    break;
                case CONTEx.AXIS_RX:
                    this.status.joy2x = value;
                    break;
                case CONTEx.AXIS_RY:
                    this.status.joy2x = value;
                    break;
            }
        }

        internal IEnumerable<bool> poll_queue() {
            for (int i = 0 ; i < this.queue.Count ; i++) {
                this.parse_key_event(this.queue[i]);
                yield return true;
            }
            this.queue.Clear();
        }

        internal bool has_queued {
            get => this.queue.Count > 0;
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
    public class cont_state_t {
        /**@type{uint} Buttons bitfield.*/
        public CONT buttons = 0;

        /**@type{byte} Left trigger value. */
        public int ltrig = 0;
        /**@type{byte} Right trigger value. */
        public int rtrig = 0;

        /**@type{byte} Main joystick x-axis value. */
        public int joyx = 0;
        /**@type{byte} Main joystick y-axis value. */
        public int joyy = 0;

        /**@type{byte} Secondary joystick x-axis value (if applicable). */
        public int joy2x = 0;
        /**@type{byte} Secondary joystick y-axis value (if applicable). */
        public int joy2y = 0;
    }

    internal static class maple_mappings {

        public static readonly GamePadButtonToCONT[] GAMEPAD_BUTTONS_MAPPING = {
            new GamePadButtonToCONT(GamepadButton.A, CONT.A | CONT.DPAD3_DOWN),
            new GamePadButtonToCONT(GamepadButton.B, CONT.B | CONT.DPAD3_RIGHT),
            new GamePadButtonToCONT(GamepadButton.X, CONT.X | CONT.DPAD3_LEFT),
            new GamePadButtonToCONT(GamepadButton.Y, CONT.Y | CONT.DPAD3_UP),
            new GamePadButtonToCONT(GamepadButton.LeftBumper, CONT.C),
            new GamePadButtonToCONT(GamepadButton.RightBumper, CONT.Z),
            new GamePadButtonToCONT(GamepadButton.Start, CONT.START),
            new GamePadButtonToCONT(GamepadButton.DpadUp, CONT.DPAD_UP ),
            new GamePadButtonToCONT(GamepadButton.DpadDown, CONT.DPAD_DOWN ),
            new GamePadButtonToCONT(GamepadButton.DpadLeft, CONT.DPAD_LEFT ),
            new GamePadButtonToCONT(GamepadButton.DpadRight, CONT.DPAD_RIGHT),
            new GamePadButtonToCONT(GamepadButton.Back, CONT.BACK_OR_SELECT ),
            new GamePadButtonToCONT(GamepadButton.Guide, CONT.HOME_OR_GUIDE )
        };
        public static readonly GamePadAxisToAxis[] GAMEPAD_AXES_MAPPING = {
            new GamePadAxisToAxis(GamepadAxis.LeftX, CONTEx.AXIS_LX),
            new GamePadAxisToAxis(GamepadAxis.LeftY, CONTEx.AXIS_LY),
            new GamePadAxisToAxis(GamepadAxis.RightX, CONTEx.AXIS_RX),
            new GamePadAxisToAxis(GamepadAxis.RightY, CONTEx.AXIS_RY),
            new GamePadAxisToAxis(GamepadAxis.LeftTrigger, CONTEx.TRIGGER_L),
            new GamePadAxisToAxis(GamepadAxis.RightTrigger, CONTEx.TRIGGER_R),
        };

        public static readonly KeyboardToCONT[] KEYBOARD_MAPPING_BUTTONS = {
            new KeyboardToCONT(Keys.ENTER, CONT.START),
            new KeyboardToCONT(Keys.A , CONT.A),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.B),
            new KeyboardToCONT(Keys.S, CONT.X),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.Y),
            new KeyboardToCONT(Keys.UP, CONT.DPAD_UP),
            new KeyboardToCONT(Keys.DOWN, CONT.DPAD_DOWN),
            new KeyboardToCONT(Keys.RIGHT, CONT.DPAD_RIGHT),
            new KeyboardToCONT(Keys.LEFT, CONT.DPAD_LEFT),
            new KeyboardToCONT(Keys.F, CONT.DPAD2_LEFT),
            new KeyboardToCONT(Keys.G, CONT.DPAD2_DOWN),
            new KeyboardToCONT(Keys.J, CONT.DPAD2_UP),
            new KeyboardToCONT(Keys.K, CONT.DPAD2_RIGHT),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD3_LEFT),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD3_DOWN),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD3_UP),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD3_RIGHT),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD4_LEFT),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD4_DOWN),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD4_UP),
            new KeyboardToCONT(Keys.UNKNOWN, CONT.DPAD4_RIGHT),
            new KeyboardToCONT(Keys.SPACE, CONT.D),
            new KeyboardToCONT(Keys.ESCAPE, CONT.BACK_OR_SELECT),
            new KeyboardToCONT(Keys.HOME, CONT.HOME_OR_GUIDE)
        };
        public static readonly KeyboardToAxis[] KEYBOARD_MAPPING_AXES = {
            new KeyboardToAxis(Keys.UNKNOWN, Keys.KP_8, CONTEx.AXIS_LY),
            new KeyboardToAxis(Keys.UNKNOWN, Keys.KP_6, CONTEx.AXIS_LX)
        };
        public static readonly KeyboardToTrigger[] KEYBOARD_MAPPING_TRIGGERS = {
            new KeyboardToTrigger(Keys.Q, CONTEx.TRIGGER_L),
            new KeyboardToTrigger(Keys.W, CONTEx.TRIGGER_R)
        };

        public static readonly GamePadToMaple[] GAMEPAD_TO_MAPLE_DEVICE = {
            new GamePadToMaple(0, 0U, 0U),
            new GamePadToMaple(1, 1U, 0U),
            new GamePadToMaple(2, 2U, 0U),
            new GamePadToMaple(3, 3U, 0U),
            new GamePadToMaple(4, 0U, 2U),
            new GamePadToMaple(5, 1U, 2U),
            new GamePadToMaple(6, 2U, 2U),
            new GamePadToMaple(7, 3U, 2U),
            new GamePadToMaple(8,  0U, 3U),
            new GamePadToMaple(9, 1U, 3U),
            new GamePadToMaple(10, 2U, 3U),
            new GamePadToMaple(11, 3U, 3U),
            new GamePadToMaple(12, 0U, 4U),
            new GamePadToMaple(13, 1U, 4U),
            new GamePadToMaple(14, 2U, 4U),
            new GamePadToMaple(15, 3U, 4U)
       };


        public static void LoadKeyboardMappings() {

            EngineSettings.GetBind("menuAccept", ref KEYBOARD_MAPPING_BUTTONS[1].scancode);
            EngineSettings.GetBind("menuAlternativeTracks", ref KEYBOARD_MAPPING_BUTTONS[3].scancode);

            EngineSettings.GetBind("menuSelectorLeft", ref KEYBOARD_MAPPING_TRIGGERS[0].scancode);
            EngineSettings.GetBind("menuSelectorRight", ref KEYBOARD_MAPPING_TRIGGERS[1].scancode);

            //EngineSettings.GetBind("left0", ref KEYBOARD_MAPPING_BUTTONS[5].scancode);
            //EngineSettings.GetBind("down0", ref KEYBOARD_MAPPING_BUTTONS[6].scancode);
            //EngineSettings.GetBind("up0", ref KEYBOARD_MAPPING_BUTTONS[7].scancode);
            //EngineSettings.GetBind("right0", ref KEYBOARD_MAPPING_BUTTONS[8].scancode);
            EngineSettings.GetBind("left1", ref KEYBOARD_MAPPING_BUTTONS[9].scancode);
            EngineSettings.GetBind("down1", ref KEYBOARD_MAPPING_BUTTONS[10].scancode);
            EngineSettings.GetBind("up1", ref KEYBOARD_MAPPING_BUTTONS[11].scancode);
            EngineSettings.GetBind("right1", ref KEYBOARD_MAPPING_BUTTONS[12].scancode);
            EngineSettings.GetBind("left2", ref KEYBOARD_MAPPING_BUTTONS[13].scancode);
            EngineSettings.GetBind("down2", ref KEYBOARD_MAPPING_BUTTONS[14].scancode);
            EngineSettings.GetBind("up2", ref KEYBOARD_MAPPING_BUTTONS[15].scancode);
            EngineSettings.GetBind("right2", ref KEYBOARD_MAPPING_BUTTONS[16].scancode);
            EngineSettings.GetBind("left3", ref KEYBOARD_MAPPING_BUTTONS[17].scancode);
            EngineSettings.GetBind("down3", ref KEYBOARD_MAPPING_BUTTONS[18].scancode);
            EngineSettings.GetBind("up3", ref KEYBOARD_MAPPING_BUTTONS[19].scancode);
            EngineSettings.GetBind("right3", ref KEYBOARD_MAPPING_BUTTONS[20].scancode);

            EngineSettings.GetBind("diamond", ref KEYBOARD_MAPPING_BUTTONS[21].scancode);
        }


        internal struct KeyboardToCONT {
            public int scancode;
            public readonly CONT button;

            public KeyboardToCONT(Keys key, CONT button) {
                this.scancode = key == Keys.UNKNOWN ? -1 : Glfw.GetKeyScanCode(key);
                this.button = button;
            }

            public override string ToString() {
                string name = Glfw.GetKeyName(Keys.UNKNOWN, scancode);
                return $"button={button} scancode={scancode} key_name={name}";
            }
        }

        internal struct KeyboardToAxis {
            public readonly CONTEx axis;
            public int scancode_low;
            public int scancode_high;

            public KeyboardToAxis(Keys key_low, Keys key_high, CONTEx axis) {
                this.axis = axis;
                this.scancode_low = Glfw.GetKeyScanCode(key_low);
                this.scancode_high = Glfw.GetKeyScanCode(key_high);
            }

            public override string ToString() {
                return $"axis={axis} scancode_low={scancode_low} scancode_high={scancode_high}";
            }
        }

        internal struct KeyboardToTrigger {
            public readonly CONTEx trigger;
            public int scancode;

            public KeyboardToTrigger(Keys key, CONTEx trigger) {
                this.trigger = trigger;
                this.scancode = Glfw.GetKeyScanCode(key);
            }

            public override string ToString() {
                string name = Glfw.GetKeyName(Keys.UNKNOWN, scancode);
                return $"trigger={trigger} scancode={scancode} key_name={name}";
            }
        }

        internal struct GamePadButtonToCONT {
            public GamepadButton glfw_button;
            public readonly CONT cont_button;

            public GamePadButtonToCONT(GamepadButton glfw_button, CONT cont_button) {
                this.glfw_button = glfw_button;
                this.cont_button = cont_button;
            }

            public override string ToString() {
                return $"glfw_button={glfw_button} cont_button={cont_button}";
            }
        }

        internal struct GamePadAxisToAxis {
            public readonly GamepadAxis glfw_axis;
            public readonly CONTEx axis;

            public GamePadAxisToAxis(GamepadAxis glfw_axis, CONTEx cont_axis) {
                this.glfw_axis = glfw_axis;
                this.axis = cont_axis;
            }

            public override string ToString() {
                return $"glfw_axis={glfw_axis} axis={axis}";
            }

        }

        internal struct GamePadToMaple {
            public readonly int index;
            public readonly uint port;
            public readonly uint unit;

            public GamePadToMaple(int index, uint port, uint unit) {
                this.index = index;
                this.port = port;
                this.unit = unit;
            }

            public override string ToString() {
                return $"index={index} port={port} unit={unit}";
            }
        }

    }

}
