#pragma once

#include "chase.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

#include <optional>

class Targeting {
    std::optional<math::vec3> m_pos{};
    Chase<float> m_state{ 0.0f, 1.0f, 4.0f };

public:
    Targeting() noexcept = default;

    void render( const RenderContext& ) const;

    void setPos( const math::vec3& );
    void hide();
    void update( const UpdateContext& );
    void setState( float );
    const math::vec3* target() const;

};
