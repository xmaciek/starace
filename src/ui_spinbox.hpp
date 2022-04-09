#pragma once

#include "widget.hpp"
#include "label.hpp"
#include "game_action.hpp"
#include "utils.hpp"
#include "tab_order.hpp"

#include <functional>
#include <string>
#include <memory_resource>

namespace ui {

class SpinBox : public Widget {
public:
    using IndexToText = std::function<std::pmr::u32string(int)>;
protected:
    TabOrder<int> m_index{}; // TODO: replace with something more fitting
    IndexToText m_indexToText = intToUTF32<int>;
    math::vec4 m_colorL{};
    math::vec4 m_colorR{};
    Label m_label{};

    math::vec4 arrowLeft() const;
    math::vec4 arrowRight() const;

public:
    ~SpinBox() noexcept = default;
    SpinBox() noexcept = default;
    SpinBox( int current, int min, int max ) noexcept;
    SpinBox( int current, int min, int max, IndexToText&& ) noexcept;

    int value() const;

    virtual void render( RenderContext ) const override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

    virtual bool onAction( Action );
};

} // namespace ui
