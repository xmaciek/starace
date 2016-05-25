#include "SAObject.h"

SAObject::SAObject() { target = NULL; }
SAObject::~SAObject() {}
GLdouble SAObject::getX() { return position.x; }
GLdouble SAObject::getY() { return position.y; }
GLdouble SAObject::getZ() { return position.z; }
GLuint SAObject::GetStatus() { return status; }
Vertex SAObject::GetPosition() { return position; }
Vertex SAObject::GetDirection() { return direction; }
Vertex SAObject::GetVelocity() { return velocity; }
GLdouble SAObject::GetSpeed() { return speed; }
void SAObject::SetStatus(const GLuint &s) { status = s; }

void SAObject::ProcessCollision(SAObject &Object) {}

void SAObject::SetTarget(SAObject *t) { target = t; }
void SAObject::TargetMe(const bool &doit) { ImTargeted = doit; }
void SAObject::Kill() { status = DEAD; }
void SAObject::Damage(const GLdouble &d) { health -= d; if (health<=0) { status = DEAD; } }
GLdouble SAObject::GetHealth() { return health; }

void SAObject::InterceptTarget() {
  if (target==NULL) { return; }
  if (target->GetStatus()!=ALIVE) {
    target = NULL;
    return;
  }
  
  Vertex D = direction;
  Vertex T = target->GetPosition();
  T = position - T;
  normalise_v(T);
      
  Vertex C = cross_product(D, T);
      
  tmp1 = atan(dot_product(D, T));
  tmp1 = tmp1 - turnrate_in_rads;
  tmp1 = tan(tmp1);
     
  D = cross_product(T, C);
  D = D + (T*tmp1);
//   D.x = T.x*tmp1 + (T.y*C.z - T.z*C.y);
//   D.y = T.y*tmp1 + (T.z*C.x - T.x*C.z);
//   D.z = T.z*tmp1 + (T.x*C.y - T.y*C.x);
      
  normalise_v(D);
  direction = D;
  velocity = direction * speed;  
}

bool SAObject::CanCollide() { return CollisionFlag; }

GLdouble SAObject::GetCollisionDistance() { return CollisionDistance; }

GLdouble SAObject::GetCollisionDamage() { return CollisionDamage; }

bool SAObject::DeleteMe() {
  if (ttl <= 0) { return true; }
  else {
    ttl-=1;
    return false;
  }
}

GLint SAObject::GetScore() { return score; }
void SAObject::AddScore(const GLint &s, bool b) { score += s; }



