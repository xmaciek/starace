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
  GLdouble GetRadiust();
  
private:
  GLdouble rotangle, radiust;
  Circle circle;
};




#endif
