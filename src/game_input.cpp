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

void Game::onMouseClickLeft( int32_t x, int32_t y )
{
    const MouseEvent mouseEvent = MouseClick{ glm::vec2{ x, y } };

    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        m_btnQuitMission.onMouseEvent( mouseEvent );
        break;

    case Screen::eMainMenu:
        m_btnSelectMission.onMouseEvent( mouseEvent )
        || m_btnCustomize.onMouseEvent( mouseEvent )
        || m_btnExit.onMouseEvent( mouseEvent )
        ;
        break;

    case Screen::eMissionSelection:
        m_btnStartMission.onMouseEvent( mouseEvent )
        || m_btnReturnToMainMenu.onMouseEvent( mouseEvent )
        || m_btnNextMap.onMouseEvent( mouseEvent )
        || m_btnPrevMap.onMouseEvent( mouseEvent )
        ;
        break;

    case Screen::eDead:
        m_screenLoose.onMouseEvent( mouseEvent );
        break;

    case Screen::eWin:
        m_screenWin.onMouseEvent( mouseEvent );
        break;

    case Screen::eGameBriefing:
        m_btnGO.onMouseEvent( mouseEvent );
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

void Game::onMouseMove( const SDL_MouseMotionEvent& event )
{
    const MouseEvent mouseEvent = MouseMove{ glm::vec2{ event.x, event.y } };
    m_btnCustomizeReturn.onMouseEvent( mouseEvent );
    m_btnCustomize.onMouseEvent( mouseEvent );
    m_btnExit.onMouseEvent( mouseEvent );
    m_btnGO.onMouseEvent( mouseEvent );
    m_btnNextJet.onMouseEvent( mouseEvent );
    m_btnNextMap.onMouseEvent( mouseEvent );
    m_btnPrevJet.onMouseEvent( mouseEvent );
    m_btnPrevMap.onMouseEvent( mouseEvent );
    m_btnQuitMission.onMouseEvent( mouseEvent );
    m_btnReturnToMainMenu.onMouseEvent( mouseEvent );
    m_btnSelectMissionCancel.onMouseEvent( mouseEvent );
    m_btnSelectMission.onMouseEvent( mouseEvent );
    m_btnStartMission.onMouseEvent( mouseEvent );
    m_btnWeap1.onMouseEvent( mouseEvent );
    m_btnWeap2.onMouseEvent( mouseEvent );
    m_btnWeap3.onMouseEvent( mouseEvent );

    m_screenWin.onMouseEvent( mouseEvent );
    m_screenLoose.onMouseEvent( mouseEvent );
}
