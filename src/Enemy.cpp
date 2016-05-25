#include "Enemy.h"

Enemy::Enemy() {
//   static int c=0;
//   cout<<"Creating default Enemy "<<c<<".\n";
//   c++;
  ReinitCoordinates();
  speed = 2.1;
  out_range = 1.0f;
  status = ALIVE;
  health = 100;
  shield = new Shield(0.1, 0.02);
  health_perc = 1;
  direction.z = 1;

  score = 0;
  
  CollisionDistance = 0.1;
  CollisionFlag = true;
  
  normalise_v(direction);
  velocity = direction*speed;
  turnrate_in_rads = speed*5*DEG2RAD*DELTATIME;
  target = NULL;
  ImTargeted = false;
  ttl = 10;
}

Enemy::~Enemy() {
  if (shield!=NULL) { delete shield; shield = NULL; }
  
//   cout<<"killing enemy\n";
}

void Enemy::SetWeapon(const BulletProto &b) {
  weapon = b;
  shotfactor = random_range(0,weapon.delay);
  
}

Bullet* Enemy::GetWeapon() {
  
  weapon.x = position.x;
  weapon.y = position.y;
  weapon.z = position.z;
  Bullet* bullet = new Bullet(weapon);
  bullet->SetDirection(direction);
  bullet->SetTarget(target);
  shotfactor = 0;
  return bullet;
  
}

bool Enemy::IsWeaponReady() {
  if (shotfactor>=weapon.delay) return true;
  return false;
}

void Enemy::ReinitCoordinates() {
  position.x = random_range(-10.0, 10.0);
  position.y = random_range(-10.0, 10.0);
  position.z = random_range(-10.0, 10.0);
}


void Enemy::Draw() {
  if (status == ALIVE) {
    glPushMatrix();
      glTranslated(position.x, position.y, position.z);
      
//       glColor3f(1,1,1);
//       glBegin(GL_LINES);
//         glVertex3d(0,0,0);
//         glVertex3d(direction.x, direction.y, direction.z);
//       glEnd();
      
      glColor3f(1.0f-health_perc+colorhalf(1.0f-health_perc), colorhalf(health_perc)+health_perc, 0);
      shield->Draw();
      if (ImTargeted) { DrawCollisionIndicator(); }
    glPopMatrix();
  }
}

void Enemy::Update() {
  if (status == ALIVE) {
    shield->Update();
    if (shotfactor<weapon.delay) { shotfactor+=1.0*DELTATIME; }
    InterceptTarget();
    position = position + velocity*DELTATIME;
    
//     if (score>0) { 
//       score -= 1*DELTATIME;
//       if (score<0) { score = 0; }
//     }
//     
//     if (health<100) {
//       health += 1*DELTATIME;
//       if (health>100) { health = 100; }      
//     }
    
    health_perc = health/100;
  }
}

void Enemy::DrawCollisionIndicator() {
//   glGet(GL_PERSPECTIVE,
  glColor3f(1,0.1,0.1);
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
    glVertex2d(-0.125, 0.125);
    glVertex2d(-0.125, -0.125);
    glVertex2d(0.125, -0.125);
    glVertex2d(0.125, 0.125);
  glEnd();
  glLineWidth(1);
}

void Enemy::DrawRadarPosition(const Vertex &Modifier, const GLdouble &RadarScale) {
  if (status!=ALIVE) return;
  Vertex RadarPosition = Modifier;
  RadarPosition = (position - Modifier) * (RadarScale/25); 
  if (length_v(RadarPosition)>RadarScale) {
    normalise_v(RadarPosition);
    RadarPosition = RadarPosition * RadarScale;
    glColor3f(1,0.4,0.05);
  } else {
    glColor4f(1,1,1,0.9);
  }
  glPushMatrix();
    glBegin(GL_LINES);
      glVertex3d(RadarPosition.x, RadarPosition.y, RadarPosition.z);
      glVertex3d(0,0,0);
    glEnd();
  
  glPopMatrix();
}

void Enemy::ProcessCollision(SAObject &Object) {
  if (!Object.CanCollide()) { return; }
  if (status == DEAD) { return; }
  if (Object.GetStatus() != ALIVE) { return; }

  if (distance_v(position, Object.GetPosition()) <= CollisionDistance+Object.GetCollisionDistance()) {
    Damage(CollisionDamage + Object.GetCollisionDamage());
    Object.Damage(CollisionDamage + Object.GetCollisionDamage());
  }
}  
