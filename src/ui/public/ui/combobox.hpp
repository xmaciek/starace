#pragma once

#include <ui/label.hpp>
#include <ui/nineslice.hpp>
#include <ui/scroll_index.hpp>
#include <ui/widget.hpp>

namespace ui {

class DataModel;

class ComboBox : public NineSlice {
    Label m_label{};
    Label m_value{};

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        std::u32string text{};
        math::vec2 position{};
        math::vec2 size{};
        uint16_t tabOrder = 0;
    };
    ~ComboBox() noexcept = default;
    ComboBox( const CreateInfo& ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
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
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
};

}
