#pragma once

#include <renderer/texture.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

#include <glm/vec2.hpp>

#include <array>

class UIRings {
    std::array<Texture, 3> m_texture{};
    std::array<float, 3> m_angle{};
    glm::vec2 m_size{};

public:
    ~UIRings() noexcept = default;
    UIRings() noexcept = default;
    UIRings( std::array<Texture, 3> ) noexcept;

    void render( RenderContext ) const;
    void update( const UpdateContext& );
    void resize( glm::vec2 );

};
