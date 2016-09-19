#ifndef SA_SHIELD_H
#define SA_SHIELD_H

#include "SA.h"
#include "Circle.h"

class Shield {
public:
    Shield( double radiustA, double radiustB );
    void Draw();
    void Update();
    void SetRadiust();
    double GetRadiust() const;

private:
    double m_rotAngle, m_radiust;
    static uint64_t s_segment;
    Circle m_circle;
};




#endif
