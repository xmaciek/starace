#ifndef SA_SHIELD_H
#define SA_SHIELD_H

#include "circle.hpp"
#include "sa.hpp"

class Shield {
public:
    Shield();
    Shield( GLdouble RadiustA, GLdouble RadiustB );
    ~Shield();
    void Draw();
    void Update();
    void SetRadiust();
    GLdouble GetRadiust();

private:
    GLdouble rotangle, radiust;
    Circle* circle;
};

#endif
