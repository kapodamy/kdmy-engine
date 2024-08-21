#pragma warning disable CS8981

namespace KallistiOS.VBLANK;


public delegate void asic_evt_handler(uint code);

public static class vblank {

    public const uint ASIC_EVT_PVR_VBLANK_BEGIN = 0x0003;


    private static asic_evt_handler[] KOS_VBLANK_HANDLERS = new asic_evt_handler[16];


    public static int handler_add(asic_evt_handler hnd) {
        for (int i = 0 ; i < KOS_VBLANK_HANDLERS.Length ; i++) {
            if (KOS_VBLANK_HANDLERS[i] == null) {
                KOS_VBLANK_HANDLERS[i] = hnd;
                return i;
            }
        }
        return -1;
    }

    public static int handler_remove(int handle) {
        if (handle >= 0 && handle < KOS_VBLANK_HANDLERS.Length) {
            if (KOS_VBLANK_HANDLERS[handle] != null) {
                KOS_VBLANK_HANDLERS[handle] = null;
                return 0;
            }
        }
        return -1;
    }


    public static void handlers_poll() {
        for (int i = 0 ; i < KOS_VBLANK_HANDLERS.Length ; i++) {
            if (KOS_VBLANK_HANDLERS[i] != null) {
                KOS_VBLANK_HANDLERS[i](ASIC_EVT_PVR_VBLANK_BEGIN);
            }
        }
    }

}
