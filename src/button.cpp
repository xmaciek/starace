#include "button.hpp"

#include "colors.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr glm::vec4 colorNormal = color::dodgerBlue;
static constexpr glm::vec4 colorHover = color::lightSkyBlue;
static constexpr glm::vec4 colorDisabled = color::lightSteelBlue;


Button::Button( std::u32string_view txt, Font* f, Texture texture, std::function<void()>&& onTrigger )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    txt
    , f
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
}

Button::Button( std::u32string_view txt, Font* f, Texture texture, Anchor a, std::function<void()>&& onTrigger )
: UIImage{ a, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    txt
    , f
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
}

Button::Button( Font* f, Texture texture, std::function<void()>&& onTrigger )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    f
    , Anchor::fCenter | Anchor::fMiddle
    , {}
    , color::white )
, m_onTrigger{ onTrigger }
{
}

void Button::render( RenderContext rctx ) const
{
    UIImage::render( rctx );
    const glm::vec2 pos = position() + offsetByAnchor() + size() * 0.5f;
    rctx.model = glm::translate( rctx.model, glm::vec3{ pos.x, pos.y, 0.0f } );
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
