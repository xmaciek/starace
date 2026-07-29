#include "game.hpp"

#include <SDL3/SDL_main.h>

int main( int argc, char** argv )
{
    return Game{ argc, argv }.run();
}
