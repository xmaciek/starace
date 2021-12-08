#include "button.hpp"

#include "colors.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr glm::vec4 colorNormal = color::dodgerBlue;
static constexpr glm::vec4 colorHover = color::lightSkyBlue;
static constexpr glm::vec4 colorDisabled = color::lightSteelBlue;


Button::Button( std::u32string_view txt, Font* f, Texture texture, std::function<void()>&& onClick )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    txt
    , f
    , Anchor::fCenter | Anchor::fMiddle
    , size() * 0.5f
    , color::white )
, m_onClick{ onClick }
{
}

Button::Button( Font* f, Texture texture, std::function<void()>&& onClick )
: UIImage{ {}, { 192.0f, 48.0f }, colorNormal, texture }
, m_label(
    f
    , Anchor::fCenter | Anchor::fMiddle
    , size() * 0.5f
    , color::white )
, m_onClick{ onClick }
{
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
    switch ( event.index() ) {
    case 1:
        m_mouseHover = testRect( std::get<MouseMove>( event ) );
        updateColor();
        return m_mouseHover;
    case 2:
        if ( !m_enabled ) { return false; }
        if ( !testRect( std::get<MouseClick>( event ) ) ) { return false; }
        assert( m_onClick );
        m_onClick();
        m_mouseHover = false;
        updateColor();
        return true;
    default:
        return false;
    }
}
