#include "circle.hpp"

#include <glm/gtc/matrix_transform.hpp>

void Circle::init()
{
    m_data.clear();
    m_data.reserve( m_segments );
    const double step = 360.0 / m_segments;
    for ( uint32_t i = 0; i < m_segments; i++ ) {
        const double angle = glm::radians( step * i );
        m_data.emplace_back( glm::sin( angle ) * m_radiust, glm::cos( angle ) * m_radiust );
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
        return m_data[ a ].x;
    }
    return 0;
}

double Circle::y( uint32_t a ) const
{
    if ( a < m_segments ) {
        return m_data[ a ].y;
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
