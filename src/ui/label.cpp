#include <ui/label.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Label::Label( const Label::CreateInfo& ci )
: Widget{ ci.position, {}, ci.anchor }
, m_dataModel{ ci.dataModel }
, m_font{ ci.font }
, m_color{ ci.color }
{
    assert( ci.font );
    if ( ci.dataModel ) {
        m_currentIdx = m_dataModel->current();
        setText( m_dataModel->at( m_currentIdx ) );
    }
    else {
        setText( ci.text );
    }
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

DataModel* Label::dataModel() const
{
    return m_dataModel;
}


}
