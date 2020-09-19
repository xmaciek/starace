#include "circle.hpp"

void Circle::init()
{
    X.clear();
    Y.clear();
    GLdouble ANGLE = ( 360.0 / segments ) / 180.0 * PI;
    for ( GLuint i = 0; i < segments; i++ ) {
        const double DEGinRAD = ( i * ANGLE );
        X.push_back( std::sin( DEGinRAD ) * radiust );
        Y.push_back( std::cos( DEGinRAD ) * radiust );
    }
}

Circle::Circle()
{
    init();
}

Circle::Circle( GLuint Segments, GLdouble Radiust )
: radiust( Radiust )
, segments( Segments )
{
    init();
}

GLdouble Circle::GetX( GLuint a ) const
{
    if ( a < segments ) {
        return X[ a ];
    }
    return 0;
}

GLdouble Circle::GetY( GLuint a ) const
{
    if ( a < segments ) {
        return Y[ a ];
    }
    return 0;
}

GLuint Circle::GetSegments() const
{
    return segments;
}

GLdouble Circle::GetRadiust() const
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
