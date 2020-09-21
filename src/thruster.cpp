#include "thruster.hpp"

#include <algorithm>

Thruster::Thruster( double length, double radius )
: m_inner( 32, radius * 0.6 )
, m_outer( 32, radius )
{
    setLength( length );
}

void Thruster::setLength( double newLength )
{
    m_length = newLength;
    m_lengthShorter = newLength * 0.95f;
}

void Thruster::setColor( uint32_t num, float* colorData )
{
    if ( num > 3 ) {
        return;
    }
    std::copy( colorData, colorData + 4, m_color[ num ] );
}

void Thruster::update( const UpdateContext& updateContext )
{
    if ( m_wiggle < 0.066f ) {
        m_len = m_length;
    }
    else {
        m_len = m_lengthShorter;
    }
    m_wiggle += updateContext.deltaTime;
    if ( m_wiggle > 0.08f ) {
        m_wiggle = 0;
    }
}

void Thruster::drawAt( double x, double y, double z ) const
{
    glPushMatrix();
    glTranslated( x, y, z );

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( m_color[ 0 ] );
    glVertex3d( 0, 0, m_len );

    glColor4fv( m_color[ 1 ] );

    for ( size_t i = m_inner.segments() - 1; i > 0; i -= 1 ) {
        glVertex2d( m_inner.x( i ), m_inner.y( i ) );
    }
    glVertex2d( m_inner.x( m_inner.segments() - 1 ), m_inner.y( m_inner.segments() - 1 ) );
    glEnd();

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( m_color[ 2 ] );
    glVertex3d( 0, 0, m_len );

    glColor4fv( m_color[ 3 ] );
    for ( size_t i = m_outer.segments() - 1; i > 0; i -= 1 ) {
        glVertex2d( m_outer.x( i ), m_outer.y( i ) );
    }
    glVertex2d( m_outer.x( m_outer.segments() - 1 ), m_outer.y( m_outer.segments() - 1 ) );
    glEnd();

    glPopMatrix();
}
