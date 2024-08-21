using System;
using System.Collections.Generic;
using CsharpWrapper;
using Engine;
using Engine.Externals.GLFW;
using Engine.Externals.LuaScriptInterop;

#pragma warning disable CS8981

namespace KallistiOS.MAPLE;

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
internal enum CONT_EX {
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

    public const int MAPLE_PORT_COUNT = 4;
    public const int MAPLE_UNIT_COUNT = 6;
    private static JoystickCallback joystickCallback;

    private static maple_device_t[,] DEVICES;
    public static maple_device_t STUB_KEYBOARD;

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
        if (dev is maple.maple_device_GLFWGamepad_t obj) obj.Poll();
        return dev.status;
    }


    public static maple_device_t enum_dev(int p, int u) {
        foreach (maple_device_t device in DEVICES) {
            if (device != null && device.port == p && device.unit == u) {
                return device.valid ? device : null;
            }
        }
        return null;
    }


    internal static void __initialize(Window nativeWindow) {
        maple.joystickCallback = InternalGamepadConnection;
        maple.DEVICES = new maple_device_t[MAPLE_PORT_COUNT, MAPLE_UNIT_COUNT];

        // allocate VMUs
        int saveslots = EngineSettings.saveslots;
        for (int i = 0 ; i < saveslots ; i++) {
            if (i < 4) {
                maple.DEVICES[i, 1] = new maple_device_t(MAPLE_FUNC.MEMCARD, i, 1);
                continue;
            }

            for (int port = 0 ; port < MAPLE_PORT_COUNT ; port++) {
                for (int unit = 0 ; unit < MAPLE_UNIT_COUNT ; unit++) {
                    foreach (var mapping in maple_mappings.GAMEPAD_TO_MAPLE_DEVICE) {
                        if (mapping.port != port && mapping.unit != unit) {
                            // unused space, assign as VMU
                            maple.DEVICES[port, unit] = new maple_device_t(MAPLE_FUNC.MEMCARD, port, unit);
                            goto L_continue;
                        }
                    }
                }
            }

L_continue:
            continue;
        }

        // STUB keyboard
        maple_device_GLFWKeyboard_t kb_dvr = new maple_device_GLFWKeyboard_t();
        Glfw.SetKeyCallback(nativeWindow, kb_dvr.keyCallback);
        maple.STUB_KEYBOARD = (maple_device_t)kb_dvr;

        // gamepad connection/disconnection
        Glfw.SetJoystickCallback(maple.joystickCallback);

        // allocate existing gamepads
        for (int i = 0, total = Glfw.JOYSTICK_LAST ; i < total ; i++) {
            if (Glfw.JoystickIsGamepad(i)) InternalGamepadConnection(i, ConnectionStatus.Connected);
        }
    }

    private static void InternalGamepadConnection(int index, ConnectionStatus status) {
        if (index <= 0 || index >= maple_mappings.GAMEPAD_TO_MAPLE_DEVICE.Length) {
            Logger.Error($"no gamepad slot. id={index} name={Glfw.GetJoystickName(index)} status={status}");
            return;
        }

        maple_device_GLFWGamepad_t dev = null;

        var mapping = maple_mappings.GAMEPAD_TO_MAPLE_DEVICE[index];
        if (DEVICES[mapping.port, mapping.unit] == null) {
            DEVICES[mapping.port, mapping.unit] = dev = new maple_device_GLFWGamepad_t(index);
        } else {
            var tmp = DEVICES[mapping.port, mapping.unit];
            if (tmp is maple_device_GLFWGamepad_t) {
                dev = (maple_device_GLFWGamepad_t)tmp;
            }
        }

        dev.valid = status == ConnectionStatus.Connected;
        dev.status.buttons = CONT.NOTHING;
        dev.status.joyx = dev.status.joyy = 0;
        dev.status.joy2x = dev.status.joy2y = 0;
        dev.status.ltrig = dev.status.rtrig = 0;

        Logger.Log($"gamepad id={index} name={Glfw.GetJoystickName(index)} status={status}");
    }

    internal static void PollAllGamepads() {
        for (int i = 0 ; i < maple_mappings.GAMEPAD_TO_MAPLE_DEVICE.Length ; i++) {
            var mapping = maple_mappings.GAMEPAD_TO_MAPLE_DEVICE[i];
            maple_device_GLFWGamepad_t device = (maple_device_GLFWGamepad_t)DEVICES[mapping.port, mapping.unit];
            if (device != null && device.valid) device.Poll();
        }
    }

    internal static void LoadKeyboardMappings() {

        EngineSettings.GetBind("menuAccept", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[1].scancode);
        EngineSettings.GetBind("menuAlternativeTracks", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[3].scancode);

        EngineSettings.GetBind("menuSelectorLeft", ref maple_mappings.KEYBOARD_MAPPING_TRIGGERS[0].scancode);
        EngineSettings.GetBind("menuSelectorRight", ref maple_mappings.KEYBOARD_MAPPING_TRIGGERS[1].scancode);

        //EngineSettings.GetBind("left0", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[5].scancode);
        //EngineSettings.GetBind("down0", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[6].scancode);
        //EngineSettings.GetBind("up0", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[7].scancode);
        //EngineSettings.GetBind("right0", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[8].scancode);
        EngineSettings.GetBind("left1", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[9].scancode);
        EngineSettings.GetBind("down1", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[10].scancode);
        EngineSettings.GetBind("up1", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[11].scancode);
        EngineSettings.GetBind("right1", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[12].scancode);
        EngineSettings.GetBind("left2", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[13].scancode);
        EngineSettings.GetBind("down2", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[14].scancode);
        EngineSettings.GetBind("up2", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[15].scancode);
        EngineSettings.GetBind("right2", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[16].scancode);
        EngineSettings.GetBind("left3", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[17].scancode);
        EngineSettings.GetBind("down3", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[18].scancode);
        EngineSettings.GetBind("up3", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[19].scancode);
        EngineSettings.GetBind("right3", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[20].scancode);

        EngineSettings.GetBind("diamond", ref maple_mappings.KEYBOARD_MAPPING_BUTTONS[21].scancode);
    }


    private class maple_device_GLFWGamepad_t : maple_device_t {

        private int gamepad_index;

        public maple_device_GLFWGamepad_t(int gamepad_index) :
            base(
                MAPLE_FUNC.CONTROLLER,
                maple_mappings.GAMEPAD_TO_MAPLE_DEVICE[gamepad_index].port,
                maple_mappings.GAMEPAD_TO_MAPLE_DEVICE[gamepad_index].unit
            ) {
            this.gamepad_index = gamepad_index;
        }


        public void Poll() {
            //
            // Notes:
            //    * The dreamcast controller does not have enough buttons/axes like modern controllers.
            //    * If the controller has additional buttons and/or axes, they are reported
            //      by KallistiOS using CONT_CAPABILITY_* in maple_enum_type*() functions.
            //
            if (this.gamepad_index < 0) return;

            GamePadState state;
            if (!Glfw.GetGamepadState(this.gamepad_index, out state)) {
                this.valid = false;
                return;
            }

            // read the gamepad buttons using the standard layout
            for (int i = 0 ; i < maple_mappings.GAMEPAD_BUTTONS_MAPPING.Length ; i++) {
                var mapping = maple_mappings.GAMEPAD_BUTTONS_MAPPING[i];
                byte button_state = state.buttons[(int)mapping.glfw_button];

                if (button_state == 0x01)
                    this.status.buttons |= mapping.cont_button;
                else if (button_state == 0x00)
                    this.status.buttons &= ~mapping.cont_button;
            }

            // read the gamepad axes using the standard layout
            for (int i = 0 ; i < maple_mappings.GAMEPAD_AXES_MAPPING.Length ; i++) {
                var mapping = maple_mappings.GAMEPAD_AXES_MAPPING[i];
                float axis = state.axes[(int)mapping.glfw_axis];
                int value = (int)(axis * 0x7F);

                switch (mapping.axis) {
                    case CONT_EX.AXIS_LX:
                        this.status.joyx = value;
                        break;
                    case CONT_EX.AXIS_LY:
                        this.status.joyy = value;
                        break;
                    case CONT_EX.AXIS_RX:
                        this.status.joy2x = value;
                        break;
                    case CONT_EX.AXIS_RY:
                        this.status.joy2y = value;
                        break;
                    case CONT_EX.TRIGGER_L:
                        this.status.ltrig = (int)(axis * 255);
                        break;
                    case CONT_EX.TRIGGER_R:
                        this.status.rtrig = (int)(axis * 255);
                        break;
                }
            }
        }
    }

    internal class maple_device_GLFWKeyboard_t : maple_device_t {

        internal readonly KeyCallback keyCallback;

        public maple_device_GLFWKeyboard_t() : base(MAPLE_FUNC.CONTROLLER, -1, -1) {
            keyCallback = this.HandleKey;
        }


        private void HandleKey(Window window, Keys key, int scancode, InputState state, ModKeys mods) {
            if (state == InputState.Repeat) return;

            LuascriptPlatform.InternalCallbackKeyboard(window, key, scancode, state, mods);

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

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_BUTTONS.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_BUTTONS[i];
                if (mapping.scancode == scancode) {
                    ProcessKey(mapping.button, CONT_EX.NONE, false, hold);
                }
            }

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_AXES.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_AXES[i];

                if (mapping.scancode_low == scancode) {
                    ProcessKey(CONT.NOTHING, mapping.axis, true, hold);
                }
                if (mapping.scancode_high == scancode) {
                    ProcessKey(CONT.NOTHING, mapping.axis, false, hold);
                }
            }

            for (int i = 0 ; i < maple_mappings.KEYBOARD_MAPPING_TRIGGERS.Length ; i++) {
                var mapping = maple_mappings.KEYBOARD_MAPPING_TRIGGERS[i];
                if (mapping.scancode == scancode) {
                    ProcessKey(CONT.NOTHING, mapping.trigger, false, hold);
                }
            }

            // reached if the key is not mapped
            return;
        }

        private void ProcessKey(CONT target_button, CONT_EX target_axis, bool negative, bool hold) {

            if (target_button != CONT.NOTHING) {
                if (hold)
                    this.status.buttons |= target_button;
                else
                    this.status.buttons &= ~target_button;
                return;
            }

            switch (target_axis) {
                case CONT_EX.TRIGGER_L:
                    this.status.ltrig = hold ? 255 : 0;
                    return;
                case CONT_EX.TRIGGER_R:
                    this.status.rtrig = hold ? 255 : 0;
                    return;
            }

            int value = 0;
            if (hold) value = negative ? -127 : 127;

            switch (target_axis) {
                case CONT_EX.AXIS_LX:
                    this.status.joyx = value;
                    break;
                case CONT_EX.AXIS_LY:
                    this.status.joyy = value;
                    break;
                case CONT_EX.AXIS_RX:
                    this.status.joy2x = value;
                    break;
                case CONT_EX.AXIS_RY:
                    this.status.joy2x = value;
                    break;
            }
        }

    }

}

