#pragma once

#include "autolerp.hpp"
#include "saobject.hpp"
#include "render_context.hpp"
#include "update_context.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <extra/csg.hpp>
#include <ui/sprite.hpp>

#include <array>
#include <memory_resource>
#include <vector>

class Targeting {
    AutoLerp<float> m_state{ 0.0f, 1.0f, 6.0f/*4.0f*/ };
    std::pmr::vector<Signal> m_signals{};
    std::span<const csg::Callsign> m_callsigns{};
    std::array<ui::Sprite, 4> m_xyuvTarget{};
    std::array<ui::Sprite, 4> m_xyuvTarget2{};
    std::array<ui::Sprite, 4> m_xyuvReticle{};
    Signal m_targetSignal{};

public:
    struct CreateInfo {
        std::span<const csg::Callsign> callsigns{};
    };

    Targeting() noexcept = default;
    Targeting( const CreateInfo& );

    void render( const RenderContext& ) const;
    void update( const UpdateContext& );
    void setSignals( std::pmr::vector<Signal>&& );
    void setTarget( Signal, float );
};
