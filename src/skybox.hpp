#pragma once

#include <engine/render_context.hpp>
#include <renderer/texture.hpp>

#include <array>
#include <cstdint>

class Skybox {
    std::array<Texture, 6> m_texture{};

public:
    ~Skybox() noexcept = default;
    Skybox() noexcept = default;

    inline Skybox( const std::array<Texture, 6>& t ) noexcept
    : m_texture{ t }
    {}

    void render( const RenderContext& ) const;
};
