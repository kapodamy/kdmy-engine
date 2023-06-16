using System.Runtime.InteropServices;

namespace Engine.Externals;

internal static class AICA {
    private const string DLL = "kdy_AICA";

    public const int StreamID_INVALID = -1;
    public const int StreamID_DECODER_FAILED = -2;
    public const int StreamID_BACKEND_FAILED = -3;

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern nint filehandle_init(/* const char* */byte[] fullpath);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern nint filehandle_init2(/* const byte* */nint data, int size);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern nint filehandle_init3(/* const char* */byte[] fullpath);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void filehandle_destroy(nint handle);


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int sndbridge_queue_ogg(nint ogg_filehandle);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int sndbridge_queue(nint external_decoder);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_dispose(int stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern double sndbridge_duration(int stream);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern double sndbridge_position(int stream);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_seek(int stream, double milliseconds);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_play(int stream);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_pause(int stream);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_stop(int stream);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_set_volume(int stream, float volume);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void sndbridge_mute(int stream, int muted);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void sndbridge_do_fade(int stream, int fade_in_or_out, float milliseconds);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool sndbridge_is_active(int stream_id);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int sndbridge_has_fade_active(int stream_id);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool sndbridge_has_ended(int stream_id);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void sndbridge_loop(int stream_id, int enable);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sndbridge_set_master_volume(float volume);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void sndbridge_set_master_muted(int muted);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, EntryPoint = "sndbridge_get_runtime_info")]
    private static extern nint __sndbridge_get_runtime_info();

    public static string sndbridge_get_runtime_info() {
        nint ptr = __sndbridge_get_runtime_info();
        return Marshal.PtrToStringAnsi(ptr);
    }

    public static void sndbridge_mute(int stream, bool muted) {
        sndbridge_mute(stream, muted ? 1 : 0);
    }

    public static void sndbridge_do_fade(int stream, bool fade_in_or_out, float milliseconds) {
        sndbridge_do_fade(stream, fade_in_or_out ? 1 : 0, milliseconds);
    }

    public static void sndbridge_loop(int stream_id, bool enable) {
        sndbridge_loop(stream_id, enable ? 1 : 0);
    }

    public static void sndbridge_set_master_muted(bool muted) {
        sndbridge_set_master_muted(muted ? 1 : 0);
    }



}
