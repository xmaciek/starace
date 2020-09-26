#include "label.hpp"

#include <cassert>

static glm::vec2 calculatePositionOffset( Font* f, std::string_view s, Label::HAlign ah, Label::VAlign av )
{
    assert( f );
    glm::vec2 ret{};

    switch ( ah ) {
    case Label::HAlign::eLeft:
        break;
    case Label::HAlign::eCenter:
        ret += glm::vec2{ -0.5f * f->textLength( s ), 0.0f };
        break;
    }

    switch ( av ) {
    case Label::VAlign::eBottom:
        break;
    case Label::VAlign::eMiddle:
        ret += glm::vec2{ 0.0f, -0.6f * f->height() };
        break;
    }

    return ret;
}

Label::Label( std::string_view s, Font* f, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_text( s )
, m_color( color )
, m_position( position )
{
    assert( f );
    m_positionOffset = calculatePositionOffset( f, s, m_halign, m_valign );
}

Label::Label( std::string_view s, HAlign ah, VAlign av, Font* f, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_text( s )
, m_color( color )
, m_position( position )
, m_halign( ah )
, m_valign( av )
{
    assert( f );
    m_positionOffset = calculatePositionOffset( f, s, ah, av );
}

Label::Label( Font* f, HAlign ah, VAlign av, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
, m_position( position )
, m_halign( ah )
, m_valign( av )
{
    assert( f );
}

void Label::render( RenderContext rctx ) const
{
    assert( m_font );
    const glm::vec2 pos = m_position + m_positionOffset;
    m_font->renderText( rctx, m_color, pos.x, pos.y, m_text );
}

void Label::setText( std::string_view str )
{
    m_positionOffset = calculatePositionOffset( m_font, str, m_halign, m_valign );
    m_text = str;
}
