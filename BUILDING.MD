# Building

The engine is available for:

* Dreamcast, written in C (requires assets conversion). See [BUILDING-DREAMCAST.MD](BUILDING-DREAMCAST.MD "BUILDING-DREAMCAST.MD") for instructions.
* Web browsers, written in javascript. See [BUILDING-JAVASCRIPT.MD](BUILDING-JAVASCRIPT.MD "BUILDING-JAVASCRIPT.MD") for instructions.
* Desktop, written in C# (runs on Windows 7 or later). See [BUILDING-DESKTOP.MD](BUILDING-DESKTOP.MD "BUILDING-DESKTOP.MD") for instructions.

## Development environment

* [VSCode](https://code.visualstudio.com/ "VSCode"), optional.
* [Visual Studio](https://visualstudio.microsoft.com/es/ "Visual Studio")
* [MSYS2](https://www.msys2.org/ "MSYS2") with [mingw-w64 toolchain](https://packages.msys2.org/group/mingw-w64-i686-toolchain "mingw-w64-x86_64-toolchain"), optional. Required only for building components.
* [emscripten](https://emscripten.org/ "emscripten"), optional.
* [DreamSDK](https://www.dreamsdk.org/ "DreamSDK") for dreamcast.
* [KallistiOS](https://github.com/KallistiOS/KallistiOS "KallistiOS") bundled with DreamSDSK.

---

## Components

### SoundBridge

* *Sound bridge*
* **C# & C only**
* Responsible for playing the ogg files (vorbis and opus) as well as communicating with the audio rendering API (preferably [DSound](https://en.wikipedia.org/wiki/DirectSound "DSound")).
* On desktop uses [PortAudio](https://portaudio.github.io/ "PortAudio").
* On dreamcast uses [KallistiOS](https://github.com/KallistiOS/KallistiOS "KallistiOS") APIs.
* Supports OGG Vorbis with [Vorbisfile](https://xiph.org/vorbis/doc/vorbisfile/ "Vorbisfile").
* Supports OGG Opus with [Opusfile](https://www.opus-codec.org/docs/opusfile_api-0.12/index.html "Opusfile") but is optional.
* Supports WAV with Yahama AICA 4-bit ADPCM only in the dreamcast.
* Automatically compiled with the engine.

### FFGraph

* *FFmpeg Graph*
* **C# & C only**
* Responsible for decoding video files (any format).
* On desktop uses [FFmpeg](https://www.ffmpeg.org/ "FFmpeg").
* On dreamcast uses [pl_ffmpeg](https://github.com/phoboslab/pl_mpeg "PL_MPEG") for MPEG1 Video decoding.

## FontAtlas

* *Font Atlas Builder*
* Creates a texture atlas from fonts.
* On javascript must be compiled as WebAssembly separately.

## LuaScript

* *Lua scripting*
* Provides bindings to lua including run and load of scripts.
* On javascript requires the LUA as WebAssembly binary to run.

### lua-interop

* *Lua interoperability*
* **C# only**
* Contains all unmanaged code for interacting with Lua API.
* Automatically compiled with the engine.

## texture-mgr

* *Texture manager*
* **C# only**
* Reads and write image files (PNG files).
* Parses icon files (ICO files)
* Automatically compiled with the engine.

## expansionsloader

* *Expansions Loader*
* **JS & C# only**
* An UI select and load expansions.
* Not available in the dreamcast, there no enough merits.
* Automatically compiled with the engine.
