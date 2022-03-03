#include "screen_win_loose.hpp"

#include "colors.hpp"
#include "utils.hpp"
#include "ui_property.hpp"

static constexpr Anchor c_textAnchor = Anchor::fCenter | Anchor::fMiddle;

ScreenWinLoose::ScreenWinLoose(
      math::vec4 color
    , Widget* rings
    , std::u32string_view title
    , std::function<void()>&& onContinue
)
: m_glow{ color }
, m_rings{ rings }
, m_title{ title, g_uiProperty.fontLarge(), c_textAnchor, {}, color::white }
, m_score{ U"Your score:", g_uiProperty.fontSmall(), c_textAnchor, {}, color::white }
, m_scoreValue{ U"N/A", g_uiProperty.fontSmall(), c_textAnchor, {}, color::white }
, m_continue{ U"Return", std::move( onContinue ) }
{
    assert( rings );
}

void ScreenWinLoose::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );
    m_title.render( rctx );
    m_score.render( rctx );
    m_scoreValue.render( rctx );
    m_continue.render( rctx );
}

void ScreenWinLoose::update( const UpdateContext& uctx )
{
    assert( m_rings );
    m_rings->update( uctx );
}

bool ScreenWinLoose::onMouseEvent( const MouseEvent& event )
{
    return m_continue.onMouseEvent( event );
}

void ScreenWinLoose::resize( math::vec2 s )
{
    setSize( s );
    m_glow.setSize( s );
    m_title.setPosition( { s.x * 0.5f, 96.0f } );
    m_score.setPosition( { s.x * 0.5f, 128.0f } );
    m_scoreValue.setPosition( { m_score.size().x + s.x * 0.5f, 128.0f } );
    m_continue.setPosition( { s.x * 0.5f - 96.0f, s.y * 0.85f } );
}

bool ScreenWinLoose::onAction( Action a )
{
    if ( !a.digital ) { return false; }
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuLeft:
    case GameAction::eMenuRight:
    case GameAction::eMenuUp:
    case GameAction::eMenuDown:
        m_continue.setFocused( true );
        return true;

    case GameAction::eMenuConfirm:
        if ( m_continue.isFocused() ) {
            m_continue.trigger();
            return true;
        }
    default:
        break;
    }
    return false;
}

void ScreenWinLoose::setScore( uint32_t v )
{
    m_scoreValue.setText( intToUTF32( v ) );
}
