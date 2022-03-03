#include "button.hpp"

#include "colors.hpp"
#include "ui_property.hpp"

#include <engine/math.hpp>

static constexpr math::vec4 colorNormal = color::dodgerBlue;
static constexpr math::vec4 colorHover = color::lightSkyBlue;
static constexpr math::vec4 colorDisabled = color::lightSteelBlue;


Button::Button( std::u32string_view txt, std::function<void()>&& onTrigger )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, g_uiProperty.buttonTexture() }
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
: UIImage{ a, { 192.0f, 48.0f }, colorNormal, g_uiProperty.buttonTexture() }
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
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, g_uiProperty.buttonTexture() }
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
    UIImage::render( rctx );
    const math::vec2 pos = position() + offsetByAnchor() + size() * 0.5f;
    rctx.model = math::translate( rctx.model, math::vec3{ pos.x, pos.y, 0.0f } );
    m_label.render( rctx );
}

void Button::trigger() const
{
    assert( m_onTrigger );
    m_onTrigger();
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
    switch ( event.index() ) {
    case 1:
        setFocused( testRect( std::get<MouseMove>( event ) ) );
        return m_focused;
    case 2:
        if ( !m_enabled ) { return false; }
        if ( !testRect( std::get<MouseClick>( event ) ) ) { return false; }
        trigger();
        return true;
    default:
        return false;
    }
}
