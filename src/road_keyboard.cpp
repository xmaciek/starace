#include "road.hpp"

void Road::OnKeyDown( SDLKey sym, SDLMod mod, Uint16 unicode )
{
    switch ( sym ) {
    case SDLK_F11:
        GoFullscreen( FULLSCREEN );
        break;
    default:
        switch ( SCREEN ) {
        case SA_GAMESCREEN:
            GameKeyboardPressed( sym );
            break;
        case SA_GAMESCREEN_PAUSED:
            GameKeyboardPausedPressed( sym );
            break;
        case SA_GAMESCREEN_BRIEFING:
            GameKeyboardBriefingPressed( sym );
            break;
        default:
            break;
        }
        break;
    }
}

void Road::OnKeyUp( SDLKey sym, SDLMod mod, Uint16 unicode )
{
    switch ( SCREEN ) {
    case SA_GAMESCREEN:
        GameKeyboardUnpressed( sym );
        break;
    case SA_GAMESCREEN_PAUSED:
        GameKeyboardPausedUnpressed( sym );
        break;
    default:
        break;
    }
}

void Road::GameKeyboardPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_ESCAPE:
        Pause();
        break;
    case SDLK_a:
        jet->RollLeft( true );
        break;
    case SDLK_d:
        jet->RollRight( true );
        break;
    case SDLK_q:
        jet->YawLeft( true );
        break;
    case SDLK_e:
        jet->YawRight( true );
        break;
    case SDLK_w:
        jet->PitchUp( true );
        break;
    case SDLK_s:
        jet->PitchDown( true );
        break;
    case SDLK_o:
        jet->SpeedUp( true );
        break;
    case SDLK_u:
        jet->SpeedDown( true );
        break;
    case SDLK_j:
        jet->Shoot( 0, true );
        break;
    case SDLK_k:
        jet->Shoot( 1, true );
        break;
    case SDLK_l:
        jet->Shoot( 2, true );
        break;
    case SDLK_i:
        Retarget();
        break;
    default:
        break;
    }
}

void Road::GameKeyboardUnpressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_a:
        jet->RollLeft( false );
        break;
    case SDLK_d:
        jet->RollRight( false );
        break;
    case SDLK_q:
        jet->YawLeft( false );
        break;
    case SDLK_e:
        jet->YawRight( false );
        break;
    case SDLK_w:
        jet->PitchUp( false );
        break;
    case SDLK_s:
        jet->PitchDown( false );
        break;
    case SDLK_o:
        jet->SpeedUp( false );
        break;
    case SDLK_u:
        jet->SpeedDown( false );
        break;
    case SDLK_j:
        jet->Shoot( 0, false );
        break;
    case SDLK_k:
        jet->Shoot( 1, false );
        break;
    case SDLK_l:
        jet->Shoot( 2, false );
        break;
    default:
        break;
    }
}

void Road::GameKeyboardBriefingPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_SPACE:
        ChangeScreen( SA_GAMESCREEN );
        break;
    default:
        break;
    }
}

void Road::GameKeyboardPausedPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_ESCAPE:
        Unpause();
        break;
    default:
        GameKeyboardPressed( sym );
        break;
    }
}

void Road::GameKeyboardPausedUnpressed( SDLKey sym )
{
    GameKeyboardUnpressed( sym );
}
