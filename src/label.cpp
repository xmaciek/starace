#include "label.hpp"

#include <renderer/renderer.hpp>

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

static glm::vec2 calculatePositionOffset( Font* f, std::string_view s, Label::HAlign ah, Label::VAlign av )
{
    assert( f );
    glm::vec2 ret{};

    switch ( ah ) {
    case Label::HAlign::eLeft:
        break;
    case Label::HAlign::eCenter:
        ret += glm::vec2{ -0.5f * (float)f->textLength( s ), 0.0f };
        break;
    }

    switch ( av ) {
    case Label::VAlign::eTop:
        break;
    case Label::VAlign::eMiddle:
        ret += glm::vec2{ 0.0f, 0.5f * (float)f->height() };
        break;
    case Label::VAlign::eBottom:
        ret += glm::vec2{ 0.0f, f->height() };
        break;
    }

    return ret;
}

Label::Label( std::string_view s, Font* f, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
{
    assert( f );
    setPosition( position );
    setText( s );
}

Label::Label( std::string_view s, HAlign ah, VAlign av, Font* f, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
, m_halign( ah )
, m_valign( av )
{
    assert( f );
    setPosition( position );
    setText( s );
}

Label::Label( Font* f, HAlign ah, VAlign av, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
, m_halign( ah )
, m_valign( av )
{
    assert( f );
    setPosition( position );
}

void Label::render( RenderContext rctx ) const
{
    assert( m_font );
    const glm::vec3 pos{ m_position + m_positionOffset, 0.0f };

    m_renderText.second.m_model = glm::translate( rctx.model, pos );
    m_renderText.second.m_view = rctx.view;
    m_renderText.second.m_projection = rctx.projection;
    rctx.renderer->push( &m_renderText.first, &m_renderText.second );
}

void Label::setText( std::string_view str )
{
    m_positionOffset = calculatePositionOffset( m_font, str, m_halign, m_valign );
    m_text = str;
    m_renderText = m_font->composeText( m_color, str );
    m_size = { m_font->textLength( m_text ), m_font->height() };
}
