#include "Circle.h"

void Circle::init()
{
    X.clear();
    Y.clear();
    GLdouble ANGLE = ( 360.0 / segments ) / 180.0 * PI;
    for ( GLubyte i = 0; i < segments; i++ ) {
        DEGinRAD = ( i * ANGLE );
        X.push_back( (GLdouble)sin( DEGinRAD ) * radiust );
        Y.push_back( (GLdouble)cos( DEGinRAD ) * radiust );
    }
}

Circle::Circle()
{
    radiust = 1.0;
    segments = 32;
    init();
}

Circle::Circle( GLuint Segments, GLdouble Radiust )
{
    radiust = Radiust;
    segments = Segments;
    init();
}

Circle::~Circle() { }

GLdouble Circle::GetX( GLuint a )
{
    if ( a < segments ) {
        return X[ a ];
    }
    return 0;
}

GLdouble Circle::GetY( GLuint a )
{
    if ( a < segments ) {
        return Y[ a ];
    }
    return 0;
}

GLuint Circle::GetSegments()
{
    return segments;
}
GLdouble Circle::GetRadiust()
{
    return radiust;
}

void Circle::SetSegments( GLuint Segments )
{
    segments = Segments;
    init();
}

void Circle::SetRadiust( GLdouble Radiust )
{
    radiust = Radiust;
    init();
}