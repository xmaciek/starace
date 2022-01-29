#pragma once

#include <engine/math.hpp>
#include <engine/render_context.hpp>

#include <optional>

class Targeting {
    std::optional<math::vec3> m_pos{};

public:
    Targeting() noexcept = default;

    void render( RenderContext ) const;

    void setPos( math::vec3 );
    void hide();

};
