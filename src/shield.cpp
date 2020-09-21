#include "shield.hpp"

Shield::Shield( double radiusA, double radiusB )
: m_radius( radiusA )
, m_circle( 6, radiusB )
{
}

void Shield::update( const UpdateContext& updateContext )
{
    m_rotAngle += updateContext.deltaTime;
    if ( m_rotAngle >= 360 ) {
        m_rotAngle -= 360;
    }
}

void Shield::draw() const
{
    glPushMatrix();
    glRotated( m_rotAngle, 0, 1, 0 );
    for ( uint32_t i = 0; i < 8; i++ ) {
        glBegin( GL_LINE_LOOP );
        for ( uint32_t j = 0; j < m_circle.segments(); j++ ) {
            glVertex3d( m_circle.x( j ), m_circle.y( j ), m_radius );
        }
        glEnd();
        glBegin( GL_LINES );
        glVertex3d( 0, 0, 0 );
        glVertex3d( 0, 0, m_radius );
        glEnd();
        glRotated( 45, 0, 1, 0 );
    }
    glPushMatrix();
    glRotated( m_rotAngle, 0, 0, 1 );
    for ( uint32_t i = 0; i < 8; i++ ) {
        glBegin( GL_LINE_LOOP );
        for ( uint32_t j = 0; j < m_circle.segments(); j++ ) {
            glVertex3d( m_circle.x( j ), m_circle.y( j ), m_radius );
        }
        glEnd();
        glBegin( GL_LINES );
        glVertex3d( 0, 0, 0 );
        glVertex3d( 0, 0, m_radius );
        glEnd();
        glRotated( 45, 0, 1, 0 );
    }
    glPushMatrix();
    glRotated( m_rotAngle, 1, 0, 0 );
    for ( uint32_t i = 0; i < 8; i++ ) {
        glBegin( GL_LINE_LOOP );
        for ( uint32_t j = 0; j < m_circle.segments(); j++ ) {
            glVertex3d( m_circle.x( j ), m_circle.y( j ), m_radius );
        }
        glEnd();
        glBegin( GL_LINES );
        glVertex3d( 0, 0, 0 );
        glVertex3d( 0, 0, m_radius );
        glEnd();
        glRotated( 45, 0, 1, 0 );
    }
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}
