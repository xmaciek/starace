#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>

#include <cstdint>

namespace ui {

class Progressbar : public Widget {
    math::vec4 m_uvwh{};
    math::vec2 m_spriteSize{};
    ui::DataModel* m_dataModel = nullptr;
    ui::DataModel::size_type m_current = 0;
    Texture m_texture{};
    float m_value = 0.0f;
    float m_spacing = 0.0f;
    uint32_t m_count = 0;

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{ 128.0f, 16.0f };
        Hash::value_type data{};
        Hash::value_type spriteId{};
        float spriteSpacing{};
        uint32_t count = 0;
    };

    virtual ~Progressbar() noexcept override = default;
    Progressbar() noexcept = default;
    Progressbar( const CreateInfo& ) noexcept;

    virtual void render( ui::RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

};

}
