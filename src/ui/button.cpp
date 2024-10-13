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
, m_label{ Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .position = ci.size * 0.5f, .anchor = Anchor::fCenter | Anchor::fMiddle } }
, m_onTrigger{ g_uiProperty.gameCallback( ci.trigger ) }
{
    setTabOrder( ci.tabOrder );
}

void Button::render( RenderContext rctx ) const
{
    if ( isFocused() ) {
        rctx.colorMain = rctx.colorFocus;
    }
    NineSlice::render( rctx );
    const math::vec2 pos = position() + offsetByAnchor();
    rctx.model = math::translate( rctx.model, math::vec3{ pos.x, pos.y, 0.0f } );
    m_label.render( rctx );
}

void Button::trigger() const
{
    assert( m_onTrigger );
    m_onTrigger();
}

void Button::setTrigger( std::function<void()> t )
{
    m_onTrigger = t;
}

void Button::setText( std::u32string_view txt )
{
    m_label.setText( txt );
}

EventProcessing Button::onMouseEvent( const MouseEvent& event )
{
    switch ( event.type ) {
    case MouseEvent::eMove:
        setFocused( testRect( event.position ) );
        return m_focused ? EventProcessing::eStop : EventProcessing::eContinue;
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
