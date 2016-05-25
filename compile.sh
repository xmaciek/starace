#!/bin/bash
cd src
rm *~
cd ..
rm *~
rm road
clear
clear
clear

#  valgrind --tool=memcheck --leak-check=full --track-origins=yes ./road
g++ -Wall -o road src/*.cpp -lm -lSDL -lSDL_ttf -lSDL_mixer -lGL -lGLU -g

if [ -f road ] ;
  then
    ./road
fi