/** 
 * @summary  Maple device info structure.
    This structure is used by the hardware to deliver the response to the device
    info request.
*/
public struct maple_devinfo_t {
    /** @type {number} Function codes supported */
    public MAPLE_FUNC functions;
}

/**
 * One maple device.
 *   Note that we duplicate the port/unit info which is normally somewhat
 *   implicit so that we can pass around a pointer to a particular device struct.
 */
public class maple_device_t {

    public bool valid;
    public int port;
    public int unit;
    public maple_devinfo_t info;
    public cont_state_t status;


    internal maple_device_t(MAPLE_FUNC funcs, int port, int unit) {
        this.port = port;
        this.unit = unit;
        this.info.functions = funcs;
        this.valid = (funcs & MAPLE_FUNC.MEMCARD) != MAPLE_FUNC.NOTHING;
        this.status = new cont_state_t();
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
    public CONT buttons = 0;

    public int ltrig = 0;
    public int rtrig = 0;

    public int joyx = 0;
    public int joyy = 0;

    public int joy2x = 0;
    public int joy2y = 0;
}

internal static class maple_mappings {

    public static readonly GamePadButtonToCONT[] GAMEPAD_BUTTONS_MAPPING = {
        new (GamepadButton.A, CONT.A | CONT.DPAD3_DOWN),
        new (GamepadButton.B, CONT.B | CONT.DPAD3_RIGHT),
        new (GamepadButton.X, CONT.X | CONT.DPAD3_LEFT),
        new (GamepadButton.Y, CONT.Y | CONT.DPAD3_UP),
        new (GamepadButton.LeftBumper, CONT.C),
        new (GamepadButton.RightBumper, CONT.Z),
        new (GamepadButton.Start, CONT.START),
        new (GamepadButton.DpadUp, CONT.DPAD_UP ),
        new (GamepadButton.DpadDown, CONT.DPAD_DOWN ),
        new (GamepadButton.DpadLeft, CONT.DPAD_LEFT ),
        new (GamepadButton.DpadRight, CONT.DPAD_RIGHT),
        new (GamepadButton.Back, CONT.BACK_OR_SELECT ),
        new (GamepadButton.Guide, CONT.HOME_OR_GUIDE )
    };
    public static readonly GamePadAxisToAxis[] GAMEPAD_AXES_MAPPING = {
        new (GamepadAxis.LeftX, CONT_EX.AXIS_LX),
        new (GamepadAxis.LeftY, CONT_EX.AXIS_LY),
        new (GamepadAxis.RightX, CONT_EX.AXIS_RX),
        new (GamepadAxis.RightY, CONT_EX.AXIS_RY),
        new (GamepadAxis.LeftTrigger, CONT_EX.TRIGGER_L),
        new (GamepadAxis.RightTrigger, CONT_EX.TRIGGER_R),
    };

