#ifndef SA_QUATERNION_H
#define SA_QUATERNION_H

#include "SA.h"

class Quaternion {
  
public:
  Quaternion();
  Quaternion(Vertex v, GLfloat w);
  ~Quaternion();
  void Conjugate();
  void Inverse();
  void Normalise();
  void CreateMatrix(GLfloat *mat);
  void CreateFromAngles(const GLdouble &X, const GLdouble &Y, const GLdouble &Z, const GLdouble &deg);
  Quaternion operator *(const Quaternion &Q);
  Quaternion operator =(const Quaternion &Q);
  void RotateVector(Vertex &v);
  Vertex GetVector();
  
private:
  GLdouble x,y,z,w;
  
};

#endif



