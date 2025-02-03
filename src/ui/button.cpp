#include <ui/button.hpp>

#include <ui/property.hpp>
#include <engine/math.hpp>

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

Button::Button( const CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .spriteArray = SLICES, .anchor = Anchor::fTop | Anchor::fLeft } }
, m_trigger{ ci.trigger }
{
    m_label = emplace_child<Label>( Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .position = ci.size * 0.5f, .anchor = Anchor::fCenter | Anchor::fMiddle, } );
    setTabOrder( ci.tabOrder );
}

void Button::trigger() const
{
    if ( m_trigger ) {
        auto tr = g_uiProperty.gameCallback( m_trigger );
        assert( tr );
        tr();
    }
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
