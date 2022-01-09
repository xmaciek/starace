#pragma once

#include "texture.hpp"

#include <engine/render_context.hpp>
#include <renderer/texture.hpp>

#include <glm/vec3.hpp>

#include <array>
#include <cstdint>
#include <string>
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

public:
    using Wall = MapCreateInfo::Wall;
    ~Map() = default;
    explicit Map( const MapCreateInfo& data );

    void render( RenderContext );
};
