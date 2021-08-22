#include "game.hpp"

void Game::onKeyDown( const SDL_Keysym& ks )
{
    switch ( ks.scancode ) {
    case SDL_SCANCODE_F11:
        goFullscreen( m_isFullscreen );
        break;
    default:
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
    assert( m_audio );
    y = (int32_t)viewportHeight() - y;
    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        if ( m_btnQuitMission.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eDead );
        }
        break;

    case Screen::eMainMenu:
        if ( m_btnSelectMission.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eMissionSelection );
            break;
        }
        if ( m_btnExit.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_isRunning = false;
            break;
        }
        if ( m_btnCustomize.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eCustomize );
            break;
        }
        break;

    case Screen::eMissionSelection:
        if ( m_btnStartMission.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eGameBriefing );
            break;
        }
        if ( m_btnReturnToMainMenu.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        assert( !m_mapsContainer.empty() );
        if ( m_btnNextMap.isClicked( x, y ) ) {
            m_currentMap++;
            m_btnNextMap.setEnabled( m_currentMap < m_mapsContainer.size() - 1 );
            m_btnPrevMap.setEnabled( m_currentMap > 0 );
            m_audio->play( m_click );
            break;
        }
        if ( m_btnPrevMap.isClicked( x, y ) ) {
            m_currentMap--;
            m_btnNextMap.setEnabled( m_currentMap < m_mapsContainer.size() - 1 );
            m_btnPrevMap.setEnabled( m_currentMap > 0 );
            m_audio->play( m_click );
            break;
        }
        break;

    case Screen::eDead:
    case Screen::eWin:
        if ( m_btnReturnToMissionSelection.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eMissionSelection );
        }
        break;

    case Screen::eGameBriefing:
        if ( m_btnGO.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eGame );
        }
        break;

    case Screen::eCustomize:
        if ( m_btnNextJet.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_currentJet++;
            m_btnPrevJet.setEnabled( m_currentJet > 0 );
            m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
            reloadPreviewModel();
            break;
        }
        if ( m_btnPrevJet.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_currentJet--;
            m_btnPrevJet.setEnabled( m_currentJet > 0 );
            m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
            reloadPreviewModel();
            break;
        }
        if ( m_btnCustomizeReturn.isClicked( x, y ) ) {
            m_audio->play( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        if ( m_btnWeap1.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_weap1++;
            if ( m_weap1 == 3 ) {
                m_weap1 = 0;
            }
            if ( m_weap1 == 0 ) {
                m_btnWeap1.setText( "Laser" );
            }
            if ( m_weap1 == 1 ) {
                m_btnWeap1.setText( "Blaster" );
            }
            if ( m_weap1 == 2 ) {
                m_btnWeap1.setText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap2.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_weap2++;
            if ( m_weap2 == 3 ) {
                m_weap2 = 0;
            }
            if ( m_weap2 == 0 ) {
                m_btnWeap2.setText( "Laser" );
            }
            if ( m_weap2 == 1 ) {
                m_btnWeap2.setText( "Blaster" );
            }
            if ( m_weap2 == 2 ) {
                m_btnWeap2.setText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap3.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_weap3++;
            if ( m_weap3 == 3 ) {
                m_weap3 = 0;
            }
            if ( m_weap3 == 0 ) {
                m_btnWeap3.setText( "Laser" );
            }
            if ( m_weap3 == 1 ) {
                m_btnWeap3.setText( "Blaster" );
            }
            if ( m_weap3 == 2 ) {
                m_btnWeap3.setText( "Torpedo" );
            }
            break;
        }
        break;

    default:
        break;
    }
}

void Game::onMouseMove( const SDL_MouseMotionEvent& event )
{
    const uint32_t y = viewportHeight() - event.y;
    const uint32_t x = (uint32_t)event.x;
    m_btnChangeFiltering.mouseMove( x, y );
    m_btnCustomizeReturn.mouseMove( x, y );
    m_btnCustomize.mouseMove( x, y );
    m_btnExit.mouseMove( x, y );
    m_btnGO.mouseMove( x, y );
    m_btnNextJet.mouseMove( x, y );
    m_btnNextMap.mouseMove( x, y );
    m_btnPrevJet.mouseMove( x, y );
    m_btnPrevMap.mouseMove( x, y );
    m_btnQuitMission.mouseMove( x, y );
    m_btnReturnToMainMenu.mouseMove( x, y );
    m_btnReturnToMissionSelection.mouseMove( x, y );
    m_btnSelectMissionCancel.mouseMove( x, y );
    m_btnSelectMission.mouseMove( x, y );
    m_btnStartMission.mouseMove( x, y );
    m_btnWeap1.mouseMove( x, y );
    m_btnWeap2.mouseMove( x, y );
    m_btnWeap3.mouseMove( x, y );
}
