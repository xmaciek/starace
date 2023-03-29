#include <ui/button.hpp>

#include <ui/property.hpp>
#include <engine/math.hpp>

static constexpr auto c_defaultAnchor = Anchor::fTop | Anchor::fLeft;
std::array<uint32_t, 9> c_slices = {
    ui::AtlasSprite::eTopLeft,
    ui::AtlasSprite::eTop,
    ui::AtlasSprite::eTopRight,
    ui::AtlasSprite::eLeft,
    ui::AtlasSprite::eMid,
    ui::AtlasSprite::eRight,
    ui::AtlasSprite::eBotLeft2,
    ui::AtlasSprite::eBot,
    ui::AtlasSprite::eBotRight2,
};

namespace ui {

Button::Button( const CreateInfo& ci ) noexcept
: NineSlice{ ci.position, ci.size, c_defaultAnchor, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
, m_label{ Label::CreateInfo{ .text = ci.text, .font = g_uiProperty.fontSmall(), .anchor = Anchor::fCenter | Anchor::fMiddle } }
, m_onTrigger{ ci.trigger }
{
    setTabOrder( ci.tabOrder );
}

void Button::render( RenderContext rctx ) const
{
    if ( isFocused() ) {
        rctx.colorMain = rctx.colorFocus;
    }
    NineSlice::render( rctx );
    const math::vec2 pos = position() + offsetByAnchor() + size() * 0.5f;
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
