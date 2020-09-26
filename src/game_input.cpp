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

void Game::onMouseClickLeft( int32_t x, int32_t y )
{
    y = viewportHeight() - y;
    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        if ( m_btnQuitMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eDead );
        }
        break;

    case Screen::eMainMenu:
        if ( m_btnSelectMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMissionSelection );
            break;
        }
        if ( m_btnExit.isClicked( x, y ) ) {
            playSound( m_click );
            m_isRunning = false;
            break;
        }
        if ( m_btnCustomize.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eCustomize );
            break;
        }
        break;

    case Screen::eMissionSelection:
        if ( m_btnStartMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eGameBriefing );
            break;
        }
        if ( m_btnReturnToMainMenu.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        if ( m_btnNextMap.isClicked( x, y ) ) {
            m_currentMap++;
            if ( m_currentMap == m_mapsContainer.size() - 1 ) {
                m_btnNextMap.setEnabled( false );
            }
            m_btnPrevMap.setEnabled( true );
            playSound( m_click );
            break;
        }
        if ( m_btnPrevMap.isClicked( x, y ) ) {
            m_currentMap--;
            if ( m_currentMap == 0 ) {
                m_btnPrevMap.setEnabled( false );
            }
            if ( m_mapsContainer.size() > 1 ) {
                m_btnNextMap.setEnabled( true );
            }
            playSound( m_click );
            break;
        }
        break;

    case Screen::eDead:
    case Screen::eWin:
        if ( m_btnReturnToMissionSelection.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMissionSelection );
        }
        break;

    case Screen::eGameBriefing:
        if ( m_btnGO.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eGame );
        }
        break;

    case Screen::eCustomize:
        if ( m_btnNextJet.isClicked( x, y ) ) {
            playSound( m_click );
            m_currentJet++;
            m_btnPrevJet.setEnabled( m_currentJet > 0 );
            m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
            reloadPreviewModel();
            break;
        }
        if ( m_btnPrevJet.isClicked( x, y ) ) {
            playSound( m_click );
            m_currentJet--;
            m_btnPrevJet.setEnabled( m_currentJet > 0 );
            m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
            reloadPreviewModel();
            break;
        }
        if ( m_btnCustomizeReturn.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        if ( m_btnWeap1.isClicked( x, y ) ) {
            playSound( m_click );
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
            playSound( m_click );
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
            playSound( m_click );
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

