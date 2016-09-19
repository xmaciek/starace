#include "Circle.h"

#include <cassert>
#include <cmath>

static void init( std::vector< std::pair<double, double> > *coord, uint64_t segs, double rads )
{
    assert( coord );
    coord->clear();
    const double angle = ( 360.0 / segs ) / ( 180.0 * atan( 1 ) * 4 );
    for ( uint64_t i=0; i<segs; i++ ) {
        const double deg = i * angle;
        coord->push_back( std::make_pair( sin( deg ) * rads, cos( deg ) * rads ) );
    }
}


Circle::Circle( uint64_t segs, double rads ) :
    m_radiust( rads )
{
    init( &m_coord, segs, rads );
}

void Circle::SetSegments( uint64_t segs )
{
    init( &m_coord, segs, m_radiust );
}

void Circle::SetRadiust( double rads )
{
    m_radiust = rads;
    init( &m_coord, m_coord.size(), rads );
}
