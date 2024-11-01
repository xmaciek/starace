#pragma once

#include <ui/widget.hpp>
#include <renderer/pipeline.hpp>

namespace ui {

class Glow : public Widget {
public:
    PipelineSlot m_pipeline{};
    virtual ~Glow() noexcept override = default;
    inline Glow( PipelineSlot p ) noexcept : m_pipeline{ p } {}
    virtual void render( RenderContext ) const override;
};

}
