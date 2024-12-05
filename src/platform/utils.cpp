#include <platform/utils.hpp>

#include <cstdlib>

#include <SDL_messagebox.h>

namespace platform {

void showFatalError( const std::string& title, const std::string& message )
{
    SDL_MessageBoxButtonData btn{
        .text = "OK",
    };
    SDL_MessageBoxData msg{
        .flags = SDL_MESSAGEBOX_ERROR,
        .title = title.c_str(),
        .message = message.c_str(),
        .numbuttons = 1,
        .buttons = &btn,
    };
    int ret = -1;
    SDL_ShowMessageBox( &msg, &ret );
    std::quick_exit( 1 );
}

}
