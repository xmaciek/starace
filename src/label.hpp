#pragma once

#include "colors.hpp"
#include "font.hpp"
#include "widget.hpp"
#include "ui_data_model.hpp"

#include <engine/math.hpp>
#include <engine/update_context.hpp>

#include <memory_resource>
#include <string_view>

namespace ui {

class Label : public Widget {
public:
    struct CreateInfo {
        DataModel* dataModel = nullptr;
        std::u32string_view text{};
        const Font* font = nullptr;
        math::vec2 position{};
        math::vec4 color = color::white;
        Anchor anchor = Anchor::fLeft | Anchor::fTop;
    };
private:
    DataModel* m_dataModel = nullptr;
    const Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    math::vec4 m_color = color::white;
    math::vec2 m_textExtent{};
    DataModel::size_type m_currentIdx = 0;
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( const CreateInfo& );

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

    void setText( std::u32string_view );
    void setText( std::pmr::u32string&& );
    DataModel* dataModel() const;
};
}

using Label = ui::Label;