    public static readonly KeyboardToCONT[] KEYBOARD_MAPPING_BUTTONS = {
        new (Keys.ENTER, CONT.START),
        new (Keys.A , CONT.A),
        new (Keys.UNKNOWN, CONT.B),
        new (Keys.X, CONT.X),
        new (Keys.UNKNOWN, CONT.Y),
        new (Keys.UP, CONT.DPAD_UP),
        new (Keys.DOWN, CONT.DPAD_DOWN),
        new (Keys.RIGHT, CONT.DPAD_RIGHT),
        new (Keys.LEFT, CONT.DPAD_LEFT),
        new (Keys.F, CONT.DPAD2_LEFT),
        new (Keys.G, CONT.DPAD2_DOWN),
        new (Keys.J, CONT.DPAD2_UP),
        new (Keys.K, CONT.DPAD2_RIGHT),
        new (Keys.UNKNOWN, CONT.DPAD3_LEFT),
        new (Keys.UNKNOWN, CONT.DPAD3_DOWN),
        new (Keys.UNKNOWN, CONT.DPAD3_UP),
        new (Keys.UNKNOWN, CONT.DPAD3_RIGHT),
        new (Keys.UNKNOWN, CONT.DPAD4_LEFT),
        new (Keys.UNKNOWN, CONT.DPAD4_DOWN),
        new (Keys.UNKNOWN, CONT.DPAD4_UP),
        new (Keys.UNKNOWN, CONT.DPAD4_RIGHT),
        new (Keys.SPACE, CONT.D),
        new (Keys.ESCAPE, CONT.BACK_OR_SELECT),
        new (Keys.HOME, CONT.HOME_OR_GUIDE)
    };
    public static readonly KeyboardToAxis[] KEYBOARD_MAPPING_AXES = {
        new (Keys.UNKNOWN, Keys.UNKNOWN, CONT_EX.AXIS_LY),
        new (Keys.UNKNOWN, Keys.UNKNOWN, CONT_EX.AXIS_LX)
    };
    public static readonly KeyboardToTrigger[] KEYBOARD_MAPPING_TRIGGERS = {
        new (Keys.NUMPAD1, CONT_EX.TRIGGER_L),
        new (Keys.NUMPAD2, CONT_EX.TRIGGER_R)
    };

