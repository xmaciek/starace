#pragma once

#include <engine/render_context.hpp>

#include <glm/vec3.hpp>

#include <optional>

class Targeting {
    std::optional<glm::vec3> m_pos{};

public:
    Targeting() noexcept = default;

    void render( RenderContext ) const;

    void setPos( glm::vec3 );
    void hide();

};
