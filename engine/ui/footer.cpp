#include "footer.hpp"

#include <ui/property.hpp>
#include <ui/font.hpp>
#include <ui/nineslice.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <memory_resource>
#include <string>

namespace ui {

Footer::Footer( const Footer::CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size }
{
    emplace_child<NineSlice>( NineSlice::CreateInfo{ .size = size(), .style = "button"_hash, .anchor = Anchor::fTop | Anchor::fLeft } );
    m_label = emplace_child<Label>( Label::CreateInfo{ .font = "medium"_hash, .position = math::vec2{ ci.size.x - 8.0f, ci.size.y * 0.5f - 2.0f }, .anchor = Anchor::fMiddle | Anchor::fRight } );
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
    m_label->setText( m_text );
}

void Footer::lockitChanged()
{
    refreshText();
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
