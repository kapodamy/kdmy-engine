
# KDMY Engine: Friday Night Funkin'

![dreamcast](https://img.shields.io/badge/dreamcast%20build-failing-red) ![C#](https://img.shields.io/badge/contains-C%23-informational) ![JS](https://img.shields.io/badge/contains-Javascript-informational) ![C](https://img.shields.io/badge/contains-C-informational) ![haxe](https://img.shields.io/badge/contains%20haxe%3F-haxen't-inactive)

kdy-e is an engine for FNF, written from scratch in javascript|C|C# (no haxe here). It was build specifically for the Sega Dreamcast, it was . . .

![at last 2](./test%20files/kdmy%20pics/at%20last%202.PNG)

## Features

As for the basics, the engine has support for gamepads, multiple saveslots, multiple players (2 limited by funkin chart format and available screen space) and self-pause if the game is minimized or unfocused. The engine was intended to not modify existing code, instead offer APIs and standardized ways to make modifications, detailed here:

* Additional weeks, by placing a new week in the weeks folder.
  * Multiple tracks with support for alternate versions (eg non-copyright).
  * You can put a message explaining why there are alternate tracks.
  * Custom difficulties (specific to each week).
  * Week's host can do "HEY!" after selecting the week.
  * Custom background color in week selector.
  * Support for custom BF and GF.
  * Each week can specify if custom BF and GF are not accepted.
  * Unlock directives, limits playable weeks and allows unlockable characters.
  * Customization of specific files for each week.
  * Week customizations for the engine (background images, menu music) once is completed.
* Support for downscroll, leftscroll and rightscroll. By default upscroll and downscroll is included, modifiable if the week requires it.
* Funkin chart format support (in reality, the only supported format).
* Support for custom strums (distributions).
  * Variable number of strum lines up to 9 lines. Requires a custom texture&atlas.
  * Hidden/diamond strum (for 5, 7 or 9 lines config) for actions like "press space to jump" or "press space to dodge".
  * Custom notes, each one with attributes like damage/heal ratio, ignore hit/miss or special-marked (for later use in lua).
  * In the chart file, each note can have an extra number-only value (the 4th array value) for later use in lua.
  * Strums, character and healthbar states. All of these are experimental, for "texture swap" like alt-sing animations .
* Per-track distributions (experimental), stage, lua script and characters.
* Credits (can be customized per week), also has lua scripting support.
* Each week and/or track stage can define the placement of characters and UI cosmetics (streak counter, ranking counter and accuracy).
* Partial week lua scripting support (also known as "modcharts").
* Built-in engine features (explained below)

## Engine especific features

Explained here [FEATURES.MD](FEATURES.MD "FEATURES.MD")

## Screenshots

![saveselector](test%20files/kdmy%20pics/screenshots/saveselector.png)
![week tutorial](test%20files/kdmy%20pics/screenshots/week%20tutorial.png)
![pause](test%20files/kdmy%20pics/screenshots/pause.png)
![weekselector](test%20files/kdmy%20pics/screenshots/weekselector.png)

## Missing features / To be implemented

* ~~Post-processing shaders~~. ¡DONE!
* ~~Dialogues (waht a shame)~~. ¡DONE!
* Expansions loader (a.k.a mods loader).
* ~~Lua API to manipulate strums, states and UI cosmetics~~. ¡DONE!
* Documentation (a lot).
* Video playback (kinda optional).

## How to compile

See [BUILDING.MD](BUILDING.MD "BUILDING.MD") and [INSTALLING.MD](INSTALLING.MD "INSTALLING.MD") for instructions

## A brief history

The idea was to bring the game to the console but due to the lack of an emulator without the ability to debug homebrew, the project was nearly impossible. Knowing this limitation, the engine was developed in javascript and the source code was written in "C style" to later transpile it once its development was completed. Even so, it was necessary to write specific code for the console (audio and graphics) and in the end the development was canceled.
Currently, the engine is ported to C# to make it playable with the exception of some low level libraries which are written in C.

## More to know

Engine was made using by reference Kade-Engine v1.4.2 and Funkin v0.2.7.1 source code. But as reference, fr fr, not only was it written from scratch but the game was also reverse engineered because the sources were unreadable (denuvo, ¿quien te conoce papa?). I got to the point of measuring the elements on the screen with a ruler, not kidding.

# Final words

Several aspects of the gameplay were rebuilt, for example, the note ranking is based on how far away you are from the marker instead of using "166ms" (I don't know what this number means), the UI has a "zoom" every 4 beats, chart speed affects GF idle animation, etc.
