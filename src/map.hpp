#pragma once

#include "render_context.hpp"
#include "texture.hpp"
#include "update_context.hpp"
#include <renderer/texture.hpp>

#include <glm/vec3.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

struct MapCreateInfo {
    std::string name{ "unnamed map" };
    enum Wall {
        eTop,
        eBottom,
        eLeft,
        eRight,
        eFront,
        eBack,
        ePreview,
        max,
    };
    std::array<std::filesystem::path,Wall::max> filePath{};
    std::array<Texture, Wall::max> texture{};
    uint32_t enemies = 0;
};

class Map {
private:
    decltype( MapCreateInfo::texture ) m_texture{};
    std::vector<glm::vec4> m_particleList{};
    glm::vec3 m_jetPosition{};
    glm::vec3 m_jetVelocity{};
    glm::vec4 m_particleLength{};

public:
    using Wall = MapCreateInfo::Wall;
    ~Map() = default;
    explicit Map( const MapCreateInfo& data );

    void render( RenderContext );
    void setJetData( const glm::vec3& position, const glm::vec3& velocity );
    void update( const UpdateContext& );
};
