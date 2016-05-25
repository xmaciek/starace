#ifndef SA_CIRCLE
#define SA_CIRCLE

#include "SA.h"
class Circle {
public:
  Circle();
  Circle(GLuint Segments, GLdouble Radiust);
  ~Circle();
  GLdouble GetX(GLuint a);
  GLdouble GetY(GLuint a);
  void SetSegments(GLuint Segments);
  void SetRadiust(GLdouble Radiust);
  GLuint GetSegments();
  GLdouble GetRadiust();
  
private:
   GLdouble DEGinRAD;
   GLdouble radiust;
   GLuint segments;
   GLuint update_i, drawing_i;
   std::vector<GLdouble> X, Y;
   void init();
};



#endif