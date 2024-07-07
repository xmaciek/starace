#include <ui/label.hpp>

#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Label::Label( const Label::CreateInfo& ci )
: Widget{ ci.position, {}, ci.anchor }
, m_dataModel{ ci.data ? g_uiProperty.dataModel( ci.data ) : nullptr }
, m_font{ g_uiProperty.font( ci.font ) }
, m_color{ g_uiProperty.color( ci.color ) }
{
    assert( m_font );
    if ( m_dataModel ) {
        m_currentIdx = m_dataModel->current();
        setText( m_dataModel->at( m_currentIdx ) );
    }
    else if ( ci.text ) {
        setText( g_uiProperty.localize( ci.text ) );
    }
}

void Label::render( RenderContext rctx ) const
{
    assert( !m_text.empty() );
    const math::vec3 pos{ position() + offsetByAnchor(), 0.0f };

    m_renderText.pushConstant.m_model = math::translate( rctx.model, pos );
    m_renderText.pushConstant.m_view = rctx.view;
    m_renderText.pushConstant.m_projection = rctx.projection;
    rctx.renderer->push( m_renderText.pushData, &m_renderText.pushConstant );
}

void Label::setText( std::u32string_view str )
{
    setText( std::pmr::u32string{ str.begin(), str.end() } );
}

void Label::setText( std::pmr::u32string&& str )
{
    assert( !str.empty() );
    m_text = std::move( str );
    m_renderText = m_font->composeText( m_color, m_text );
    m_size = m_renderText.extent;
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
