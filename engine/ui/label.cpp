#include <ui/label.hpp>

#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <algorithm>
#include <ranges>

namespace ui {

Label::Label( const Label::CreateInfo& ci )
: Widget{ ci.position, ci.size, ci.anchor }
, m_dataModel{ ci.data ? g_uiProperty.dataModel( ci.data ) : nullptr }
, m_font{ g_uiProperty.font( ci.font ) }
, m_color{ g_uiProperty.color( ci.color ) }
, m_labelExtent{ ci.size }
, m_locText{ ci.text }
{
    assert( m_font );
    if ( m_dataModel ) {
        m_revision = m_dataModel->revision();
        setText( m_dataModel->data( m_dataModel->current() ).visit( GetString{} ) );
    }
    else if ( m_locText ) {
        setText( g_uiProperty.localize( m_locText ) );
    }
}

void Label::render( const RenderContext& rctx ) const
{
    if ( m_renderText.pushData.m_instanceCount == 0 ) [[unlikely]] return;
    using PushConstant = PushConstant<Pipeline::eSpriteSequence>;
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };
    RenderInfo ri = m_renderText.pushData;
    ri.m_uniforms.emplace_back( pushConstant );
    ri.m_uniforms.emplace_back();

    auto renderChunk = [&ri, &rctx]( std::span<const PushConstant::Instance> chunk )
    {
        ri.m_uniforms.back() = chunk;
        ri.m_uniforms.back().sizeHint = sizeof( PushConstant::Instance ) * PushConstant::INSTANCES;
        ri.m_instanceCount = (uint32_t)chunk.size();
        rctx.renderer->render( ri );
    };
    auto chunks = m_renderText.data | std::views::chunk( pushConstant.INSTANCES );
    std::ranges::for_each( chunks, renderChunk );
}

void Label::refreshInput()
{
    Widget::refreshInput();
    if ( m_hasActions ) {
        m_renderText = m_font->composeText( m_text, m_labelExtent );
        m_size = m_renderText.extent;
    }
}

void Label::lockitChanged()
{
    Widget::lockitChanged();
    if ( m_dataModel ) {
        m_revision = m_dataModel->revision();
        setText( m_dataModel->data( m_dataModel->current() ).visit( GetString{} ) );
    }
    else if ( m_locText ) {
        setText( g_uiProperty.localize( m_locText ) );
    }
}

void Label::setText( std::u32string_view str )
{
    setText( std::pmr::u32string{ str.begin(), str.end() } );
}

void Label::setText( std::pmr::u32string&& str )
{
    m_text = std::move( str );
    auto isAction = []( char32_t c )
    {
        return c >= (char32_t)Action::base && c < (char32_t)Action::end;
    };
    m_hasActions = std::ranges::find_if( m_text, isAction ) != m_text.end();
    m_renderText = m_font->composeText( m_text, m_labelExtent );
    m_size = m_renderText.extent;
}

void Label::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto rev = m_dataModel->revision();
    if ( rev == m_revision ) { return; }
    m_revision = rev;
    setText( m_dataModel->data( m_dataModel->current() ).visit( GetString{} ) );
}

DataModel* Label::dataModel() const
{
    return m_dataModel;
}


}
