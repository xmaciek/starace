#pragma once

#include "widget.hpp"
#include "label.hpp"
#include "game_action.hpp"
#include "utils.hpp"
#include "tab_order.hpp"
#include "ui_data_model.hpp"

#include <functional>
#include <string>
#include <memory_resource>

namespace ui {

class SpinBox : public Widget {
protected:
    TabOrder<DataModel::size_type> m_index{}; // TODO: replace with something more fitting
    DataModel* m_model = nullptr;

    math::vec4 m_colorL{};
    math::vec4 m_colorR{};
    Label m_label{};

    math::vec4 arrowLeft() const;
    math::vec4 arrowRight() const;

public:
    ~SpinBox() noexcept = default;
    SpinBox() noexcept = default;
    SpinBox( DataModel* ) noexcept;

    DataModel::size_type value() const;

    virtual void render( RenderContext ) const override;
    virtual bool onMouseEvent( const MouseEvent& ) override;
    virtual bool onAction( Action ) override;
};

} // namespace ui
