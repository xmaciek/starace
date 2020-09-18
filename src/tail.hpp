#pragma once

#include "sa.hpp"

#include <cstdint>
#include <deque>

class Tail {
public:
    Tail() = default;
    Tail( uint16_t segments, const Vertex& v );
    typedef std::deque<Vertex>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    void insert( const Vertex& v );

private:
    std::deque<Vertex> m_segments;
};
