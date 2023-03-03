# Building

The engine is available in Javascript (for web browsers) and C# (Desktop, Windows). The C version is incomplete, `i giveup (c sourcecode).rar` file contains the sourcecode in C. Also the C version requires [DreamSDK](https://www.dreamsdk.org/ "DreamSDK") and [KallistiOS](https://cadcdev.sourceforge.net/softprj/kos/ "KallistiOS").

## Components

Everything was written from scratch, with no frameworks except low-level libraries and the .NET Framework itself. There are also specific parts that are written in C.

## Development environment

* [VSCode](https://code.visualstudio.com/ "VSCode"), optional.
* [Visual Studio](https://visualstudio.microsoft.com/es/ "Visual Studio")
* [MSYS2](https://www.msys2.org/ "MSYS2") with [mingw-w64 toolchain](https://packages.msys2.org/group/mingw-w64-i686-toolchain "mingw-w64-x86_64-toolchain"), optional. Required only for building components.
* [emscripten](https://emscripten.org/ "emscripten"), optional.

---

## FontAtlas

* *Font Atlas Builder*
* Creates a font texture atlas, written in C. Uses [freetype](https://freetype.org/ "freetype") library.
* The source code is located in the **fontatlas** folder.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run:
  * `make build-release-dll-x64` this creates **kdy_fontatlas.dll** file in *output* folder.
  * (optional) `make build-release` creates a [WASM](https://webassembly.org/ "WASM") file for the Javascript version.
* Note: "JS glue" code must be patched before being used.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.

## LuaScript

* *Lua scripting*
* Provides bindings to lua including run and load of scripts.
* Requires a [emscripten](https://emscripten.org/ "emscripten") static library of [Lua](https://www.lua.org/ "Lua").
* The source code is located in the **luascript** folder.
* Used **only** in the Javascript version.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run `make build-release`, this creates a [WASM](https://webassembly.org/ "WASM") file.
* Note: "JS glue" code must be patched before being used.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.
* *the C# version is automatically compiled with the engine.*

## SndBridge

* *Sound bridge*
* Responsible for playing the ogg files (vorbis and opus) as well as communicating with the audio rendering API (preferably [DSound](https://en.wikipedia.org/wiki/DirectSound "DSound")).
* Uses [PortAudio](https://portaudio.github.io/ "PortAudio"), [Vorbisfile](https://xiph.org/vorbis/doc/vorbisfile/ "Vorbisfile") and [Opusfile](https://www.opus-codec.org/docs/opusfile_api-0.12/index.html "Opusfile") libraries.
* The source code is located in the **sndbridge** folder.
* Used **only** in the C# version.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run `make build-release` this creates **kdy_AICA.dll** file in *output* folder.

## FFGraph

* *FFmpeg Graph*
* Responsible for decoding video files (any format).
* Uses [FFmpeg](https://www.ffmpeg.org/ "FFmpeg").
* The source code is located in the **ffgraph** folder.
* Headers of FFmpeg are required (not provided). See ffgraph/include/README.TXT for more info.
* Shared libraries (dll files) of FFmpeg are required (not provided). See ffgraph/lib/x64/README.TXT for more info.
* Used **only** in the C# version.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run `make build-release` this creates **kdy_FFgraph.dll** file in *output* folder.

## lua-interop

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

---

## Compiling Javascript version

Not required, but there a project for [VSCode](https://code.visualstudio.com/ "VSCode").
Note: if LuaScript and FontAtlas are updated, place the [WASM](https://webassembly.org/ "WASM") files inside of `/engine/externals` folder.

## Compiling C# version

Open `kdmy_engine.sln` with [Visual Studio](https://visualstudio.microsoft.com/es/ "Visual Studio") to compile, before launch you need various libraries and a copy of assets folder (see [INSTALLING.MD](INSTALLING.MD#2-getting-the-assets-folder "INSTALLING.MD") for instructions).
Requires the library [Newtonsoft.Json](https://www.newtonsoft.com/json "Newtonsoft.Json") via [NuGet](https://www.nuget.org/ "NuGet") package manager.

### IMPORTANT

Do not use **Clean solution** or **Clean project** in [Visual Studio](https://visualstudio.microsoft.com/es/ "Visual Studio"). This will wipe your `bin` folder deleting required dlls, saves, and the copy of **assets** folder.

---

### TODO

* friendly [VSCode](https://code.visualstudio.com/ "VSCode") project.
* C# version of FontAtlas and SndBridge components, goodbye C.
