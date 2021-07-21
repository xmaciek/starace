#pragma once

#include "render_context.hpp"
#include "texture.hpp"
#include "update_context.hpp"
#include <renderer/texture.hpp>

#include <glm/vec3.hpp>

#include <cstdint>
#include <string>
#include <vector>

struct MapProto {
    Texture preview_image{};
    uint32_t enemies = 0;
    std::string name{ "unnamed map" };
    std::string texture_location{};
    std::string TOP{};
    std::string BOTTOM{};
    std::string LEFT{};
    std::string RIGHT{};
    std::string FRONT{};
    std::string BACK{};
    std::string preview_image_location{};
};

class Map {
private:
    Texture m_back{};
    Texture m_bottom{};
    Texture m_front{};
    Texture m_left{};
    Texture m_right{};
    Texture m_top{};
    std::vector<glm::vec4> m_particleList{};
    glm::vec3 m_jetPosition{};
    glm::vec3 m_jetVelocity{};
    glm::vec4 m_particleLength{};

public:
    ~Map();
    explicit Map( const MapProto& data );

    void render( RenderContext );
    void setJetData( const glm::vec3& position, const glm::vec3& velocity );
    void update( const UpdateContext& );
};
