# Building for C

## Assets

Due hardware limitations, desktop/web assets can not be used as-is in the dreamcast.
A proper conversion is required, however, the base game assets are already provided in the `assets_dc` folder.
Any attempt to port expansions (mods) must take in care the hardware limitations and the amount of memory available.
Tools are here [kdmy-engine-tools](https://github.com/kapodamy/kdmy-engine-tools), required for xml, json, png and ogg conversion. 

## Expansions

There no UI implemented to choose an expansion (but the logic is there), if necessary follow this steps:

* Goto `src-dreamcast/output/cd_root/expansions` folder, create the folder `expansions` if does not exists.
* Place the expansion folder inside, remember, the expansion assets must be converted first.
* Create a txt file called `expansion.txt`.
* Write inside the name of the expansion to load, this must be the expansion's folder name on `src-dreamcast/output/cd_root/expansions` folder.
* Note: the `funkin` expansion is always loaded if the folder exists.

## Required libraries

First of all, the libraries **FreeType**, **LUA** and **libtremor** must be present. Instructions:

* Run **DreamSDK Manager**, this can be done by typing `dreamsdk manage` in the **DreamSDK Shell** prompt.
* Goto tab **KallistiOS Ports**.
* One by one, select the required libraries and click "Install" in the right panel.
* Some additional dependencies may be downloaded.

## Building the engine

Open the **DreamSDK Shell** prompt on `src-dreamcast` and write `make build-release`, this will compile and build the dreamcast disc image (an cdi file).
To rebuild only the dreamcast disc image file write `make disc`, this will repacks the files inside of `output/cd_root` folder.
