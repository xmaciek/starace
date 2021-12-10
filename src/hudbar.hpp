#pragma once

#include "widget.hpp"
#include "label.hpp"

class Font;

class HudBar : public Widget {
    Label m_label{};
    float m_value = 0.0f;

public:
    ~HudBar() noexcept = default;
    HudBar() noexcept = default;
    HudBar( std::u32string_view, Font* ) noexcept;

    virtual void render( RenderContext ) const override;

    void setValue( float );
};
