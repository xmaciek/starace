#ifndef SA_MAP_H
#define SA_MAP_H

#include "sa.hpp"
#include "texture.hpp"
// #include "Enemy.h"

class Map {
private:
    GLuint length;
    char name[ 32 ];
    GLuint TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK;
    GLuint drawing_i, update_I;
    GLdouble v1, v2;
    GLdouble min, max;

    vector<Vertex> particle;

    Vertex jetPosition, jetVelocity, particleLength, tmp;

    //   vector<Enemy> ememies;

public:
    Map( const MapProto& data );
    ~Map();
    void GetJetData( const Vertex& Position, const Vertex& Velocity );
    void Update();
    void Draw();
};

#endif
