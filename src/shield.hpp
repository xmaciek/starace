#ifndef SA_SHIELD_H
#define SA_SHIELD_H

#include "circle.hpp"
#include "sa.hpp"

class Shield {
private:
    GLdouble m_rotAngle = 0.0;
    GLdouble m_radius = 0.0;
    Circle m_circle{};

public:
    Shield() = default;
    Shield( GLdouble radiusA, GLdouble radiusB );

    GLdouble radius();
    void draw() const;
    void update();
};

#endif
