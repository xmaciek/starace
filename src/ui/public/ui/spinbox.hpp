#pragma once

#include <ui/data_model.hpp>
#include <ui/label.hpp>
#include <ui/tab_order.hpp>
#include <ui/widget.hpp>

namespace ui {

class SpinBox : public Widget {
protected:
    TabOrder<DataModel::size_type> m_index{}; // TODO: replace with something more fitting
    DataModel* m_model = nullptr;
    float m_animL = 1.0f;
    float m_animR = 1.0f;
    bool m_focusL : 1 = false;
    bool m_focusR : 1 = false;
    Label m_label{};

    math::vec4 arrowLeft() const;
    math::vec4 arrowRight() const;

public:
    ~SpinBox() noexcept = default;
    SpinBox() noexcept = default;
    SpinBox( DataModel* ) noexcept;

    DataModel::size_type value() const;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual MouseEvent::Processing onMouseEvent( const MouseEvent& ) override;
    virtual bool onAction( ui::Action ) override;
};

} // namespace ui
