# Building for javascript

## FontAtlas

* *Font Atlas Builder*
* Creates a font texture atlas, written in C. Uses [freetype](https://freetype.org/ "freetype") library.
* Requires a [emscripten](https://emscripten.org/ "emscripten") to compile.
* The source code is located in the **fontatlas-javascript** folder.
* Used **only** in the Javascript version.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run `make build-release` creates a [WASM](https://webassembly.org/ "WASM") file for the Javascript version.
* copy the **module_fontatlas.wasm** and **module_fontatlas.js** files to `/src-javascript/engine/externals` folder.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.

## LuaScript

* *Lua scripting*
* Provides bindings to LUA required to run scripts.
* Requires a [emscripten](https://emscripten.org/ "emscripten") to compile.
* The source code is located in the **luascript-javascript** folder.
* Used **only** in the Javascript version.
* To compile open **MSYS2 MinGW x64** command prompt inside of the folder and run `make build-release`, this creates a [WASM](https://webassembly.org/ "WASM") file.
* copy the **lua.wasm** and **lua.js** files to `/src-javascript/engine/externals` folder.
* Project for [VSCode](https://code.visualstudio.com/ "VSCode") available.

## Engine

* Already written in javascript, there nothing to compile.
* The code is on `/src-javascript/engine`.
