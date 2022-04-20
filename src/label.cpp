#include "label.hpp"

#include "colors.hpp"

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Label::Label( DataModel* dataModel, const Font* f, const math::vec2& position )
: Widget{ position, {} }
, m_dataModel{ dataModel }
, m_font{ f }
, m_color{ color::white }
{
    assert( dataModel );
    assert( f );
    m_currentIdx = m_dataModel->current();
    setText( m_dataModel->at( m_currentIdx ) );
}

Label::Label( std::u32string_view s, const Font* f, const math::vec2& position, const math::vec4& color )
: Widget{ position, {} }
, m_font{ f }
, m_color{ color }
{
    assert( f );
    setText( s );
}

Label::Label( std::u32string_view s, const Font* f, Anchor a, const math::vec2& position, const math::vec4& color )
: Widget{ position, {}, a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
    setText( s );
}

Label::Label( std::u32string_view s, const Font* f, Anchor a, const math::vec4& color )
: Widget{ a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
    setText( s );
}

Label::Label( const Font* f, Anchor a, const math::vec2& position, const math::vec4& color )
: Widget{ position, {}, a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
}

Label::Label( const Font* f, Anchor a, const math::vec4& color )
: Widget{ a }
, m_font{ f }
, m_color{ color }
{
    assert( f );
}

void Label::render( RenderContext rctx ) const
{
    assert( m_font );
    assert( !m_text.empty() );
    const math::vec3 pos{ position() + offsetByAnchor(), 0.0f };

    m_renderText.second.m_model = math::translate( rctx.model, pos );
    m_renderText.second.m_view = rctx.view;
    m_renderText.second.m_projection = rctx.projection;
    rctx.renderer->push( m_renderText.first, &m_renderText.second );
}

void Label::setText( std::u32string_view str )
{
    setText( std::pmr::u32string{ str.begin(), str.end() } );
}

void Label::setText( std::pmr::u32string&& str )
{
    m_text = std::move( str );
    m_renderText = m_font->composeText( m_color, m_text );
    m_size = { m_font->textLength( m_text ), m_font->height() };
    m_textExtent = m_size;
}

void Label::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto idx = m_dataModel->current();
    if ( idx == m_currentIdx ) { return; }
    m_currentIdx = idx;
    setText( m_dataModel->at( idx ) );
}


}
