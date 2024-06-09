#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>

#include <cstdint>

class Progressbar : public ui::Widget {
    ui::DataModel* m_dataModel = nullptr;
    ui::DataModel::size_type m_current = 0;
    math::vec4 m_uvwh{};
    Texture m_texture{};
    float m_value = 0.0f;
    float m_spacing = 0.0f;
    uint16_t m_w = 0;
    uint16_t m_h = 0;

public:
    struct CreateInfo {
        math::vec2 position{};
        Hash::value_type data{};
        Hash::value_type spriteId{};
        float spriteSpacing{};
    };

    virtual ~Progressbar() noexcept override = default;
    Progressbar() noexcept = default;
    Progressbar( const CreateInfo& ) noexcept;

    virtual void render( ui::RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

};
