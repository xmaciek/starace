#include "button.hpp"

#include "colors.hpp"
#include "ui_property.hpp"
#include "game_action.hpp"

#include <engine/math.hpp>

static constexpr auto c_defaultAnchor = Anchor::fCenter | Anchor::fMiddle;
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

Button::Button( std::u32string_view txt, std::function<void()>&& onTrigger )
: NineSlice{ {}, { 192.0f, 48.0f }, c_defaultAnchor, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
, m_label(
    txt
    , g_uiProperty.fontSmall()
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
}

Button::Button( std::u32string_view txt, Anchor a, std::function<void()>&& onTrigger )
: NineSlice{ {}, { 192.0f, 48.0f }, a, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
, m_label(
    txt
    , g_uiProperty.fontSmall()
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
}

Button::Button( std::function<void()>&& onTrigger )
: NineSlice{ {}, { 192.0f, 48.0f }, c_defaultAnchor, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
, m_label(
    g_uiProperty.fontSmall()
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
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

MouseEvent::Processing Button::onMouseEvent( const MouseEvent& event )
{
    switch ( event.type ) {
    case MouseEvent::eMove:
        setFocused( testRect( event.position ) );
        return m_focused ? MouseEvent::eStop : MouseEvent::eContinue;
    case MouseEvent::eClick:
        if ( !m_enabled ) { return MouseEvent::eContinue; }
        if ( !testRect( event.position ) ) { return MouseEvent::eContinue; }
        trigger();
        return MouseEvent::eStop;
    default:
        return MouseEvent::eContinue;
    }
}

bool Button::onAction( Action a )
{
    if ( !a.digital ) {
        return false;
    }
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuConfirm:
        trigger();
        return true;
    default:
        return false;
    }
}

}
