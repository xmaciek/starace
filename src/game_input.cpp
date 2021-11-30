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
    assert( m_audio );
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
            quit();
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

    case Screen::eCustomize: {
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

        constexpr auto weaponToString = []( int i ) -> std::u32string_view
        {
            using namespace std::string_view_literals;
            switch ( i ) {
            case 0:
                return U"Laser"sv;
            case 1:
                return U"Blaster"sv;
            case 2:
                return U"Torpedo"sv;
            }
            assert( !"invalid weapon id" );
            return U"invalid id"sv;
        };

        if ( m_btnWeap1.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_btnWeap1.setText( weaponToString( *++m_weap1 ) );
        }
        else if ( m_btnWeap2.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_btnWeap2.setText( weaponToString( *++m_weap2 ) );
        }
        else if ( m_btnWeap3.isClicked( x, y ) ) {
            m_audio->play( m_click );
            m_btnWeap3.setText( weaponToString( *++m_weap3 ) );
        }
    } break;

    default:
        break;
    }
}

void Game::onMouseMove( const SDL_MouseMotionEvent& event )
{
    const Widget::MouseEvent mouseEvent{ event.x, event.y };
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
    m_btnReturnToMissionSelection.onMouseEvent( mouseEvent );
    m_btnSelectMissionCancel.onMouseEvent( mouseEvent );
    m_btnSelectMission.onMouseEvent( mouseEvent );
    m_btnStartMission.onMouseEvent( mouseEvent );
    m_btnWeap1.onMouseEvent( mouseEvent );
    m_btnWeap2.onMouseEvent( mouseEvent );
    m_btnWeap3.onMouseEvent( mouseEvent );
}
