#include "button.hpp"

#include "colors.hpp"
#include "ui_property.hpp"

#include <engine/math.hpp>

static constexpr math::vec4 colorNormal = color::dodgerBlue;
static constexpr math::vec4 colorHover = color::lightSkyBlue;
static constexpr math::vec4 colorDisabled = color::lightSteelBlue;
static constexpr auto c_defaultAnchor = Anchor::fCenter | Anchor::fMiddle;
std::array<uint32_t, 9> c_slices = {
    ui::AtlasSprite::eTopLeft,
    ui::AtlasSprite::eTop,
    ui::AtlasSprite::eTopRight,
    ui::AtlasSprite::eLeft,
    ui::AtlasSprite::eMid,
    ui::AtlasSprite::eRight,
    ui::AtlasSprite::eBotLeft,
    ui::AtlasSprite::eBot,
    ui::AtlasSprite::eBotRight,
};

Button::Button( std::u32string_view txt, std::function<void()>&& onTrigger )
: NineSlice{ {}, { 192.0f, 48.0f }, colorNormal, c_defaultAnchor, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
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
: NineSlice{ {}, { 192.0f, 48.0f }, colorNormal, a, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
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
: NineSlice{ {}, { 192.0f, 48.0f }, colorNormal, c_defaultAnchor, g_uiProperty.atlas(), c_slices, g_uiProperty.atlasTexture() }
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

void Button::updateColor()
{
    setColor( m_enabled ? ( m_focused ? colorHover : colorNormal ) : colorDisabled );
}

void Button::setEnabled( bool b )
{
    m_enabled = b;
    updateColor();
}

bool Button::isEnabled() const
{
    return m_enabled;
}

void Button::setFocused( bool b )
{
    m_focused = b;
    updateColor();
}

bool Button::isFocused() const
{
    return m_focused;
}

void Button::setText( std::u32string_view txt )
{
    m_label.setText( txt );
}

bool Button::onMouseEvent( const MouseEvent& event )
{
    switch ( event.type ) {
    case MouseEvent::eMove:
        setFocused( testRect(  event.position ) );
        return m_focused;
    case MouseEvent::eClick:
        if ( !m_enabled ) { return false; }
        if ( !testRect( event.position ) ) { return false; }
        trigger();
        return true;
    default:
        return false;
    }
}
