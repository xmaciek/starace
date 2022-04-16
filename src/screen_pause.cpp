#include "screen_pause.hpp"

#include "colors.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"

ScreenPause::ScreenPause(
      Widget* rings
    , std::function<void()>&& onUnpause
    , std::function<void()>&& onResume
    , std::function<void()>&& onExit
) noexcept
: m_glow{ color::dodgerBlue }
, m_rings{ rings }
, m_resume{ ui::loc::resume, Anchor::fLeft | Anchor::fMiddle, std::move( onResume ) }
, m_exit{ ui::loc::return2, Anchor::fRight | Anchor::fMiddle, std::move( onExit ) }
, m_pause{ ui::loc::paused, g_uiProperty.fontMedium(), Anchor::fCenter | Anchor::fMiddle, {}, color::yellow }
, m_unpause{ std::move( onUnpause ) }
{
    assert( rings );
    assert( m_unpause );
}

void ScreenPause::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );
    m_pause.render( rctx );
    m_exit.render( rctx );
    m_resume.render( rctx );
}

void ScreenPause::update( const UpdateContext& uctx )
{
    assert( m_rings );
    m_rings->update( uctx );
}

void ScreenPause::resize( math::vec2 wh )
{
    m_glow.setSize( wh );
    assert( m_rings );
    m_rings->setSize( wh );
    m_pause.setPosition( wh * math::vec2{ 0.5f, 0.1f } );
    m_exit.setPosition( wh * math::vec2{ 0.45f, 0.7f } );
    m_resume.setPosition( wh * math::vec2{ 0.55f, 0.7f } );
}

bool ScreenPause::onAction( Action a )
{
    if ( !a.digital ) { return false; }
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuLeft:
        m_currentTab = 0;
        m_resume.setFocused( false );
        m_exit.setFocused( true );
        break;
    case GameAction::eMenuRight:
        m_currentTab = 1;
        m_resume.setFocused( true );
        m_exit.setFocused( false );
        break;
    case GameAction::eMenuConfirm:
        switch ( m_currentTab ) {
        case 0: m_exit.trigger(); break;
        case 1: m_resume.trigger(); break;
        }
        break;
    case GameAction::eGamePause:
        m_currentTab = 1;
        assert( m_unpause );
        m_unpause();
        break;
    default:
        return false;
    }
    return true;
}

bool ScreenPause::onMouseEvent( const MouseEvent& event )
{
    if ( m_exit.onMouseEvent( event ) ) {
        m_currentTab = 0;
        return true;
    }
    if ( m_resume.onMouseEvent( event ) ) {
        m_currentTab = 1;
        return true;
    }
    m_currentTab = c_invalidTabOrder;
    return false;
}
