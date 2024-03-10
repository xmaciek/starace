#pragma once

#include "autolerp.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>

#include <array>
#include <optional>

class Targeting {
    std::optional<math::vec3> m_pos{};
    AutoLerp<float> m_state{ 0.0f, 1.0f, 4.0f };
    std::array<math::vec4, 4> m_xyuv{};
    Texture m_texture{};

public:
    struct CreateInfo {
        std::array<Hash::value_type, 4> reticleSprites{};
    };

    Targeting() noexcept = default;
    Targeting( const CreateInfo& );

    void render( const RenderContext& ) const;
    void setPos( const math::vec3& );
    void hide();
    void update( const UpdateContext& );
    void setState( float );
    const math::vec3* target() const;

};
