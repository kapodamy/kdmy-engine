using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge.Interop;

using PaDeviceIndex = Int32;
using PaHostApiIndex = Int32;
using PaTime = Double;


internal enum PaError : int {
    paNoError = 0,

    paNotInitialized = -10000,
    paUnanticipatedHostError,
    paInvalidChannelCount,
    paInvalidSampleRate,
    paInvalidDevice,
    paInvalidFlag,
    paSampleFormatNotSupported,
    paBadIODeviceCombination,
    paInsufficientMemory,
    paBufferTooBig,
    paBufferTooSmall,
    paNullCallback,
    paBadStreamPtr,
    paTimedOut,
    paInternalError,
    paDeviceUnavailable,
    paIncompatibleHostApiSpecificStreamInfo,
    paStreamIsStopped,
    paStreamIsNotStopped,
    paInputOverflowed,
    paOutputUnderflowed,
    paHostApiNotFound,
    paInvalidHostApi,
    paCanNotReadFromACallbackStream,
    paCanNotWriteToACallbackStream,
    paCanNotReadFromAnOutputOnlyStream,
    paCanNotWriteToAnInputOnlyStream,
    paIncompatibleStreamHostApi,
    paBadBufferPtr
}

internal enum PaHostApiTypeId : uint {
    paInDevelopment = 0,
    paDirectSound = 1,
    paMME = 2,
    paASIO = 3,
    paSoundManager = 4,
    paCoreAudio = 5,
    paOSS = 7,
    paALSA = 8,
    paAL = 9,
    paBeOS = 10,
    paWDMKS = 11,
    paJACK = 12,
    paWASAPI = 13,
    paAudioScienceHPI = 14
}

internal enum PaSampleFormat : uint {
    paFloat32 = 0x00000001,
    paInt32 = 0x00000002,
    paInt24 = 0x00000004,
    paInt16 = 0x00000008,
    paInt8 = 0x00000010,
    paUInt8 = 0x00000020,
    paCustomFormat = 0x00010000,

    paNonInterleaved = 0x80000000
}

internal enum PaStreamCallbackFlags : uint {
    paInputUnderflow = 0x00000001,
    paInputOverflow = 0x00000002,
    paOutputUnderflow = 0x00000004,
    paOutputOverflow = 0x00000008,
    paPrimingOutput = 0x00000010
}

internal enum PaStreamCallbackResult : uint {
    paContinue = 0,
    paComplete = 1,
    paAbort = 2
}

[Flags]
internal enum PaStreamFlags : uint {
    paNoFlag = 0,
    paClipOff = 0x00000001,
    paDitherOff = 0x00000002,
    paNeverDropInput = 0x00000004,
    paPrimeOutputBuffersUsingStreamCallback = 0x00000008,
    paPlatformSpecificFlags = 0xFFFF0000,
}


//[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//internal delegate void PaStreamFinishedCallback(nint userData);

//[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//internal unsafe delegate int PaStreamCallback(float* input, float* output, uint frameCount, PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, nint userData);


//[StructLayout(LayoutKind.Sequential)]
//internal readonly struct PaStream { }

[StructLayout(LayoutKind.Sequential)]
internal readonly unsafe struct PaDeviceInfo {
    public readonly int structVersion;
    public readonly char* name;
    public readonly PaHostApiIndex hostApi;

    public readonly int maxInputChannels;
    public readonly int maxOutputChannels;

    public readonly PaTime defaultLowInputLatency;
    public readonly PaTime defaultLowOutputLatency;

    public readonly PaTime defaultHighInputLatency;
    public readonly PaTime defaultHighOutputLatency;

    public readonly double defaultSampleRate;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly unsafe struct PaHostApiInfo {
    public readonly int structVersion;
    public readonly PaHostApiTypeId type;
    public readonly char* name;

    public readonly int deviceCount;

    public readonly PaDeviceIndex defaultInputDevice;

    public readonly PaDeviceIndex defaultOutputDevice;

}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct PaStreamParameters {
    public PaDeviceIndex device;

    public int channelCount;

    public PaSampleFormat sampleFormat;

    public PaTime suggestedLatency;

    public void* hostApiSpecificStreamInfo;

}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct PaStreamCallbackTimeInfo {
    public readonly PaTime inputBufferAdcTime;
    public readonly PaTime currentTime;
    public readonly PaTime outputBufferDacTime;
}

internal unsafe static partial class PortAudio {
    private const string DLL = "libportaudio";

    public const int paNoDevice = -1;
    public const int paUseHostApiSpecificDeviceSpecification = -2;
    public const int paFramesPerBufferUnspecified = 0;


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaTime Pa_GetStreamTime(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Pa_Sleep(int msec);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_IsFormatSupported(PaStreamParameters* inputParameters, PaStreamParameters* outputParameters, double sampleRate);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_OpenStream(nint* stream, PaStreamParameters* inputParameters, PaStreamParameters* outputParameters, double sampleRate, uint framesPerBuffer, PaStreamFlags streamFlags, delegate* unmanaged[Cdecl]<float*, float*, uint, PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, nint, PaStreamCallbackResult> streamCallback, nint userData);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_SetStreamFinishedCallback(nint stream, delegate* unmanaged[Cdecl]<nint, void> streamFinishedCallback);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_Initialize();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_Terminate();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_CloseStream(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Pa_IsStreamStopped(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Pa_IsStreamActive(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_AbortStream(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaError Pa_StartStream(nint stream);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaHostApiIndex Pa_GetDefaultHostApi();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaHostApiIndex Pa_GetHostApiCount();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaHostApiInfo* Pa_GetHostApiInfo(PaHostApiIndex hostApi);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaDeviceInfo* Pa_GetDeviceInfo(PaDeviceIndex device);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern PaDeviceIndex Pa_GetDefaultOutputDevice();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern nint Pa_GetVersionText();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern nint Pa_GetErrorText(PaError errorCode);

}
