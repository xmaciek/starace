#include "Shield.h"

Shield::Shield(GLdouble RadiustA, GLdouble RadiustB) :
    circle( 6, RadiustB ),
    radiust( RadiustA ),
    rotangle( 0 )
{
}


void Shield::Update() {
  rotangle += 1;
  if (rotangle>=360) { rotangle -= 360; }
}

void Shield::Draw() {
  glPushMatrix();
    glRotated(rotangle,0,1,0);
    for (GLuint i=0; i<8; i++) {
      glBegin(GL_LINE_LOOP);
        for (GLuint j=0; j<circle.GetSegments(); j++) { 
          glVertex3d(circle.GetX(j), circle.GetY(j), radiust); }
      glEnd();
      glBegin(GL_LINES);
        glVertex3d(0,0,0);
        glVertex3d(0,0,radiust);
      glEnd();
      glRotated(45,0,1,0);
    }
    glPushMatrix();
      glRotated(rotangle,0,0,1);
      for (GLuint i=0; i<8; i++) {
        glBegin(GL_LINE_LOOP);
          for (GLuint j=0; j<circle.GetSegments(); j++) { 
            glVertex3d(circle.GetX(j), circle.GetY(j), radiust); }
        glEnd();
        glBegin(GL_LINES);
          glVertex3d(0,0,0);
          glVertex3d(0,0,radiust);
        glEnd();
        glRotated(45,0,1,0);
      }
      glPushMatrix();
        glRotated(rotangle,1,0,0);
        for (GLuint i=0; i<8; i++) {
          glBegin(GL_LINE_LOOP);
            for (GLuint j=0; j<circle.GetSegments(); j++) { 
              glVertex3d(circle.GetX(j), circle.GetY(j), radiust); }
          glEnd();
          glBegin(GL_LINES);
            glVertex3d(0,0,0);
            glVertex3d(0,0,radiust);
          glEnd();
          glRotated(45,0,1,0);
          
        }
        glPopMatrix();
        glPopMatrix();
      glPopMatrix();
    
    
}
