#pragma once

#include "font.hpp"
#include "widget.hpp"
#include "ui_data_model.hpp"

#include <engine/math.hpp>
#include <engine/update_context.hpp>

#include <memory_resource>
#include <string_view>

namespace ui {

class Label : public Widget {
private:
    DataModel* m_dataModel = nullptr;
    const Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    math::vec4 m_color{};
    math::vec2 m_textExtent{};
    DataModel::size_type m_currentIdx = 0;
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( DataModel*, const Font*, const math::vec2& position );
    Label( std::u32string_view, const Font*, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, const Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, const Font*, Anchor, const math::vec4& color );
    Label( const Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( const Font*, Anchor, const math::vec4& color );

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

    void setText( std::u32string_view );
    void setText( std::pmr::u32string&& );
};
}

using Label = ui::Label;
