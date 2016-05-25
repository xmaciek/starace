#ifndef SA_TARGET_H
#define SA_TARGET_H

#include "SA.h"

class SAObject {
public:
   SAObject();
  virtual ~SAObject();
  GLdouble getX();
  GLdouble getY();
  GLdouble getZ();
  GLuint GetStatus();
  void SetStatus(const GLuint &s);
  void SetTarget(SAObject *t);
  Vertex GetPosition();
  Vertex GetDirection();
  Vertex GetVelocity();
  GLdouble GetSpeed();
  bool DeleteMe();
  void Kill();
  void Damage(const GLdouble &d);
  void TargetMe(const bool &doit);
  GLdouble GetHealth();
  
  virtual void ProcessCollision(SAObject &Object);
  bool CanCollide();
  GLdouble GetCollisionDistance();
  
  GLint GetScore();
  virtual void AddScore(const GLint &s, bool b=false);
  
  GLdouble GetCollisionDamage();
  
  static const GLuint DEAD=0;
  static const GLuint ALIVE=1;
  static const GLuint OUT=2;
  static const GLuint NOT_VISIBLE=3;

protected:
  SAObject *target;
  bool ImTargeted;
  bool CollisionFlag;
  GLdouble CollisionDistance;
  GLdouble CollisionDamage;
  GLint score;
  
  GLdouble speed;
  GLuint status;
  GLdouble health;
  
  Vertex position, direction, velocity;
  GLuint update_i, drawing_i;
  
  GLuint ttl;
  
  GLdouble turnrate_in_rads;
  void InterceptTarget();
  

  
  GLdouble tmp1, tmp2, tmp3;

  
};


#endif