#include "game.hpp"

#include <SDL/SDL.h>

void Game::onKeyDown( SDLKey sym, SDLMod, Uint16 )
{
    switch ( sym ) {
    case SDLK_F11:
        goFullscreen( m_isFullscreen );
        break;
    default:
        switch ( m_currentScreen ) {
        case Screen::eGame:
            gameKeyboardPressed( sym );
            break;
        case Screen::eGamePaused:
            gameKeyboardPausedPressed( sym );
            break;
        case Screen::eGameBriefing:
            gameKeyboardBriefingPressed( sym );
            break;
        default:
            break;
        }
        break;
    }
}

void Game::onKeyUp( SDLKey sym, SDLMod, Uint16 )
{
    switch ( m_currentScreen ) {
    case Screen::eGame:
        gameKeyboardUnpressed( sym );
        break;
    case Screen::eGamePaused:
        gameKeyboardPausedUnpressed( sym );
        break;
    default:
        break;
    }
}

void Game::gameKeyboardPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_ESCAPE:
        pause();
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
        retarget();
        break;
    default:
        break;
    }
}

void Game::gameKeyboardUnpressed( SDLKey sym )
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

void Game::gameKeyboardBriefingPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_SPACE:
        changeScreen( Screen::eGame );
        break;
    default:
        break;
    }
}

void Game::gameKeyboardPausedPressed( SDLKey sym )
{
    switch ( sym ) {
    case SDLK_ESCAPE:
        unpause();
        break;
    default:
        gameKeyboardPressed( sym );
        break;
    }
}

void Game::gameKeyboardPausedUnpressed( SDLKey sym )
{
    gameKeyboardUnpressed( sym );
}
