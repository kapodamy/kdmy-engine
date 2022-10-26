# Installing

This is going to be divided into 3 parts, because MUCHO TEXTO.
Before continue create a new folder somewhere (eg *dist*), here all compiled files will be placed.

---

## 1. Copy required libraries

All required libraries are inside of **external_libraries-x86** folder, easy. This includes:

* `freetype.dll`
* `glfw.dll`
* `libogg-0.dll`
* `libvorbis-0.dll`
* `libvorbisfile-3.dll`
* `libportaudio.dll`
* `lua.dll`
* `kdy_AICA.dll` (prebuilt)
* `kdy_fontatlas.dll` (prebuilt)

If sndbrigde was built with opus codec support, the following dll files also are required:

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

### 2b. Week 7 assets

¿Is week 7 still is exclusive to [Newgrounds.com](https://www.newgrounds.com/ "Newgrounds.com")?.

## 3. Building the engine

See [BUILDING.MD](BUILDING.MD "BUILDING.MD") for instructions.
If the engine is already built, grab the files from `/src-desktop/kdy-e/bin/Release/` and place in the in the *dist* folder.
Javascript version of the engine does not require any compilation, just copy the folder `engine` and the files `kdmy_engine.html` and `kdmy_engine_layout_visor.html` from **src-javascript**.

## Final result

![files](./test%20files/kdmy%20pics/files.PNG)

### Launching the javascript version

Open `kdmy_engine.html` to launch the engine or `kdmy_engine_layout_visor.html` to open the *Layout Visor*.
