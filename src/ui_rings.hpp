#pragma once

#include "widget.hpp"

#include <renderer/texture.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

#include <array>

class UIRings : public Widget {
    std::array<Texture, 3> m_texture{};
    std::array<float, 3> m_angle{};

public:
    ~UIRings() noexcept = default;
    UIRings() noexcept = default;
    UIRings( std::array<Texture, 3> ) noexcept;

    void render( RenderContext ) const override;
    void update( const UpdateContext& ) override;

};
