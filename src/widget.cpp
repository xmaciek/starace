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

void Widget::update( const UpdateContext& )
{
}

void Layout::render( RenderContext rctx ) const
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ m_position, 0.0f } );
    for ( size_type i = 0; i < m_count; ++i ) {
        m_widgets[ i ]->render( rctx );
    }
}

void Layout::update( const UpdateContext& uctx )
{
    if ( m_count == 0 ) { return; }

    for ( size_type i = 0; i < m_count; ++i ) {
        m_widgets[ i ]->update( uctx );
    }

    glm::vec2 pos{};
    for ( size_type i = 0; i < m_count - 1; ++i ) {
        const glm::vec2 wpos = m_widgets[ i ]->size();
        pos += wpos;
        Widget* w = m_widgets[ i + 1 ];
        switch ( m_flow ) {
        case eHorizontal:
            pos.y = w->position().y;
            w->setPosition( pos );
            break;
        case eVertical:
            pos.x = w->position().x;
            w->setPosition( pos );
            break;
        }
    }
    const Widget* w = m_widgets[ m_count - 1 ];
    setSize( w->position() + w->size() );
}

void Layout::add( Widget* w )
{
    assert( m_count < m_widgets.size() );
    m_widgets[ m_count ] = w;
    m_count++;
}
