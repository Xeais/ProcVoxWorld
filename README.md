# ProcVoxWorld

A huge procedurally generated voxel world with coherent graphics 

![ProcVoxWorld from a bird's eye view](https://user-images.githubusercontent.com/18394014/153664300-9af6a2d7-df8c-4592-aec4-2285232d4cf3.png)
![Many different biomes, again, from a bird's eye view](https://user-images.githubusercontent.com/18394014/153664726-b1629e3b-1ecb-40c2-8920-4c3fee28284c.png)
![These are some nice trees.](https://user-images.githubusercontent.com/18394014/153665064-db7126be-8506-486e-9702-926f37ec3cee.png)

## Screencast
[![Presentation of ProcVoxWorld by a YouTube-Screencast](https://user-images.githubusercontent.com/18394014/154851198-aef027f4-5577-42af-89f5-95c11eef90cc.png)
](https://youtu.be/5-GRuv55CA0)

## Some Features
- Almost infinite multithreaded world generation
- Six different biomes, each with a unique heightmap
- Player physics (on land plus in water)
- Voxels (alternatively: blocks) can be deleted and placed freely.
- [SQLite](https://sqlite.org)-database to store map and player information.
- Depth of field (smooth/non-smooth)
- Motion blur
- Day-night cycle (including dusk)
- A configuration file (INI) to change various parameters without recompiling.

## Controls
- WASD = Move player forth/back and left/right.
- Tab = Toggle fly mode.
- Shift = Run mode or fly up
- Ctrl = Sneak or fly down
- Hold key Up/Down = Increase/decrease fly camera speed.
- Space bar = Jump or emerge
- C = Camera zoom
- P = Toggle debug picture in picture view for shadow maps.
- Left mouse button = Delete block.
- Right mouse button = Place block.
- Mouse scroll wheel = Change build block.
- Esc = Release the cursor.

## Supported Platforms
The release versions run with ...
- Windows 10/11: :thumbsup: plus ...
- POSIX-compliant operating systems:
  - most Linux distributions: :open_hands: and ...
  - macOS: :open_hands:.

:thumbsup: = tested, okay | :open_hands: =	not tested, but most likely  
:point_right: https://stackoverflow.com/a/31865755/ 

## Line-up of Dependencies
|Dependency                                                |Version|Source                                                                               |
|----------------------------------------------------------|-------|-------------------------------------------------------------------------------------|
|[GLFW](https://www.glfw.org)                              |3.3.6  |[vcpkg](https://vcpkg.io) (I compiled the library myself.)                           |
|[glad](https://github.com/Dav1dde/glad/)                  |0.1.35 |[Webservice](https://glad.dav1d.de)                                                  |
|[cglm](https://cglm.readthedocs.io)                       |0.8.4  |[GitHub-Release](https://github.com/recp/cglm/releases/tag/v0.8.4/) (see above, GLFW)|
|[TinyCThread](https://tinycthread.github.io)              |1.2    |[GitHub](https://github.com/tinycthread/tinycthread/tree/master/source/)             |
|[ini](https://github.com/rxi/ini/)                        |0.1.1  |[GitHub](https://github.com/rxi/ini/tree/master/src/)                                |
|[stb_image](https://github.com/nothings/stb/)             |2.27   |[GitHub](https://github.com/nothings/stb/blob/master/stb_image.h)                    |
|[SQLite](https://www.sqlite.org)                          |3.37.0 |[Vcpkg](https://vcpkg.io)                                                            |
|[FastNoise Lite](https://github.com/Auburn/FastNoiseLite/)|1.0.3  |[GitHub-Release](https://github.com/Auburn/FastNoiseLite/releases/tag/v1.0.3/)       |

**Explanatory notes:**
- *Rule*: If the dependency offers one or more releases, go for the latest one unless it is several years behind the main branch.
- This file and the directory structure for the dependencies (based on the example of vcpkg) were created on *January 21, 2022*.

## Requirements
- Windows 10 (Version 21H1) – only tested with that version
- Requires an up-to-date Windows 10 SDK; version 10.0.20348.0 does the job. Windows 11 with its SDK (from version 10.0.22000) should also be functional as development platform.
- If you set up your own development environment, you should also be able to work with ProcVoxWorld under Linux or macOS.
---
- All dependencies are already included.
- No include pathes / other pathes have to be adjusted because macros are used in the [Visual Studio](https://visualstudio.microsoft.com/vs/) [solution (.sln) file](https://docs.microsoft.com/en-us/visualstudio/extensibility/internals/solution-dot-sln-file?view=vs-2019). While it's certainly possible to get it working with another IDE (see: third spot right above), with Visual Studio 2019 and up ([Community](https://visualstudio.microsoft.com/vs/community/) is completely sufficient) it will be the easiest, as the project was obviously created with it.
