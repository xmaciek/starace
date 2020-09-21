#pragma once

#include "sa.hpp"
#include "texture.hpp"
#include "update_context.hpp"

#include <glm/vec3.hpp>

class Map {
private:
    std::vector<glm::vec3> m_particleList{};
    double m_max = 0.0;
    double m_min = 0.0;
    double m_v1 = 0.0;
    double m_v2 = 0.0;
    glm::vec3 m_jetPosition{};
    glm::vec3 m_jetVelocity{};
    glm::vec3 m_particleLength{};
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
    void setJetData( const glm::vec3& position, const glm::vec3& velocity );
    void update( const UpdateContext& );
};
