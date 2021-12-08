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

bool Widget::onMouseEvent( const MouseEvent& )
{
    return false;
}

void Widget::update( const UpdateContext& )
{
}

bool Widget::testRect( glm::vec2 p ) const
{
    const glm::vec2 br = m_position + m_size;
    return ( p.x >= m_position.x )
        && ( p.y >= m_position.y )
        && ( p.x < br.x )
        && ( p.y < br.y );
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
