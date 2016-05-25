#ifndef SA_THRUSTER
#define SA_THRUSTER

#include "SA.h"
#include "Circle.h"

class Thruster {
public:
  Thruster(GLdouble Length, GLdouble Radiust);
  ~Thruster();
//   void Draw();
  void DrawAt(const GLdouble &X, const GLdouble &Y, const GLdouble &Z);
  void Update();
  void SetColor(GLuint Num, GLfloat* ColorData);
  void SetLength(const GLdouble &newLength);
  
private:
  GLdouble length, length_shorter, Len, radiust;
  GLuint update_i, drawing_i;
  GLfloat color[4][4];
  Circle *inner, *outer;
  
  
};


#endif