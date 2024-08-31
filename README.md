# StarAce
Space shooter game for Linux

### About:
This project is my bachelor/engineer diploma work made in late 2012 using OpenGL and sucessfuly passed in march 2013 with best grade.

At the moment I treat it as a test bench to try out new ideas. To see what works for me and what not, what is convinient to use and what is not.
And in general to have fun with it, so don't expect best commits made at 3am :)

### Fast forward to present day
Hello Vulkan and few more improvements that should be there from the beginning :)

### Requirments:
* C++20 compiler
* Vulkan >= 1.3
* SDL 2 >= 2.30 ( comes with LunarG vulkan SDK )
* GLM >= 1.0.1 ( comes with LunarG vulkan SDK )
* Freetype 2 >= 2.10

### Target OS:
* Linux
* Windows - technically does work, but its not actively checked

### Example build instructions:
`cmake path/to/repo/ --preset release; make -j 15`
