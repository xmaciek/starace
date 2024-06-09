#pragma once

#include <ui/data_model.hpp>
#include <ui/label.hpp>
#include <ui/tab_order.hpp>
#include <ui/widget.hpp>

namespace ui {

class SpinBox : public Widget {
protected:
    DataModel* m_model = nullptr;
    TabOrder<DataModel::size_type> m_index{}; // TODO: replace with something more fitting
    float m_animL = 1.0f;
    float m_animR = 1.0f;
    bool m_focusL : 1 = false;
    bool m_focusR : 1 = false;
    Label m_label{};

    math::vec4 arrowLeft() const;
    math::vec4 arrowRight() const;

public:
    static constexpr inline bool TAB_ORDERING = true;

    struct CreateInfo {
        Hash::value_type data;
        math::vec2 position{};
        math::vec2 size{};
        uint16_t tabOrder{};
    };

    ~SpinBox() noexcept = default;
    SpinBox() noexcept = default;
    SpinBox( const CreateInfo& ) noexcept;

    DataModel::size_type value() const;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
};

template <> struct TabOrdering<SpinBox> : public std::true_type {};

} // namespace ui
