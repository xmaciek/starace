#pragma once

#include <ui/data_model.hpp>
#include "decorator.hpp"

#include <shared/hash.hpp>
#include <renderer/texture.hpp>

namespace ui {

class Image : public Decorator {
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_revision = 0xFFFF;

public:
    struct CreateInfo {
        Hash::value_type data{};
        Hash::value_type path{};
        Hash::value_type color = "white"_hash;
        math::vec2 position{};
        math::vec2 size{};
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };
    ~Image() noexcept = default;
    Image( const CreateInfo& ) noexcept;

    virtual void update( const UpdateContext& ) override;
    void setColor( math::vec4 );
    void setTexture( Sprite );
};

}
