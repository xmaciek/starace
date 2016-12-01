# StarAce
OpenGL based space shooter game for Linux

### About:
This project is my bachelor/engineer diploma work made in late 2012 and sucessfuly defneded in march 2013 with best mark.
Although this game is made using OpenGL 2 specification, it was due to time limitation (6 months) and my quite archaic hardware at the time which did not support newer OpenGL very well.

Also in mid 2013 I suffered hard disk failure and I lost every further improvements like billboarding and sprites support. So I didn't do anything more with it.


Right now I am considering this project to be kind of a sandbox for my ideas.

### Requirments:
Linux, SDL 1.2

### Mid-Long term goals:
After porting to OpenGL 3.3 is done (in `wip/gl3` branch, strongly recommended to also take a look), move the OpenGL specific implementation to a separate library.
This will allow implementing Vulkan backend. To follow Valve general advice, game engine should be unaware of rendering context.

###### This project is under GNU GPLv3 license.
