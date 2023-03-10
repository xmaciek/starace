#pragma once

#include "chase.hpp"
#include "game_action.hpp"
#include "label.hpp"
#include "nineslice.hpp"
#include "tab_order.hpp"
#include "ui_data_model.hpp"
#include "ui_property.hpp"
#include "utils.hpp"
#include "widget.hpp"

namespace ui {

class ComboBox : public NineSlice {
    [[maybe_unused]]
    DataModel* m_model = nullptr;
    Chase<float> m_anim{ 0.0f, 0.0f, 5.618f };
    Label m_label{};
    Label m_value{};

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        std::u32string text{};
        math::vec2 position{};
        math::vec2 size{};
    };
    ~ComboBox() noexcept = default;
    ComboBox( const CreateInfo& ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual MouseEvent::Processing onMouseEvent( const MouseEvent& ) override;
    virtual bool onAction( Action ) override;
};


class ComboBoxList : public Widget {
    DataModel* m_model = nullptr;
    DataModel::size_type m_highlightIndex = 0;
    float m_lineHeight = 0.0f;
    float m_topPadding = 0.0f;
    float m_botHeight = 8.0f;

    DataModel::size_type visibleCount() const;

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        math::vec2 position{};
        math::vec2 size{};
    };


    ~ComboBoxList() noexcept = default;
    ComboBoxList() noexcept = default;
    ComboBoxList( const CreateInfo& p ) noexcept;


    virtual void render( RenderContext ) const override;
    virtual MouseEvent::Processing onMouseEvent( const MouseEvent& ) override;
    virtual bool onAction( Action ) override;
};

}
