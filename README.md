# StarAce
Space shooter game for Linux

### About:
This project is my bachelor/engineer diploma work made in late 2012 using OpenGL and sucessfuly passed in march 2013 with best grade.

At the moment I treat it as a test bench to try out new ideas. To see what works for me and what not, what is convinient to use and what is not.
And in general to have fun with it, so ~~don't~~ expect best commits made at 3am :)

### Fast forward to present day
Hello Vulkan and few more improvements that should be there from the beginning :)

### Requirments:
* C++20 compiler
* Vulkan >= 1.3
* SDL 2 >= 2.30 (comes with LunarG Vulkan SDK)
  * or sdl2-compat (Linux)
* GLM >= 1.0.1 (comes with LunarG Vulkan SDK)
* Freetype 2 >= 2.10

### Target OS:
* Linux
* Windows - technically does work, but its not actively checked

### Example build instructions (Linux):
`cmake <path/to/repo/> --preset release; make -j 15`

### Example build instructions (Windows):
* Download and install LunarG Vulkan SDK with GLM and SDL2 `https://www.lunarg.com/vulkan-sdk/`
* Download precompiled Freetype 2 library `https://github.com/ubawurinna/freetype-windows-binaries`
  * `cd <path/to/repo>/sdk`
  * `git clone https://github.com/ubawurinna/freetype-windows-binaries.git`
* Ensure your Visual Studio has CMake with Ninja installed (see Visual Installer modules)
  * Don't forget Windows SDK install as well, the latest version the better
* Open `<path/to/repo>` directory in Visual Studio as CMake project
* Select build type `Release (Windows)`
* Select target view
* Select CMake Target view
* Build target `cook`
* Build target `starace`
* Set target `starace` as Startup Item
* Run

<img src="https://github.com/xmaciek/starace/blob/main/readme/targetview.png" width="128"/> <img src="https://github.com/xmaciek/starace/blob/main/readme/targetview2.png" width="128"/>
<img src="https://github.com/xmaciek/starace/blob/main/readme/cook.png" width="128"/> <img src="https://github.com/xmaciek/starace/blob/main/readme/starace.png" width="128"/>
