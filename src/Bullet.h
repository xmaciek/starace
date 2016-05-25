#ifndef SA_BULLET_H
#define SA_BULLET_H

#include "SA.h"
#include "Model.h"
#include "Texture.h"
#include "SAObject.h"
// #define PI 3.14159265
typedef struct {
  GLdouble x,y,z;
  GLdouble speed, length, damage, delay;
  GLuint type, texture1, texture2, energy, score_per_hit;
//   GLuint owner;
  GLfloat color1[4], color2[4];
//   GLubyte name[16];
} BulletProto;

class Bullet : public SAObject {
private:
  GLdouble length;
  GLdouble max_range, range;
  
  Vertex trail[24];
  GLdouble rotX, rotY, rotZ;
  GLuint owner, type;
  GLdouble damage;
  GLuint rotation, texture1, texture2;
  GLfloat color1[4], color2[4];
  inline void Draw1();
  inline void Draw2();
  inline void DrawLaser();
  
public:
  Bullet(BulletProto bp);
  ~Bullet();
  void SetDirection(Vertex v);
  void Draw();
  void Update();
  GLuint getDamage();
  GLuint GetType();
  
  void ProcessCollision(SAObject &Object);
  
  static const GLuint SLUG = 0;
  static const GLuint BLASTER = 1;
  static const GLuint TORPEDO = 2;
  static const GLuint WAVE = 3;
  static const GLuint MINE = 4;
//   GLFloat[] getCoords() { return {x,y,z}; }
  
};

// inline bool Collision(Vertex A, Vertex B, GLfloat Range);
#endif