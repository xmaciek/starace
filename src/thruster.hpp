#pragma once

#include "circle.hpp"
#include "sa.hpp"

class Thruster {
private:
    double m_length = 0.0;
    double m_lengthShorter = 0.0;
    double m_len = 0.0;
    uint32_t m_wiggle = 0;
    float m_color[ 4 ][ 4 ]{};
    Circle m_inner;
    Circle m_outer;

public:
    Thruster( double length, double radius );

    void drawAt( double x, double y, double z ) const;
    void update();
    void setColor( uint32_t num, float* colorData );
    void setLength( double newLength );
};
