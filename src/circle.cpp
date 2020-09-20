#include "circle.hpp"

void Circle::init()
{
    m_x.clear();
    m_y.clear();
    m_x.reserve( m_segments );
    m_y.reserve( m_segments );
    GLdouble ANGLE = ( 360.0 / m_segments ) / 180.0 * PI;
    for ( GLuint i = 0; i < m_segments; i++ ) {
        const double DEGinRAD = ( i * ANGLE );
        m_x.push_back( std::sin( DEGinRAD ) * m_radiust );
        m_y.push_back( std::cos( DEGinRAD ) * m_radiust );
    }
}

Circle::Circle()
{
    init();
}

Circle::Circle( GLuint Segments, GLdouble Radiust )
: m_radiust( Radiust )
, m_segments( Segments )
{
    init();
}

GLdouble Circle::GetX( GLuint a ) const
{
    if ( a < m_segments ) {
        return m_x[ a ];
    }
    return 0;
}

GLdouble Circle::GetY( GLuint a ) const
{
    if ( a < m_segments ) {
        return m_y[ a ];
    }
    return 0;
}

GLuint Circle::GetSegments() const
{
    return m_segments;
}

GLdouble Circle::GetRadiust() const
{
    return m_radiust;
}

void Circle::SetSegments( GLuint Segments )
{
    m_segments = Segments;
    init();
}

void Circle::SetRadiust( GLdouble Radiust )
{
    m_radiust = Radiust;
    init();
}
