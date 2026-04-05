#include "list.hpp"
#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

List::List( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_layoutDirection{ ci.layoutDirection }
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


Widget* List::emplace_child( UniquePointer<Widget>&& ptr )
{
    assert( ptr );
    math::vec2 pos{};
    if ( !m_children.empty() ) {
        auto&& child = m_children.back();
        pos = child->position() + child->offsetByAnchor();
        using enum LayoutDirection;
        switch ( m_layoutDirection ) {
        default: assert( !"unhandled LayoutDirection enum" ); [[fallthrough]];
        case eVerticalDown: pos.y += child->size().y; break;
        case eHorizontalRight: pos.x += child->size().x; break;
        }
    }
    auto wgt = Widget::emplace_child( std::move( ptr ) );
    wgt->setPosition( pos );
    return wgt;
}

}
