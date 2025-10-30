#include "button.hpp"

#include <ui/property.hpp>
#include <engine/math.hpp>

namespace ui {

Button::Button( const CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .style = "button"_hash, .anchor = ci.anchor } }
, m_trigger{ ci.trigger }
{
    m_label = emplace_child<Label>( Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .position = ci.size * 0.5f, .anchor = Anchor::fCenter | Anchor::fMiddle, } );
    setTabOrder( ci.tabOrder );
}

void Button::trigger() const
{
    struct TriggerFire {
        void operator () ( std::monostate ) {}
        void operator () ( const std::function<void()>& f ) { assert( f ); f(); }
        void operator () ( Hash::value_type h ) { if ( h ) (*this)( g_uiProperty.gameCallback( h ) ); }
    };
    std::visit( TriggerFire{}, m_trigger );
}

void Button::setTrigger( std::function<void()>&& f )
{
    assert( f );
    m_trigger = std::move( f );
}

void Button::setText( std::u32string_view txt )
{
    assert( m_label );
    m_label->setText( txt );
}

EventProcessing Button::onMouseEvent( const MouseEvent& event )
{
    switch ( event.type ) {
    case MouseEvent::eMove:
        setFocused( testRect( event.position ) );
        return EventProcessing::eContinue;
    case MouseEvent::eClick:
        if ( !m_enabled ) { return EventProcessing::eContinue; }
        if ( !testRect( event.position ) ) { return EventProcessing::eContinue; }
        trigger();
        return EventProcessing::eStop;
    default:
        return EventProcessing::eContinue;
    }
}

EventProcessing Button::onAction( ui::Action action )
{
    if ( action.value == 0 ) { return EventProcessing::eContinue; }
    switch ( action.a ) {
    case ui::Action::eMenuConfirm:
        trigger();
        return EventProcessing::eStop;
    default:
        return EventProcessing::eContinue;
    }
}

}
