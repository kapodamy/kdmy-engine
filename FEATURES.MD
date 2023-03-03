# Engine features

Engine specific APIs

* Layout, is a html-like file used for building stages and screens.
  * Everything is written in XML, no codding needed.
  * Can contains images (sprites), text and sounds.
  * Basic alignment and resize for sprites and text.
  * Import animations for sprites, text and groups.
  * Camera support.
  * 3-axis parallax.
  * Triggers, useful for beat-looped animations.
  * Groups. Allows control opacity, visibility, parallax, and matrix-transforms for grouped graphics items.
  * Placeholders. Used for building custom menu screens (note: menus itself can not be modded)
  * Antialiasing control, for use in pixelated stages.
  * Actions, partial scripted layout modifications.
  * Video playback.
* Non-scripted animations
  * Also written in XML.
  * Frame-based animations:
    * For defining animations using an atlas (texture atlas).
    * Individual or grouped frame import.
    * Alternative sets (used for dance-left/right animations and note splashes)
* Macro-based animations:
  * Partially scripted animations.
  * Interpolators (tweens).
  * Randomization by range or list of values.
  * Mostly used for complex animations like fading.
* Gameplay specific features:
  * FPS limiter.
  * Keyboard binding.
  * Allows change the difficult in the gameover screen
  * Ask player if "Ready?"
  * The Healthbar warns the player if the opponent recovers health.
* Expansions, intended to add modding support.
  * An expansion can add new files or replace existing ones without touching the "assets" folder.
  * Expansions can be "chained" using a chain file.
  * Currently there is no option in the configuration that allows load expansions, it must be done launching the engine using command line.
* Output debugging, to display the messages issued by the engine or lua scripts, the engine must be launched from the cmd (command prompt).
* Manifest files, manifests are used to define a behavior or definitions:
  * Characters.
  * Gameplay.
  * Week.
  * Menus.
* Layout visor, an html page used to visualize layouts.
* Engine menus customization and creation of custom menu screens.
