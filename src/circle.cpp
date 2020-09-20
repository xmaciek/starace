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

Circle::Circle( GLuint segments, GLdouble radius )
: m_radiust( radius )
, m_segments( segments )
{
    init();
}

GLdouble Circle::x( GLuint a ) const
{
    if ( a < m_segments ) {
        return m_x[ a ];
    }
    return 0;
}

GLdouble Circle::y( GLuint a ) const
{
    if ( a < m_segments ) {
        return m_y[ a ];
    }
    return 0;
}

GLuint Circle::segments() const
{
    return m_segments;
}

GLdouble Circle::radius() const
{
    return m_radiust;
}

void Circle::setSegments( GLuint segments )
{
    m_segments = segments;
    init();
}

void Circle::setRadius( GLdouble radius )
{
    m_radiust = radius;
    init();
}
