#pragma once

#include "sa.hpp"
#include "texture.hpp"

class Map {
private:
    std::vector<Vertex> m_particleList{};
    GLdouble m_max = 0.0;
    GLdouble m_min = 0.0;
    GLdouble m_v1 = 0.0;
    GLdouble m_v2 = 0.0;
    Vertex m_jetPosition{};
    Vertex m_jetVelocity{};
    Vertex m_particleLength{};
    GLuint m_back = 0;
    GLuint m_bottom = 0;
    GLuint m_front = 0;
    GLuint m_left = 0;
    GLuint m_right = 0;
    GLuint m_top = 0;

public:
    ~Map();
    explicit Map( const MapProto& data );

    void draw();
    void setJetData( const Vertex& position, const Vertex& velocity );
    void update();
};
