# Lua workspace

This vscode project contains the source code of all week lua scripts. These scripts are written in [TypeScript](https://www.typescriptlang.org/ "TypeScript") and then transpiled to Lua using [TypescriptToLua](https://typescripttolua.github.io/ "TypescriptToLua").
I choosen TypeScript because is strongly typed, anyway, you can check out  other typed languages like [Pallene](https://github.com/pallene-lang/pallene "Pallene"), [MoonScript](https://moonscript.org/ "MoonScript"), [Clue](https://github.com/ClueLang/Clue "Clue"), [Teal](https://github.com/teal-language/tl "Teal") or simply write scripts in plain [Lua](https://www.lua.org/ "Lua").

---

## kdmyEngine.d.ts

This file contains all the engine accessible APIs. Is not complete, there are several APIs that are not yet available in lua.

## cheatset_callbacks.d.ts

A Cheatset list of all the functions that the engine will call from your script.

---

## Installing TypescriptToLua

* First of all you need [Node.JS](https://nodejs.org/en/ "Node.JS")
* Open the *luaworkspace* project in [VSCode](https://code.visualstudio.com/ "VSCode"), optional.
* Go to the terminal window.
* Write `npm install -D typescript-to-lua`. This will install **TypescriptToLua** inside *luaworkspace* only.
* Close and reopen VSCode if changes are not detected.
* Alternatively it is possible to do it with cmd or powershell inside the luaworkspace folder.

## Compiling lua scripts

* When you finish writing the script (in typescript language) execute the command in the console `npm run build` or in VSCode, in the **NPM Scripts** tab execute the task `build tstl`.
* If you see the error `error TS2393: Duplicate function implementation.` ignore it, the scripts compile anyways.
* All compiled lua files are inside of **build** folder.
