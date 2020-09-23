#pragma once

#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

class Circle {
private:
    std::vector<glm::vec2> m_data{};
    double m_radiust = 1.0;
    uint32_t m_segments = 32;

    void init();

public:
    ~Circle() = default;
    Circle();
    Circle( uint32_t segments, double radius );
    double x( uint32_t ) const;
    double y( uint32_t ) const;
    void setSegments( uint32_t );
    void setRadius( double );
    uint32_t segments() const;
    double radius() const;
};
