#include <ui/label.hpp>

#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Label::Label( const Label::CreateInfo& ci )
: Widget{ ci.position, ci.size, ci.anchor }
, m_dataModel{ ci.data ? g_uiProperty.dataModel( ci.data ) : nullptr }
, m_font{ g_uiProperty.font( ci.font ) }
, m_color{ g_uiProperty.color( ci.color ) }
, m_labelExtent{ ci.size }
{
    assert( m_font );
    if ( m_dataModel ) {
        m_revision = m_dataModel->revision();
        setText( m_dataModel->at( m_dataModel->current() ) );
    }
    else if ( ci.text ) {
        setText( g_uiProperty.localize( ci.text ) );
    }
}

void Label::render( RenderContext rctx ) const
{
    assert( !m_text.empty() );

    m_renderText.pushConstant.m_model = rctx.model;
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
    m_renderText = m_font->composeText( m_color, m_text, m_labelExtent );
    m_size = m_renderText.extent;
}

void Label::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto rev = m_dataModel->revision();
    if ( rev == m_revision ) { return; }
    m_revision = rev;
    setText( m_dataModel->at( m_dataModel->current() ) );
}

DataModel* Label::dataModel() const
{
    return m_dataModel;
}


}
