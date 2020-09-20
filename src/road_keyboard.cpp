#include "road.hpp"

void Road::OnKeyDown( SDLKey sym, SDLMod, Uint16 )
{
    switch ( sym ) {
    case SDLK_F11:
        GoFullscreen( m_isFullscreen );
        break;
    default:
        switch ( m_currentScreen ) {
        case Screen::eGame:
            GameKeyboardPressed( sym );
            break;
        case Screen::eGamePaused:
            GameKeyboardPausedPressed( sym );
            break;
        case Screen::eGameBriefing:
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
    switch ( m_currentScreen ) {
    case Screen::eGame:
        GameKeyboardUnpressed( sym );
        break;
    case Screen::eGamePaused:
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
        m_jet->rollLeft( true );
        break;
    case SDLK_d:
        m_jet->rollRight( true );
        break;
    case SDLK_q:
        m_jet->yawLeft( true );
        break;
    case SDLK_e:
        m_jet->yawRight( true );
        break;
    case SDLK_w:
        m_jet->pitchUp( true );
        break;
    case SDLK_s:
        m_jet->pitchDown( true );
        break;
    case SDLK_o:
        m_jet->speedUp( true );
        break;
    case SDLK_u:
        m_jet->speedDown( true );
        break;
    case SDLK_j:
        m_jet->shoot( 0, true );
        break;
    case SDLK_k:
        m_jet->shoot( 1, true );
        break;
    case SDLK_l:
        m_jet->shoot( 2, true );
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
        m_jet->rollLeft( false );
        break;
    case SDLK_d:
        m_jet->rollRight( false );
        break;
    case SDLK_q:
        m_jet->yawLeft( false );
        break;
    case SDLK_e:
        m_jet->yawRight( false );
        break;
    case SDLK_w:
        m_jet->pitchUp( false );
        break;
    case SDLK_s:
        m_jet->pitchDown( false );
        break;
    case SDLK_o:
        m_jet->speedUp( false );
        break;
    case SDLK_u:
        m_jet->speedDown( false );
        break;
    case SDLK_j:
        m_jet->shoot( 0, false );
        break;
    case SDLK_k:
        m_jet->shoot( 1, false );
        break;
    case SDLK_l:
        m_jet->shoot( 2, false );
        break;
    default:
        break;
    }
}

void Road::GameKeyboardBriefingPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_SPACE:
        ChangeScreen( Screen::eGame );
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
