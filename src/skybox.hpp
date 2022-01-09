#pragma once

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
        max,
    };
    std::filesystem::path previewPath{};
    std::array<std::filesystem::path,Wall::max> filePath{};
    std::array<Texture, 6> texture{};
    Texture preview{};
    uint32_t enemies = 0;
};

class Skybox
{
    std::array<Texture, 6> m_texture{};

public:
    ~Skybox() noexcept = default;
    Skybox() noexcept = default;

    inline Skybox( const std::array<Texture, 6>& t ) noexcept
    : m_texture{ t }
    {}

    void render( const RenderContext& ) const;
};
