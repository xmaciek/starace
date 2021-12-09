#include "label.hpp"

#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cassert>

static glm::vec2 positionAnchor( glm::vec2 sizeExtent, Anchor a )
{
    glm::vec2 ret{};

    if ( a && Anchor::fCenter ) {
        ret.x = sizeExtent.x * -0.5f;
    }
    else if ( a && Anchor::fRight ) {
        ret.x = -sizeExtent.x;
    }
    // else ret.x = 0.0f;

    if ( a && Anchor::fMiddle ) {
        ret.y = sizeExtent.y *  0.5f;
    }
    else if ( a && Anchor::fTop ) {
        ret.y = sizeExtent.y;
    }
    // else ret.x = 0.0f;

    return ret;
}

Label::Label( std::u32string_view s, Font* f, const glm::vec2& position, const glm::vec4& color )
: Widget{ position, {} }
, m_font{ f }
, m_color{ color }
{
    assert( f );
    setText( s );
}

Label::Label( std::u32string_view s, Font* f, Anchor a, const glm::vec2& position, const glm::vec4& color )
: Widget{ position, {}, a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
    setText( s );
}

Label::Label( Font* f, Anchor a, const glm::vec2& position, const glm::vec4& color )
: Widget{ position, {}, a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
}

void Label::render( RenderContext rctx ) const
{
    assert( m_font );
    const glm::vec3 pos{ m_position + positionAnchor( m_textExtent, m_anchor ), 0.0f };

    m_renderText.second.m_model = glm::translate( rctx.model, pos );
    m_renderText.second.m_view = rctx.view;
    m_renderText.second.m_projection = rctx.projection;
    rctx.renderer->push( &m_renderText.first, &m_renderText.second );
}

void Label::setText( std::u32string_view str )
{
    m_text = str;
    m_renderText = m_font->composeText( m_color, str );
    m_size = { m_font->textLength( m_text ), m_font->height() };
    m_textExtent = m_size;
}
