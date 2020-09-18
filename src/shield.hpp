#ifndef SA_SHIELD_H
#define SA_SHIELD_H

#include "Circle.h"
#include "SA.h"

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