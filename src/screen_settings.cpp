#include "screen_settings.hpp"

#include "colors.hpp"
#include "ui_property.hpp"

#include <cassert>

ScreenSettings::ScreenSettings(
    Widget* rings
    , std::function<void()>&& onReturn
) noexcept
: m_glow{ g_uiProperty.colorA() }
, m_rings{ rings }
, m_title{ U"Settings", g_uiProperty.fontMedium(), Anchor::fCenter | Anchor::fMiddle, color::white }
, m_btnReturn{ U"Return", Anchor::fCenter | Anchor::fMiddle, std::move( onReturn ) }
{
    assert( m_rings );
}

void ScreenSettings::resize( math::vec2 wh )
{
    m_glow.setSize( wh );
    m_rings->setSize( wh );
    m_title.setPosition( wh * math::vec2{ 0.5f, 0.1f } );
    m_btnReturn.setPosition( wh * math::vec2{ 0.5f, 0.9f } );
}

void ScreenSettings::update( const UpdateContext& uctx )
{
    m_rings->update( uctx );
}

void ScreenSettings::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );
    m_title.render( rctx );
    m_btnReturn.render( rctx );
}

void ScreenSettings::onAction( Action a )
{
    if ( !a.digital ) { return; }

    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuConfirm:
    case GameAction::eMenuCancel:
        m_btnReturn.trigger();
    default:
        break;
    }
}

bool ScreenSettings::onMouseEvent( const MouseEvent& event )
{
    return m_btnReturn.onMouseEvent( event );
}
