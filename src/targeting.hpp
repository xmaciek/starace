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
    math::vec4 m_pos{ 0.0f, 0.0f, 1.0f, 0.0f };
    AutoLerp<float> m_state{ 0.0f, 1.0f, 4.0f };
    std::array<math::vec4, 4> m_xyuv{};
    Texture m_texture{};
    bool m_enabled = true;

public:
    struct CreateInfo {
        std::array<Hash::value_type, 4> reticleSprites{};
    };

    Targeting() noexcept = default;
    Targeting( const CreateInfo& );

    void render( const RenderContext& ) const;
    void hide();
    void update( const UpdateContext& );
    void setState( math::vec4 );
    math::vec3 position() const;
    inline operator bool () const { return m_enabled; }
};
