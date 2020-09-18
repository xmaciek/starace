#include "Thruster.h"

Thruster::~Thruster()
{
    cout << "+-+ Deleting Thruster\n";
    if ( inner != NULL ) {
        cout << "|  ";
        delete inner;
        inner = NULL;
    }

    if ( outer != NULL ) {
        cout << "|  ";
        delete outer;
        outer = NULL;
    }
}

Thruster::Thruster( GLdouble Length, GLdouble Radiust )
{
    SetLength( Length );
    radiust = Radiust;
    inner = new Circle( 32, radiust * 0.6f );
    outer = new Circle( 32, radiust );
    update_i = 0;
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
    if ( update_i < 3 ) {
        Len = length;
    }
    else {
        Len = length_shorter;
    }
    update_i++;
    if ( update_i > 5 ) {
        update_i = 0;
    }
}

void Thruster::DrawAt( const GLdouble& X, const GLdouble& Y, const GLdouble& Z )
{
    glPushMatrix();
    glTranslated( X, Y, Z );

    /* rysowanie inner */
    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( color[ 0 ] );
    glVertex3d( 0, 0, Len );

    glColor4fv( color[ 1 ] );

    for ( drawing_i = inner->GetSegments() - 1; drawing_i > 0; drawing_i -= 1 ) {
        glVertex2d( inner->GetX( drawing_i ), inner->GetY( drawing_i ) );
    }
    glVertex2d( inner->GetX( inner->GetSegments() - 1 ), inner->GetY( inner->GetSegments() - 1 ) );
    glEnd();

    /* rysowanie outer */
    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( color[ 2 ] );
    glVertex3d( 0, 0, Len );

    glColor4fv( color[ 3 ] );
    for ( drawing_i = outer->GetSegments() - 1; drawing_i > 0; drawing_i -= 1 ) {
        glVertex2d( outer->GetX( drawing_i ), outer->GetY( drawing_i ) );
    }
    glVertex2d( outer->GetX( outer->GetSegments() - 1 ), outer->GetY( outer->GetSegments() - 1 ) );
    glEnd();

    glPopMatrix();
}