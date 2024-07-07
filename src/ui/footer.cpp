#include <ui/footer.hpp>

#include <ui/property.hpp>
#include <ui/font.hpp>
#include <ui/nineslice.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <memory_resource>
#include <string>

static constexpr std::array<ui::Atlas::hash_type, 9> SLICES = {
    "topLeft"_hash,
    "top"_hash,
    "topRight"_hash,
    "left"_hash,
    "mid"_hash,
    "right"_hash,
    "botLeft2"_hash,
    "bot"_hash,
    "botRight2"_hash,
};

namespace ui {

Footer::Footer( const Footer::CreateInfo& ci ) noexcept
{
    uint32_t idx = 0;
    for ( auto&& entry : ci.entries ) {
        assert( idx < m_actions.size() );
        if ( idx == m_actions.size() ) break;
        m_actions[ idx++ ] = ActionInfo{ entry.action, entry.textId, entry.triggerId };
    }

    refreshText();
}

void Footer::refreshText()
{
    m_text.clear();
    for ( auto&& action : m_actions ) {
        if ( action.textId == 0 ) continue;
        m_text.push_back( (char32_t)action.action );
        m_text.append( U" " );
        m_text.append( g_uiProperty.localize( action.textId ) );
        m_text.append( U"    " );
    };
}

void Footer::render( RenderContext rctx ) const
{
    NineSlice{ NineSlice::CreateInfo{ position(), size(), SLICES, Anchor::fTop | Anchor::fLeft } }.render( rctx );
    const Font* font = g_uiProperty.fontMedium();
    float mid = size().y * 0.5f - font->height() * 0.618f;
    auto [ pushData, pushConstant, extent ] = font->composeText( math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, m_text, size() );
    pushConstant.m_model = math::translate( rctx.model, math::vec3{ size().x - extent.x, position().y + mid, 0 } );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    rctx.renderer->push( pushData, &pushConstant );
}

EventProcessing Footer::onMouseEvent( const MouseEvent& )
{
    return {};
}

EventProcessing Footer::onAction( ui::Action a )
{
    for ( auto&& action : m_actions ) {
        if ( action.triggerId == 0 ) continue;
        if ( action.action != a.a ) continue;
        auto callback = g_uiProperty.gameCallback( action.triggerId );
        assert( callback );
        callback();
        return EventProcessing::eStop;
    }

    return EventProcessing::eContinue;
}

}
