#include "screen_win_loose.hpp"

#include "colors.hpp"

static constexpr Anchor c_textAnchor = Anchor::fCenter | Anchor::fMiddle;

ScreenWinLoose::ScreenWinLoose(
    Texture glow
    , Texture button
    , glm::vec4 color
    , Font* fontSmall
    , Font* fontLarge
    , std::u32string_view title
    , std::function<void()>&& onContinue
)
: m_glow{ glow, color }
, m_title{ title, fontLarge, c_textAnchor, {}, color::white }
, m_score{ U"Your score:", fontSmall, c_textAnchor, {}, color::white }
, m_scoreValue{ U"N/A", fontSmall, c_textAnchor, {}, color::white }
, m_continue{ U"Return", fontSmall, button, std::move( onContinue ) }
{}

void ScreenWinLoose::render( RenderContext rctx ) const
{
    m_glow.render( rctx );
    m_title.render( rctx );
    m_score.render( rctx );
    m_scoreValue.render( rctx );
    m_continue.render( rctx );
}

bool ScreenWinLoose::onMouseEvent( const MouseEvent& event )
{
    return m_continue.onMouseEvent( event );
}

void ScreenWinLoose::resize( glm::vec2 s )
{
    setSize( s );
    m_glow.setSize( s );
    m_title.setPosition( { s.x * 0.5f, 96.0f } );
    m_score.setPosition( { s.x * 0.5f, 128.0f } );
    m_scoreValue.setPosition( { m_score.size().x + s.x * 0.5f, 128.0f } );
    m_continue.setPosition( { s.x * 0.5f - 96.0f, s.y * 0.85f } );
}
