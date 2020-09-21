#pragma once

#include "circle.hpp"
#include "sa.hpp"

class Shield {
private:
    double m_rotAngle = 0.0;
    double m_radius = 0.0;
    Circle m_circle{};

public:
    Shield() = default;
    Shield( double radiusA, double radiusB );

    double radius();
    void draw() const;
    void update();
};
