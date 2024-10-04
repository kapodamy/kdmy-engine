# Installing

This is going to be divided into 3 parts, because MUCHO TEXTO.
Before continue create a new folder somewhere (eg *dist*), here all compiled files will be placed.

---

## 1. Copy required libraries (desktop only)

All required libraries are inside of **external_libraries-x64** folder, easy. This includes:

* `freetype.dll`
* `glfw3.dll`
* `libogg-0.dll`
* `libvorbis-0.dll`
* `libvorbisfile-3.dll`
* `libportaudio.dll`
* `lua.dll`
* `avcodec-61.dll`
* `avformat-61.dll`
* `avutil-59.dll`
* `swresample-5.dll`
* `swscale-8.dll`

If sndbrigde was built with OPUS codec support, the following dll files also are required:

* `opusfile.dll`
* `opus.dll`
* `libssl-1_1.dll`
* `libcrypto-1_1.dll`

## 2. Getting the assets folder

The engine provides a different file order plus extra files used by the engine itself.
Building *assets* folder:

* Copy **assets** folder to *dist* folder.
* (optional) Copy the **expansions** folder to *dist* folder.

### 2a. Strum backgrounds

Every strum line can have a background, a model (texture + atlas) is required with the name **strumBackground** (strumBackground.xml+strumBackground.png)  in `/assets/common/image/week-round` folder. In **test files** folder the file `/assets/test/strumBackground.xml` is an example of strums having backgrounds.
This feature is optional.

## 3. Building the engine

See [BUILDING.MD](BUILDING.MD "BUILDING.MD") for instructions.
If the engine is already built, grab the files from `/src-desktop/kdy-e/bin/Release/` and place in the in the *dist* folder.
Javascript version of the engine does not require any compilation, just copy the folder `engine` and the files `kdmy_engine.html` and `kdmy_engine_layout_visor.html` from **src-javascript**.

### Launching the javascript version

Open `kdmy_engine.html` to launch the engine or `kdmy_engine_layout_visor.html` to open the *Layout Visor*.
If the engine does not load try [allowing access to local files](https://dev.to/dengel29/loading-local-files-in-firefox-and-chrome-m9f "guide") or mounting a web server.

### Launching the dreamcast version

The GD-ROM file is located at `src-dreamcast/output/kdy_e.cdi`. The MakeFile can create a mds/mdf disc image by using `make disc-mds` after running `make build-debug` or `make build-release`.
