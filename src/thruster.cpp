#include "thruster.hpp"

Thruster::Thruster( GLdouble length, GLdouble radius )
: m_inner( 32, radius * 0.6 )
, m_outer( 32, radius )
{
    SetLength( length );
}

void Thruster::SetLength( GLdouble newLength )
{
    m_length = newLength;
    m_lengthShorter = newLength * 0.95f;
}

void Thruster::SetColor( GLuint Num, GLfloat* ColorData )
{
    if ( Num > 3 ) {
        return;
    }
    memcpy( m_color[ Num ], ColorData, sizeof( GLfloat ) * 4 );
}

void Thruster::Update()
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

void Thruster::DrawAt( GLdouble X, GLdouble Y, GLdouble Z )
{
    glPushMatrix();
    glTranslated( X, Y, Z );

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( m_color[ 0 ] );
    glVertex3d( 0, 0, m_len );

    glColor4fv( m_color[ 1 ] );

    for ( size_t i = m_inner.GetSegments() - 1; i > 0; i -= 1 ) {
        glVertex2d( m_inner.GetX( i ), m_inner.GetY( i ) );
    }
    glVertex2d( m_inner.GetX( m_inner.GetSegments() - 1 ), m_inner.GetY( m_inner.GetSegments() - 1 ) );
    glEnd();

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( m_color[ 2 ] );
    glVertex3d( 0, 0, m_len );

    glColor4fv( m_color[ 3 ] );
    for ( size_t i = m_outer.GetSegments() - 1; i > 0; i -= 1 ) {
        glVertex2d( m_outer.GetX( i ), m_outer.GetY( i ) );
    }
    glVertex2d( m_outer.GetX( m_outer.GetSegments() - 1 ), m_outer.GetY( m_outer.GetSegments() - 1 ) );
    glEnd();

    glPopMatrix();
}
