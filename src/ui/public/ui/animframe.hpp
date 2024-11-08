#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>

#include <cstdint>
namespace ui {

class AnimFrame : public Widget {
    math::vec4 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<math::vec4, 16> m_uvwh{};
    DataModel* m_dataModel = nullptr;
    Texture m_texture{};
    DataModel::size_type m_revision = 0xFFFF;
    uint32_t m_index{};
    uint32_t m_count{};

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type data{};
        Hash::value_type color = "white"_hash;
        std::array<Hash::value_type, 16> frames{};
    };

    virtual ~AnimFrame() noexcept override = default;
    AnimFrame() noexcept = default;
    AnimFrame( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
    virtual void update( const UpdateContext& ) override;

};

}