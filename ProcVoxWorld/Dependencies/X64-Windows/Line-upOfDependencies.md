|Dependency                                                |Version|Source                                                                               |
|----------------------------------------------------------|-------|-------------------------------------------------------------------------------------|
|[GLFW](https://www.glfw.org)                              |3.3.6  |[vcpkg](https://vcpkg.io) (I compiled the library myself.)                           |
|[glad](https://github.com/Dav1dde/glad/)                  |0.1.35 |[Webservice](https://glad.dav1d.de)                                                  |
|[cglm](https://cglm.readthedocs.io)                       |0.8.4  |[GitHub-Release](https://github.com/recp/cglm/releases/tag/v0.8.4/) (see above, GLFW)|
|[TinyCThread](https://tinycthread.github.io).             |1.2    |[GitHub](https://github.com/tinycthread/tinycthread/tree/master/source/)             |
|[ini](https://github.com/rxi/ini/)                        |0.1.1  |[GitHub](https://github.com/rxi/ini/tree/master/src/)                                |
|[stb_image](https://github.com/nothings/stb/)             |2.27   |[GitHub](https://github.com/nothings/stb/blob/master/stb_image.h)                    |
|[SQLite](https://www.sqlite.org)                          |3.37.0 |[Vcpkg](https://vcpkg.io)                                                            |
|[FastNoise Lite](https://github.com/Auburn/FastNoiseLite/)|1.0.3  |[GitHub-Release](https://github.com/Auburn/FastNoiseLite/releases/tag/v1.0.3/)       |

**Explanatory notes:**
- *Rule*: If the dependency offers one or more releases, go for the latest one unless it is several years behind the main branch.
- This file and the directory structure for the dependencies (based on the example of vcpkg) were created on *January 21, 2022*.