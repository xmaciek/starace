#ifndef SA_JET_H
#define SA_JET_H


#include "SA.h"
#include "Model.h"
#include "Texture.h"
#include "Bullet.h"
#include "Thruster.h"
#include "Shield.h"
#include "Circle.h"
#include "SAObject.h"
#include "Quaternion.h"

class Jet : public SAObject {
  
public:
  GLdouble energy;
  Quaternion quaternion, rotation;
  Jet(const ModelProto &model_data);
  ~Jet();
  
  void Draw();
  void DrawWireframe();
  void Update();
  void RollLeft(bool doit);
  void RollRight(bool doit);
  void PitchUp(bool doit);
  void PitchDown(bool doit);
  void YawLeft(bool doit);
  void YawRight(bool doit);
  void SpeedUp(bool doit);
  void SpeedDown(bool doit);
  void SetModel(Model *M);
  
  void LockTarget(SAObject *t);
  bool IsWeaponReady(GLuint WeaponNum);
  bool IsShooting(GLuint WeaponNum);
  void Shoot(GLuint WeaponNum, bool doit);
  Vertex GetWeaponPoint(GLuint wID);
  Bullet* GetWeaponType(GLuint wID);
  void SetWeapon(BulletProto bp, GLuint ID);
  void TakeEnergy(GLuint wID);
  void ProcessCollision(vector<Bullet*> &Bullets);
  void AddScore(const GLint &s, bool b=false);
  
private: 
  GLdouble roll, pitch, yaw, rotZ, rotX, rotY;
  BulletProto Weapon[3];
  GLint  accX, accY, accZ;
  GLint maxangleX, maxangleY, maxangleZ;
  GLdouble anglespeedX, anglespeedY, anglespeedZ;
  
  bool btn_roll_left, btn_roll_right, 
	btn_yaw_left, btn_yaw_right, 
	btn_pitch_up, btn_pitch_down;
  
//   GLfloat speed;
  Model model;
  
  GLbyte speed_acc;
  GLdouble max_speed, min_speed, norm_speed;
  
  Circle *crosshair;
  Thruster *thruster;
  Shield *shield;
  Quaternion animation;
  GLubyte CIRCLE_LOOP;
  bool shooting[3];
  GLdouble shotfactor[3];
  bool target_locked[3];
};




#endif
