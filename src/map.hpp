#ifndef SA_MAP_H
#define SA_MAP_H

#include "sa.hpp"
#include "texture.hpp"

class Map {
private:
    GLuint TOP = 0;
    GLuint BOTTOM = 0;
    GLuint LEFT = 0;
    GLuint RIGHT = 0;
    GLuint FRONT = 0;
    GLuint BACK = 0;
    GLdouble v1 = 0.0;
    GLdouble v2 = 0.0;
    GLdouble min = 0.0;
    GLdouble max = 0.0;

    std::vector<Vertex> particle{};

    Vertex jetPosition{};
    Vertex jetVelocity{};
    Vertex particleLength{};

public:
    explicit Map( const MapProto& data );
    ~Map();
    void GetJetData( const Vertex& Position, const Vertex& Velocity );
    void Update();
    void Draw();
};

#endif
