#pragma once

#include "widget.hpp"

class Glow : public Widget {
public:
    virtual ~Glow() noexcept override = default;
    Glow() noexcept = default;
    virtual void render( ui::RenderContext ) const override;
};
