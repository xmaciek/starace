#ifndef SA_SHIELD_H
#define SA_SHIELD_H

#include "circle.hpp"
#include "sa.hpp"

class Shield {
public:
    Shield() = default;
    Shield( GLdouble RadiustA, GLdouble RadiustB );
    ~Shield();
    void Draw() const;
    void Update();
    void SetRadiust();
    GLdouble GetRadiust();

private:
    GLdouble rotangle = 0.0;
    GLdouble radiust = 0.0;
    Circle* circle = nullptr;
};

#endif
