#pragma once

#include "label.hpp"
#include "nineslice.hpp"
#include "widget.hpp"
#include <ui/data_model.hpp>
#include <ui/scroll_index.hpp>

namespace ui {

class ComboBox : public NineSlice {
    [[maybe_unused]]
    DataModel* m_model = nullptr;
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
    virtual bool onAction( ui::Action ) override;
};


class ComboBoxList : public Widget {
    DataModel* m_model = nullptr;
    ScrollIndex m_index{};
    float m_lineHeight = 0.0f;
    float m_topPadding = 0.0f;
    float m_botHeight = 8.0f;

    DataModel::size_type visibleCount() const;
    DataModel::size_type selectedIndex() const;

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
    virtual bool onAction( ui::Action ) override;
};

}
