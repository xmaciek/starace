#pragma once

#include "sa.hpp"

#include <cstdint>
#include <deque>

class Tail {
private:
    std::deque<Vertex> m_segments;

public:
    Tail() = default;
    Tail( uint16_t segments, const Vertex& v );
    using const_iterator  = std::deque<Vertex>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    void insert( const Vertex& v );
};
