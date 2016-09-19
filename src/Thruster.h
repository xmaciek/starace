#pragma once

#include "Circle.h"
#include <cstdint>

class Thruster {
public:
    Thruster( double length, double radiust );
    void DrawAt( double x, double y, double z );
    void Update();
    void SetColor( uint64_t num, float* colorData );
    void SetLength( double newLength );

private:
    uint64_t m_inner, m_outter;
    uint64_t m_innerColor, m_outterColor;
    uint64_t m_scaleSwitcher;
    double m_scale, m_length;
    double m_radiust;
    float m_color[4][4];
    Circle m_innerCircle, m_outterCircle;


};
