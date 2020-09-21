#pragma once

#include "sa.hpp"
#include "texture.hpp"

class Map {
private:
    std::vector<Vertex> m_particleList{};
    double m_max = 0.0;
    double m_min = 0.0;
    double m_v1 = 0.0;
    double m_v2 = 0.0;
    Vertex m_jetPosition{};
    Vertex m_jetVelocity{};
    Vertex m_particleLength{};
    uint32_t m_back = 0;
    uint32_t m_bottom = 0;
    uint32_t m_front = 0;
    uint32_t m_left = 0;
    uint32_t m_right = 0;
    uint32_t m_top = 0;

public:
    ~Map();
    explicit Map( const MapProto& data );

    void draw();
    void setJetData( const Vertex& position, const Vertex& velocity );
    void update();
};
