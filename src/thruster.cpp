#include "thruster.hpp"

Thruster::Thruster( GLdouble length, GLdouble radius )
: m_inner( 32, radius * 0.6 )
, m_outer( 32, radius )
{
    setLength( length );
}

void Thruster::setLength( GLdouble newLength )
{
    m_length = newLength;
    m_lengthShorter = newLength * 0.95f;
}

void Thruster::setColor( GLuint num, GLfloat* colorData )
{
    if ( num > 3 ) {
        return;
    }
    memcpy( m_color[ num ], colorData, sizeof( GLfloat ) * 4 );
}

void Thruster::update()
{
    if ( m_wiggle < 3 ) {
        m_len = m_length;
    }
    else {
        m_len = m_lengthShorter;
    }
    m_wiggle++;
    if ( m_wiggle > 5 ) {
        m_wiggle = 0;
    }
}

void Thruster::drawAt( GLdouble x, GLdouble y, GLdouble z )
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
