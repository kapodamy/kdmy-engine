# Installing

This is going to be divided into 3 parts, because MUCHO TEXTO.
Before continue create a new folder somewhere (eg *dist*), here all compiled files will be placed.

---

## 1. Copy required libraries

All required libraries are inside of **external_libraries-x64** folder, easy. This includes:

* `freetype.dll`
* `glfw.dll`
* `libogg-0.dll`
* `libvorbis-0.dll`
* `libvorbisfile-3.dll`
* `libportaudio.dll`
* `lua.dll`
* `kdy_AICA.dll` (prebuilt)
* `kdy_fontatlas.dll` (prebuilt)
* `kdy_FFgraph.dll` (prebuilt)

If sndbrigde was built with OPUS codec support, the following dll files also are required otherwise **the engine will not run**:

* `opusfile.dll`
* `opus.dll`
* `libssl-1_1.dll`
* `libcrypto-1_1.dll`

If video playback is necessary, download the [FFmpeg](https://www.ffmpeg.org/download.html#build-windows "FFmpeg download link") shared libraries, required dll files are:

* `avcodec-60.dll`
* `avformat-60.dll`
* `avutil-58.dll`
* `swscale-7.dll`
* `swresample-4.dll`

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

## Final result

![files](./test%20files/kdmy%20pics/files.PNG)

### Launching the javascript version

Open `kdmy_engine.html` to launch the engine or `kdmy_engine_layout_visor.html` to open the *Layout Visor*.
If the engine does not load try [allowing access to local files](https://dev.to/dengel29/loading-local-files-in-firefox-and-chrome-m9f "guide") or mounting a web server.
