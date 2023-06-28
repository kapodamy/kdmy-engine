using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace Engine.Externals.SoundBridge.Interop;

internal static class WASAPI {
    public static readonly Guid IAudioClient_GUID = new Guid("726778cd-f60a-4eda-82de-e47610cd78aa");
    public static readonly string DEVINTERFACE_AUDIO_RENDER = "{e6327cad-dcec-4949-ae8a-991e976a79d2}";


    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("72A22D78-CDE4-431D-B8CC-843A71199B6D")]
    public interface IActivateAudioInterfaceAsyncOperation {
        void GetActivateResult(out int activateResult, [MarshalAs(UnmanagedType.IUnknown)] out object activateInterface);
    }

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("41D949AB-9862-444A-80F6-C261334DA5EB")]
    public interface IActivateAudioInterfaceCompletionHandler {
        void ActivateCompleted(IActivateAudioInterfaceAsyncOperation activateOperation);
    }

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("94ea2b94-e9cc-49e0-c0ff-ee64ca8f5b90")]
    public interface IAgileObject {

    }

    private class CompletionHandler : IActivateAudioInterfaceCompletionHandler, IAgileObject {
        public void ActivateCompleted(IActivateAudioInterfaceAsyncOperation activateOperation) { }
    }

    [DllImport("Mmdevapi.dll", ExactSpelling = true, PreserveSig = false)]
    public static extern void ActivateAudioInterfaceAsync([In, MarshalAs(UnmanagedType.LPWStr)] string deviceInterfacePath, [In, MarshalAs(UnmanagedType.LPStruct)] Guid riid, nint activationParams, [In] IActivateAudioInterfaceCompletionHandler completionHandler, out IActivateAudioInterfaceAsyncOperation activationOperation);


    public static void TryEnableAutomaticStreamRouting() {
        if (Environment.OSVersion.Platform != PlatformID.Win32NT) return;
        if (Environment.OSVersion.Version.Major < 6) return;
        if (Environment.OSVersion.Version.Major < 7 && Environment.OSVersion.Version.Minor < 2) return;
        if (Thread.CurrentThread.GetApartmentState() != ApartmentState.STA) return;

        CompletionHandler handler = new CompletionHandler();

        IActivateAudioInterfaceAsyncOperation operation;
        int hresult;
        object unknown;

        ActivateAudioInterfaceAsync(
            DEVINTERFACE_AUDIO_RENDER, IAudioClient_GUID, 0x00, handler, out operation
        );

        operation.GetActivateResult(out hresult, out unknown);

        if (hresult != 0) {
            Exception e = Marshal.GetExceptionForHR(hresult, -1);
            Logger.Warn($"WASAPI::ActivateAudioInterfaceAsync() failed: {e.Message}");
        }
    }
}



