using System;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge.Interop;

using PaWasapiHostProcessorCallback = nint;
using PaWinWaveFormatChannelMask = UInt32;


internal enum PaWasapiThreadPriority : uint {
    eThreadPriorityNone = 0,
    eThreadPriorityAudio,
    eThreadPriorityCapture,
    eThreadPriorityDistribution,
    eThreadPriorityGames,
    eThreadPriorityPlayback,
    eThreadPriorityProAudio,
    eThreadPriorityWindowManager
}
internal enum PaWasapiStreamCategory : uint {
    eAudioCategoryOther = 0,
    eAudioCategoryCommunications = 3,
    eAudioCategoryAlerts = 4,
    eAudioCategorySoundEffects = 5,
    eAudioCategoryGameEffects = 6,
    eAudioCategoryGameMedia = 7,
    eAudioCategoryGameChat = 8,
    eAudioCategorySpeech = 9,
    eAudioCategoryMovie = 10,
    eAudioCategoryMedia = 11
}

internal enum PaWasapiStreamOption : uint {
    eStreamOptionNone = 0,
    eStreamOptionRaw = 1,
    eStreamOptionMatchFormat = 2
}

[Flags]
internal enum PaWasapiFlags : uint {
    paWinWasapiExclusive = (1 << 0),

    paWinWasapiRedirectHostProcessor = (1 << 1),

    paWinWasapiUseChannelMask = (1 << 2),

    paWinWasapiPolling = (1 << 3),

    paWinWasapiThreadPriority = (1 << 4),

    paWinWasapiExplicitSampleFormat = (1 << 5),

    paWinWasapiAutoConvert = (1 << 6)
}


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct PaWasapiStreamInfo {
    public uint size;
    public PaHostApiTypeId hostApiType;
    public uint version;

    public PaWasapiFlags flags;

    public PaWinWaveFormatChannelMask channelMask;

    public PaWasapiHostProcessorCallback hostProcessorOutput;
    public PaWasapiHostProcessorCallback hostProcessorInput;

    public PaWasapiThreadPriority threadPriority;

    public PaWasapiStreamCategory streamCategory;

    public PaWasapiStreamOption streamOption;
}
