#include "SA.h"

  GLdouble random_range(GLdouble a, GLdouble b) {  
    return ((b-a)*((GLdouble)rand()/RAND_MAX))+a;
  }


GLfloat colorhalf(GLfloat col) {
  if (col>=0.5) return 1.0-col;
  return col;
}

GLdouble length_v(const Vertex &v) {
  return sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}

Vertex cross_product(const Vertex &a, const Vertex &b) {
  Vertex v;
  v.x = (a.y*b.z) - (a.z*b.y);
  v.y = (a.z*b.x) - (a.x*b.z);
  v.z = (a.x*b.y) - (a.y*b.x);
  return v;
};

GLdouble dot_product(const Vertex &a, const Vertex &b) {
  return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) ;
}

void normalise_v(Vertex &v) {
  GLdouble length = length_v(v);
  if (length==0) { length = 1; }
  v.x/=length;
  v.y/=length;
  v.z/=length;
};



GLdouble distance_v(const Vertex &v1, const Vertex &v2) {
  return sqrt( 
    ((v1.x-v2.x)*(v1.x-v2.x)) +
    ((v1.y-v2.y)*(v1.y-v2.y)) +
    ((v1.z-v2.z)*(v1.z-v2.z))
  );
};