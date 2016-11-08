#include "tail.hpp"

Tail::Tail( uint16_t segments, const Vertex& v )
{
    while ( segments-- > 0 ) {
        m_segments.push_back( v );
    }
}

Tail::const_iterator Tail::begin() const
{
    return m_segments.begin();
}

Tail::const_iterator Tail::end() const
{
    return m_segments.end();
}

void Tail::insert( const Vertex &v )
{
    m_segments.push_front( v );
    m_segments.pop_back();
}
