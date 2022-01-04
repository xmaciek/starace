#include "game.hpp"

void Game::onKeyDown( const SDL_Keysym& ks )
{
    switch ( m_currentScreen ) {
    case Screen::eGame:
        gameKeyboardPressed( ks.scancode );
        break;
    case Screen::eGamePaused:
        gameKeyboardPausedPressed( ks.scancode );
        break;
    case Screen::eGameBriefing:
        gameKeyboardBriefingPressed( ks.scancode );
        break;
    default:
        break;
    }
}

void Game::onKeyUp( const SDL_Keysym& ks )
{
    switch ( m_currentScreen ) {
    case Screen::eGame:
        gameKeyboardUnpressed( ks.scancode );
        break;
    case Screen::eGamePaused:
        gameKeyboardPausedUnpressed( ks.scancode );
        break;
    default:
        break;
    }
}

void Game::gameKeyboardPressed( SDL_Scancode sc )
{
    switch ( sc ) {
    case SDL_SCANCODE_ESCAPE:
        pause();
        return;
    case SDL_SCANCODE_I:
        retarget();
        return;
    default:
        break;
    }
}

void Game::gameKeyboardUnpressed( SDL_Scancode )
{
}

void Game::gameKeyboardBriefingPressed( SDL_Scancode sc )
{
    switch ( sc ) {
    case SDL_SCANCODE_SPACE:
        changeScreen( Screen::eGame );
        break;
    default:
        break;
    }
}

void Game::gameKeyboardPausedPressed( SDL_Scancode sc )
{
    switch ( sc ) {
    case SDL_SCANCODE_ESCAPE:
        unpause();
        break;
    default:
        gameKeyboardPressed( sc );
        break;
    }
}

void Game::gameKeyboardPausedUnpressed( SDL_Scancode sc )
{
    gameKeyboardUnpressed( sc );
}

void Game::onMouseEvent( const MouseEvent& mouseEvent )
{
    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        m_screenPause.onMouseEvent( mouseEvent );
        break;

    case Screen::eMainMenu:
        m_screenTitle.onMouseEvent( mouseEvent );
        break;

    case Screen::eMissionSelection:
        m_screenMissionSelect.onMouseEvent( mouseEvent );
        break;

    case Screen::eDead:
        m_screenLoose.onMouseEvent( mouseEvent );
        break;

    case Screen::eWin:
        m_screenWin.onMouseEvent( mouseEvent );
        break;

    case Screen::eCustomize:
        m_btnPrevJet.onMouseEvent( mouseEvent )
        || m_btnNextJet.onMouseEvent( mouseEvent )
        || m_btnCustomizeReturn.onMouseEvent( mouseEvent )
        || m_btnWeap1.onMouseEvent( mouseEvent )
        || m_btnWeap2.onMouseEvent( mouseEvent )
        || m_btnWeap3.onMouseEvent( mouseEvent )
        ;
        break;

    default:
        break;
    }
}