    public static readonly GamePadToMaple[] GAMEPAD_TO_MAPLE_DEVICE = {
        new (0, 0),
        new (1, 0),
        new (2, 0),
        new (3, 0),
        new (0, 2),
        new (1, 2),
        new (2, 2),
        new (3, 2),
        new (0, 3),
        new (1, 3),
        new (2, 3),
        new (3, 3),
        new (0, 4),
        new (1, 4),
        new (2, 4),
        new (3, 4)
   };


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
        public readonly CONT_EX axis;
        public int scancode_low;
        public int scancode_high;

        public KeyboardToAxis(Keys key_low, Keys key_high, CONT_EX axis) {
            this.axis = axis;
            this.scancode_low = Glfw.GetKeyScanCode(key_low);
            this.scancode_high = Glfw.GetKeyScanCode(key_high);
        }

        public override string ToString() {
            return $"axis={axis} scancode_low={scancode_low} scancode_high={scancode_high}";
        }
    }

    internal struct KeyboardToTrigger {
        public readonly CONT_EX trigger;
        public int scancode;

        public KeyboardToTrigger(Keys key, CONT_EX trigger) {
            this.trigger = trigger;
            this.scancode = Glfw.GetKeyScanCode(key);
        }

        public override string ToString() {
            string name = Glfw.GetKeyName(Keys.UNKNOWN, scancode);
            return $"trigger={trigger} scancode={scancode} key_name={name}";
        }
    }

    internal readonly struct GamePadButtonToCONT {
        public readonly GamepadButton glfw_button;
        public readonly CONT cont_button;

        public GamePadButtonToCONT(GamepadButton glfw_button, CONT cont_button) {
            this.glfw_button = glfw_button;
            this.cont_button = cont_button;
        }

        public override string ToString() {
            return $"glfw_button={glfw_button} cont_button={cont_button}";
        }
    }

    internal readonly struct GamePadAxisToAxis {
        public readonly GamepadAxis glfw_axis;
        public readonly CONT_EX axis;

        public GamePadAxisToAxis(GamepadAxis glfw_axis, CONT_EX cont_axis) {
            this.glfw_axis = glfw_axis;
            this.axis = cont_axis;
        }

        public override string ToString() {
            return $"glfw_axis={glfw_axis} axis={axis}";
        }

    }

    internal readonly struct GamePadToMaple {
        public readonly int port;
        public readonly int unit;

        public GamePadToMaple(int port, int unit) {
            this.port = port;
            this.unit = unit;
        }
    }

}
