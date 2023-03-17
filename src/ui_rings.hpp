#pragma once

#include <ui/widget.hpp>

#include <renderer/texture.hpp>
#include <engine/update_context.hpp>

#include <array>

class UIRings : public ui::Widget {
    std::array<Texture, 3> m_texture{};
    std::array<float, 3> m_angle{};

public:
    ~UIRings() noexcept = default;
    UIRings() noexcept = default;
    UIRings( std::array<Texture, 3> ) noexcept;

    void render( ui::RenderContext ) const override;
    void update( const UpdateContext& ) override;

};
