#ifndef SA_MAP_H
#define SA_MAP_H

#include "SA.h"
#include "Texture.h"

#include "settings.hpp"
#include "shader.hpp"

#include <cstdint>
#include <string>

class Map {
private:
    Settings m_settings;
    Texture m_topTexture, m_bottomTexture;
    Texture m_leftTexture, m_rightTexture;
    Texture m_frontTexture, m_backTexture;
    Texture m_previewTexture;

    Buffer m_topBuffer, m_bottomBuffer;
    Buffer m_leftBuffer, m_rightBuffer;
    Buffer m_frontBuffer, m_backBuffer;
    Buffer m_previewBuffer;
    uint32_t m_coordQuad;

    std::string m_name;

  GLuint length;
  char name[32];
  GLuint TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK;
  GLuint drawing_i,update_I;
  GLdouble v1, v2;
  GLdouble min, max;
  
  vector<Vertex> particle;
  
  Vertex jetPosition, jetVelocity, particleLength, tmp;
  
//   vector<Enemy> ememies;
  
public:
  Map(const MapProto &data);
  Map( const std::string& fileName );
  void GetJetData(const Vertex &Position, const Vertex &Velocity);
  void Update();
  void Draw();
    void drawPreview();
    void draw();
    void releaseResources();
};




#endif
