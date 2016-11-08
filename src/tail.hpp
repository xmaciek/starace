#pragma once

#include "SA.h"

#include <cstdint>
#include <deque>

class Tail {
public:
    Tail( uint16_t segments, const Vertex& v );
    typedef std::deque<Vertex>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    void insert( const Vertex& v );

private:
    std::deque<Vertex> m_segments;
};
