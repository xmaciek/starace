#include "Bullet.h"



Bullet::Bullet(BulletProto bp) {
//     cout<<"Creating bullet.\n";
    CollisionDistance = 0;
    CollisionFlag = true;
    target = NULL;
    type = bp.type;    
    speed = bp.speed;
    damage = bp.damage;
//     length = bp.length;
    memcpy(color1, bp.color1, sizeof(GLfloat)*4);
    memcpy(color2, bp.color2, sizeof(GLfloat)*4);
    score = bp.score_per_hit;
    
    if (type==SLUG) {
      memcpy(color2, color1, sizeof(GLfloat)*4); 
      color1[3]=1;
      color2[3]=0;
    }
//     texture1 = bp.texture1;
//     texture2 = bp.texture2;
//     std::cout<<texture1;
    target = NULL;
    position.x = bp.x;
    position.y = bp.y;
    position.z = bp.z;
    turnrate_in_rads = (speed*10)*DEG2RAD*DELTATIME;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
    rotation = rand()%360;
    
    for (GLuint i=0; i<24; i++) {
      trail[i]= position;
    }
    range = 0;
    max_range=150;
    
    status = ALIVE;
    ttl=20;
  };
  
  Bullet::~Bullet(){ /*cout<<"Deleting Bullet.\n";*/ };
  
  inline void Bullet::Draw1() {
    glPushMatrix();
    glColor4fv(color1);
      glBegin(GL_LINES);
        glVertex3d(trail[0].x, trail[0].y, trail[0].z);
        glVertex3d(trail[3].x, trail[3].y, trail[3].z);
      glEnd();
      glBegin(GL_LINES);
        glColor4f(color1[0], color1[1], color1[2], 1);
        glVertex3d(trail[3].x, trail[3].y, trail[3].z);
        glColor4f(color1[0], color1[1], color1[2], 0);
        glVertex3d(trail[8].x, trail[8].y, trail[8].z);
      glEnd();  
      
 
    glPopMatrix();
  };
  
  
  inline void Bullet::DrawLaser() {
    glPushMatrix();
//       glLineWidth(10);
      glBegin(GL_LINES);
        glColor4fv(color1);
        glVertex3d(trail[0].x, trail[0].y, trail[0].z);
//         glColor4fv(color2);
        glVertex3d(trail[1].x, trail[1].y, trail[1].z);
      glEnd();
//       glLineWidth(1);
    glPopMatrix();
  }
  
  inline void Bullet::Draw2() {
    glPushMatrix();
    
    glColor4fv(color1);
      glBegin(GL_LINES);
        glVertex3d(trail[0].x, trail[0].y, trail[0].z);
        glVertex3d(trail[1].x, trail[1].y, trail[1].z);
      glEnd();
      glColor4f(1,1,1,1);
    for (drawing_i=1; drawing_i<23; drawing_i++) {
//       glColor4f(1,1,1, (1.0/23)*(23-drawing_i));
      glBegin(GL_LINES);
        glVertex3d(trail[drawing_i].x, trail[drawing_i].y, trail[drawing_i].z);
        glColor4f(1,1,1, 1.0/drawing_i);
        glVertex3d(trail[drawing_i+1].x, trail[drawing_i+1].y, trail[drawing_i+1].z);
      glEnd();
      
      }

    glPopMatrix();
    
  };

  void Bullet::Draw() { 
    if (status==DEAD) { return; }
    switch (type) {
      case SLUG: DrawLaser(); break;
      case BLASTER: Draw1(); break;
      case TORPEDO: Draw2(); break;
      case WAVE: break;
      case MINE: break;
    }
  };
  
  void Bullet::Update() {
    
    if (status==DEAD) { return; }
    if (range>max_range) { status = DEAD; return; }
    
    switch (type) {
      case SLUG: 
        color1[3]-=2.0*DELTATIME;
//         tmp2 = 1-(max_range/range);
//         if(tmp2>=0){ color1[3]=tmp2; } else { color1[3]=0; }
        range+=max_range*2.0*DELTATIME;
        break;
      case TORPEDO:
        InterceptTarget();/*no break*/
      case BLASTER:
        position = position + velocity*DELTATIME;
        for (update_i=23; update_i>0; update_i-=1) { trail[update_i] = trail[update_i-1]; }
        trail[0]=position;
        range+=speed*DELTATIME;
        break;
      case WAVE: break;
      case MINE: break;
    }
    

    
  };
  

  
void Bullet::ProcessCollision(SAObject &Object) {
  if (!Object.CanCollide()) { return; }
  if (status == DEAD) { return; }
  if (Object.GetStatus() != ALIVE) { return; }

  Vertex v;
  Vertex p = Object.GetPosition();
  
  if (type==BLASTER) { v = trail[3] - trail[0]; }
  else { v = trail[1] - trail[0]; }
  Vertex w = p - trail[0];
  
  
  GLdouble dist;
  tmp2 = dot_product(w, v);
  if (tmp2 <= 0) {
    dist = length_v(p - trail[0]);
  }
  else {
    tmp3 = dot_product(v,v);
    if (tmp3<=tmp2) {
      dist = length_v(p - trail[0]);
    }
    else {
      GLdouble b = tmp2/tmp3;
      Vertex Pb = trail[0] + (v*b);
      dist = length_v(p - Pb);
    }
  }
  
  GLdouble col = CollisionDistance + Object.GetCollisionDistance();
  if (dist<=col) {
  Object.AddScore(score);
  switch (type) {
    case SLUG: { 
      Object.Damage(damage*color1[3]); 
      break;
    }
    case BLASTER: {
      Object.Damage(damage);
      status = DEAD;
      break;
    }
    case TORPEDO: {
      Object.Damage(damage);
      status = DEAD;
      break;
    }
    case WAVE: {}
    case MINE: {}
    default: break;
  }
  }
  
}
  
  
  
  void Bullet::SetDirection(Vertex v) {
    direction = v;
    normalise_v(direction);
    velocity = direction * speed;
    if (type==SLUG) { 
      trail[1]=position+(direction*1000);
    }
  }
  
  GLuint Bullet::getDamage() { return damage; } 
//   GLFloat[] getCoords() { return {x,y,z}; }
  
GLuint Bullet::GetType() { return type; }