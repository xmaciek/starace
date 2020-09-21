#pragma once

#include "sa.hpp"

#include <glm/vec3.hpp>

#include <cstdint>
#include <deque>

class Tail {
private:
    std::deque<glm::vec3> m_segments;

public:
    Tail() = default;
    Tail( uint16_t segments, const glm::vec3& v );
    using const_iterator = std::deque<glm::vec3>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    void insert( const glm::vec3& v );
};
