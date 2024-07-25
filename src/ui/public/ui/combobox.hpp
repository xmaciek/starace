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
    Hash::value_type m_data{};

    void open();

public:
    struct CreateInfo {
        Hash::value_type data{};
        Hash::value_type text{};
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

template <> struct TabOrdering<ComboBox> : public std::true_type {};

}
