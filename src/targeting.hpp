#pragma once

#include "autolerp.hpp"
#include "saobject.hpp"
#include "render_context.hpp"
#include "update_context.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <extra/csg.hpp>

#include <array>
#include <memory_resource>
#include <vector>

class Targeting {
    AutoLerp<float> m_state{ 0.0f, 1.0f, 6.0f/*4.0f*/ };
    std::pmr::vector<Signal> m_signals{};
    std::span<csg::Callsign> m_callsigns{};
    std::array<math::vec4, 4> m_xyuvTarget{};
    std::array<math::vec4, 4> m_xyuvTarget2{};
    std::array<math::vec4, 4> m_xyuvReticle{};
    Signal m_targetSignal{};
    Texture m_texture{};
    bool m_enabled = true;

public:
    struct CreateInfo {
        std::array<Hash::value_type, 4> targetSprites{};
        std::array<Hash::value_type, 4> targetSprites2{};
        std::array<Hash::value_type, 4> reticleSprites{};
        std::span<csg::Callsign> callsigns{};
    };

    Targeting() noexcept = default;
    Targeting( const CreateInfo& );

    void render( const RenderContext& ) const;
    void hide();
    void update( const UpdateContext& );
    void setSignals( std::pmr::vector<Signal>&& );
    void setTarget( Signal, float );
    inline operator bool () const { return m_enabled; }
};
