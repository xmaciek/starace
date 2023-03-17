#pragma once

#include <ui/widget.hpp>

class Glow : public ui::Widget {
public:
    virtual ~Glow() noexcept override = default;
    Glow() noexcept = default;
    virtual void render( ui::RenderContext ) const override;
};
