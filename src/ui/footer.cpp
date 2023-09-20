#include <ui/footer.hpp>

#include <ui/property.hpp>
#include <ui/font.hpp>
#include <ui/nineslice.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
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
}

void Footer::render( RenderContext rctx ) const
{
    NineSlice ns{ position(), size(), Anchor::fTop | Anchor::fLeft, SLICES };
    ns.render( rctx );

    std::u32string text;
    auto inputToText = []( auto i ) -> std::u32string_view
    {
        switch ( i ) {
            case Action::eMenuCancel: return U"[Esc]";
            case Action::eMenuApply: return U"[Space]";
            default:
                assert( !"TODO: more input to text" );
                return U"[]";
        }
    };
    for ( auto&& action : m_actions ) {
        if ( action.textId == 0 ) continue;
        text.append( inputToText( action.action ) );
        text.append( U" " );
        text.append( g_uiProperty.localize( action.textId ) );
        text.append( U"    " );
    };
    float textLength = g_uiProperty.fontMedium()->textLength( text );
    auto [ pushData, pushConstant ] = g_uiProperty.fontMedium()->composeText( math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, text );
    pushConstant.m_model = math::translate( rctx.model, math::vec3{ size().x - textLength, position().y + g_uiProperty.fontMedium()->height() * 0.618f, 0 } );
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
