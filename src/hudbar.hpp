#pragma once

#include <ui/widget.hpp>
#include <ui/label.hpp>

class HudBar : public ui::Widget {
    ui::Label m_label{};
    float m_value = 0.0f;

public:
    ~HudBar() noexcept = default;
    HudBar() noexcept = default;
    HudBar( std::u32string_view ) noexcept;

    virtual void render( ui::RenderContext ) const override;

    void setValue( float );
};
