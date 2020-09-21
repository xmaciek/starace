#include "tail.hpp"

Tail::Tail( uint16_t segments, const glm::vec3& v )
: m_segments( segments, v )
{
}

Tail::const_iterator Tail::begin() const
{
    return m_segments.begin();
}

Tail::const_iterator Tail::end() const
{
    return m_segments.end();
}

void Tail::insert( const glm::vec3& v )
{
    m_segments.push_front( v );
    m_segments.pop_back();
}
