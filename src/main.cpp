#include "game.hpp"

int main( int argc, char** argv )
{
    (void)argc;
    (void)argv;
    Game* game = new Game();
    const int ret = game->run();
    delete game;
    return ret;
}
