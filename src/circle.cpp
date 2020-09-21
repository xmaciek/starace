#include "circle.hpp"

void Circle::init()
{
    m_x.clear();
    m_y.clear();
    m_x.reserve( m_segments );
    m_y.reserve( m_segments );
    double ANGLE = ( 360.0 / m_segments ) / 180.0 * PI;
    for ( uint32_t i = 0; i < m_segments; i++ ) {
        const double DEGinRAD = ( i * ANGLE );
        m_x.push_back( std::sin( DEGinRAD ) * m_radiust );
        m_y.push_back( std::cos( DEGinRAD ) * m_radiust );
    }
}

Circle::Circle()
{
    init();
}

Circle::Circle( uint32_t segments, double radius )
: m_radiust( radius )
, m_segments( segments )
{
    init();
}

double Circle::x( uint32_t a ) const
{
    if ( a < m_segments ) {
        return m_x[ a ];
    }
    return 0;
}

double Circle::y( uint32_t a ) const
{
    if ( a < m_segments ) {
        return m_y[ a ];
    }
    return 0;
}

uint32_t Circle::segments() const
{
    return m_segments;
}

double Circle::radius() const
{
    return m_radiust;
}

void Circle::setSegments( uint32_t segments )
{
    m_segments = segments;
    init();
}

void Circle::setRadius( double radius )
{
    m_radiust = radius;
    init();
}
