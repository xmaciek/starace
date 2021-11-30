#include "button.hpp"

#include "colors.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr glm::vec4 colorNormal = color::dodgerBlue;
static constexpr glm::vec4 colorHover = color::lightSkyBlue;
static constexpr glm::vec4 colorDisabled = color::lightSteelBlue;


Button::Button( std::u32string_view txt, Font* f, Texture texture )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    txt
    , f
    , Align::fCenter | Align::fMiddle
    , size() * 0.5f
    , color::white )
{
}

Button::Button( Font* f, Texture texture )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    f
    , Align::fCenter | Align::fMiddle
    , size() * 0.5f
    , color::white )
{
}

bool Button::isClicked( uint32_t x, uint32_t y ) const
{
    return m_enabled && testRect( glm::vec2{ x, y } );
}

void Button::render( RenderContext rctx ) const
{
    UIImage::render( rctx );
    rctx.model = glm::translate( rctx.model, glm::vec3{ m_position.x, m_position.y, 0.0f } );
    m_label.render( rctx );
}

void Button::updateColor()
{
    setColor( m_enabled ? ( m_mouseHover ? colorHover : colorNormal ) : colorDisabled );
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

void Button::setText( std::u32string_view txt )
{
    m_label.setText( txt );
}

bool Button::onMouseEvent( const MouseEvent& event )
{
    m_mouseHover = testRect( event );
    updateColor();
    return m_mouseHover;
}
