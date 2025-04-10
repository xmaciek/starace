#pragma once

#include <ui/data_model.hpp>
#include <ui/font.hpp>
#include <ui/widget.hpp>

#include <shared/hash.hpp>
#include <engine/math.hpp>

#include <memory_resource>
#include <string_view>

namespace ui {

class DataModel;

class Label : public Widget {
public:
    struct CreateInfo {
        Hash::value_type data{};
        Hash::value_type text{};
        Hash::value_type font = "small"_hash;
        Hash::value_type color = "white"_hash;
        math::vec2 position{};
        math::vec2 size{ 1000.0f, 32.0f };
        Anchor anchor = Anchor::fLeft | Anchor::fTop;
    };

private:
    DataModel* m_dataModel = nullptr;
    const Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    math::vec4 m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    math::vec2 m_labelExtent{};
    Hash::value_type m_locText{};
    DataModel::size_type m_revision = 0xFFFF;
    bool m_hasActions : 1 = false;
    // TODO remove mutable
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( const CreateInfo& );

    virtual void render( const RenderContext& ) const override;
    virtual void update( const UpdateContext& ) override;

    void setText( std::u32string_view );
    void setText( std::pmr::u32string&& );
    DataModel* dataModel() const;

    virtual void lockitChanged() override;
    virtual void refreshInput() override;
};
}

using Label = ui::Label;
