#include "list.hpp"
#include <renderer/renderer.hpp>

namespace ui {

List::List( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
{
}

void List::preRender( RenderContext& rctx ) const
{
    rctx.renderer->pushScissor( math::vec4{ rctx.viewportOffset, size() }, rctx.viewport );
}

void List::postRender( RenderContext& rctx ) const
{
    rctx.renderer->popScissor();
}

}
