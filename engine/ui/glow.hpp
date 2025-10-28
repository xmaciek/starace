#pragma once

#include <ui/widget.hpp>
#include <renderer/pipeline.hpp>

namespace ui {

class Glow : public Widget {
public:
    PipelineSlot m_pipeline{};
    virtual ~Glow() noexcept override = default;
    Glow() noexcept;
    virtual void render( const RenderContext& ) const override;
};

}
