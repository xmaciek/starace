#include "label.hpp"

#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cassert>

constexpr bool operator && ( Align a, Align b ) noexcept
{
    using T = std::underlying_type_t<Align>;
    return ( static_cast<T>( a ) & static_cast<T>( b ) ) == static_cast<T>( b );
}

static glm::vec2 calculatePositionOffset( Font* f, std::u32string_view s, Align a )
{
    assert( f );
    glm::vec2 ret{};

    if ( a && Align::fCenter ) {
        ret += glm::vec2{ -0.5f * (float)f->textLength( s ), 0.0f };
    }
    else if ( a && Align::fRight ) {
        assert( !"todo" );
    }

    if ( a && Align::fMiddle ) {
        ret += glm::vec2{ 0.0f, 0.5f * (float)f->height() };
    }
    else if ( a && Align::fBottom ) {
        ret += glm::vec2{ 0.0f, f->height() };
    }

    return ret;
}

Label::Label( std::u32string_view s, Font* f, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
{
    assert( f );
    setPosition( position );
    setText( s );
}

Label::Label( std::u32string_view s, Font* f, Align a, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
, m_align( a )
{
    assert( f );
    setPosition( position );
    setText( s );
}

Label::Label( Font* f, Align a, const glm::vec2& position, const glm::vec4& color )
: m_font( f )
, m_color( color )
, m_align( a )
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

void Label::setText( std::u32string_view str )
{
    m_text = str;
    m_positionOffset = calculatePositionOffset( m_font, m_text, m_align );
    m_renderText = m_font->composeText( m_color, str );
    m_size = { m_font->textLength( m_text ), m_font->height() };
}
