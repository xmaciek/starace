#include "road.hpp"

void Road::OnKeyDown( SDLKey sym, SDLMod, Uint16 )
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

void Road::OnKeyUp( SDLKey sym, SDLMod, Uint16 )
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
        m_jet->RollLeft( true );
        break;
    case SDLK_d:
        m_jet->RollRight( true );
        break;
    case SDLK_q:
        m_jet->YawLeft( true );
        break;
    case SDLK_e:
        m_jet->YawRight( true );
        break;
    case SDLK_w:
        m_jet->PitchUp( true );
        break;
    case SDLK_s:
        m_jet->PitchDown( true );
        break;
    case SDLK_o:
        m_jet->SpeedUp( true );
        break;
    case SDLK_u:
        m_jet->SpeedDown( true );
        break;
    case SDLK_j:
        m_jet->Shoot( 0, true );
        break;
    case SDLK_k:
        m_jet->Shoot( 1, true );
        break;
    case SDLK_l:
        m_jet->Shoot( 2, true );
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
        m_jet->RollLeft( false );
        break;
    case SDLK_d:
        m_jet->RollRight( false );
        break;
    case SDLK_q:
        m_jet->YawLeft( false );
        break;
    case SDLK_e:
        m_jet->YawRight( false );
        break;
    case SDLK_w:
        m_jet->PitchUp( false );
        break;
    case SDLK_s:
        m_jet->PitchDown( false );
        break;
    case SDLK_o:
        m_jet->SpeedUp( false );
        break;
    case SDLK_u:
        m_jet->SpeedDown( false );
        break;
    case SDLK_j:
        m_jet->Shoot( 0, false );
        break;
    case SDLK_k:
        m_jet->Shoot( 1, false );
        break;
    case SDLK_l:
        m_jet->Shoot( 2, false );
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
