#include "widget.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Widget::setPosition( glm::vec2 v )
{
    m_position = v;
}

void Widget::setSize( glm::vec2 v )
{
    m_size = v;
}

glm::vec2 Widget::position() const
{
    return m_position;
}

glm::vec2 Widget::size() const
{
    return m_size;
}

void Widget::setAnchor( Anchor a )
{
    m_anchor = a;
}

uint16_t Widget::tabOrder() const
{
    return m_tabOrder;
}

void Widget::setTabOrder( uint16_t v )
{
    m_tabOrder = v;
}

bool Widget::onMouseEvent( const MouseEvent& )
{
    return false;
}

void Widget::update( const UpdateContext& )
{
}

bool Widget::testRect( glm::vec2 p ) const
{
    const glm::vec2 pos = position() + offsetByAnchor();
    const glm::vec2 br = pos + size();
    return ( p.x >= pos.x )
        && ( p.y >= pos.y )
        && ( p.x < br.x )
        && ( p.y < br.y );
}

glm::vec2 Widget::offsetByAnchor() const
{
    glm::vec2 ret{};

    if ( m_anchor && Anchor::fCenter ) {
        ret.x = m_size.x * -0.5f;
    }
    else if ( m_anchor && Anchor::fRight ) {
        ret.x = -m_size.x;
    }
    // else ret.x = 0.0f;

    if ( m_anchor && Anchor::fMiddle ) {
        ret.y = m_size.y * -0.5f;
    }
    else if ( m_anchor && Anchor::fBottom ) {
        ret.y = -m_size.y;
    }
    // else ret.x = 0.0f;

    return ret;
}

void Layout::operator()( Widget** begin, Widget** end ) const
{
    glm::vec2 position = m_position;
    switch ( m_flow ) {
    case eHorizontal:
        for ( ; begin != end; ++begin ) {
            const glm::vec2 size = (*begin)->size();
            (*begin)->setPosition( position );
            position.x += size.x;
        }
        break;
    case eVertical:
        for ( ; begin != end; ++begin ) {
            const glm::vec2 size = (*begin)->size();
            (*begin)->setPosition( position );
            position.y += size.y;
        }
        break;
    default:
        assert( !"unhandled enum" );
        break;
    }
}
