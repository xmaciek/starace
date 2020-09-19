#include "thruster.hpp"

Thruster::Thruster( GLdouble Length, GLdouble Radiust )
: inner( 32, Radiust * 0.6 )
, outer( 32, Radiust )
{
    SetLength( Length );
}

void Thruster::SetLength( const GLdouble& newLength )
{
    length = newLength;
    length_shorter = newLength * 0.95f;
}

void Thruster::SetColor( GLuint Num, GLfloat* ColorData )
{
    if ( Num > 3 ) {
        return;
    }
    memcpy( color[ Num ], ColorData, sizeof( GLfloat ) * 4 );
}

void Thruster::Update()
{
    if ( wiggle < 3 ) {
        Len = length;
    }
    else {
        Len = length_shorter;
    }
    wiggle++;
    if ( wiggle > 5 ) {
        wiggle = 0;
    }
}

void Thruster::DrawAt( const GLdouble& X, const GLdouble& Y, const GLdouble& Z )
{
    glPushMatrix();
    glTranslated( X, Y, Z );

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( color[ 0 ] );
    glVertex3d( 0, 0, Len );

    glColor4fv( color[ 1 ] );

    for ( size_t i = inner.GetSegments() - 1; i > 0; i -= 1 ) {
        glVertex2d( inner.GetX( i ), inner.GetY( i ) );
    }
    glVertex2d( inner.GetX( inner.GetSegments() - 1 ), inner.GetY( inner.GetSegments() - 1 ) );
    glEnd();

    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( color[ 2 ] );
    glVertex3d( 0, 0, Len );

    glColor4fv( color[ 3 ] );
    for ( size_t i = outer.GetSegments() - 1; i > 0; i -= 1 ) {
        glVertex2d( outer.GetX( i ), outer.GetY( i ) );
    }
    glVertex2d( outer.GetX( outer.GetSegments() - 1 ), outer.GetY( outer.GetSegments() - 1 ) );
    glEnd();

    glPopMatrix();
}
