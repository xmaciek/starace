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
    auto inputToText = []( auto i ) -> std::pmr::u32string
    {
        switch ( i ) {
        // TODO action to actuator, then localize
        case Action::eMenuCancel: return g_uiProperty.localize( "input.escape"_hash );
        case Action::eMenuApply: return g_uiProperty.localize( "input.space"_hash );
        case Action::eMenuConfirm: return g_uiProperty.localize( "input.enter"_hash );
        default:
            assert( !"TODO: more input to text" );
            return U"[]";
        }
    };
    m_text.clear();
    for ( auto&& action : m_actions ) {
        if ( action.textId == 0 ) continue;
        m_text.append( inputToText( action.action ) );
        m_text.append( U" " );
        m_text.append( g_uiProperty.localize( action.textId ) );
        m_text.append( U"    " );
    };
    m_textLength = g_uiProperty.fontMedium()->textLength( m_text );
}

void Footer::render( RenderContext rctx ) const
{
    NineSlice{ NineSlice::CreateInfo{ position(), size(), SLICES, Anchor::fTop | Anchor::fLeft } }.render( rctx );

    auto [ pushData, pushConstant ] = g_uiProperty.fontMedium()->composeText( math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, m_text );
    pushConstant.m_model = math::translate( rctx.model, math::vec3{ size().x - m_textLength, position().y + g_uiProperty.fontMedium()->height() * 0.618f, 0 } );
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
