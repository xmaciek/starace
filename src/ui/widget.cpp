#include <ui/widget.hpp>

namespace ui {

EventProcessing Widget::onEvent( const MouseEvent& e )
{
    const math::vec2 pos = position() + offsetByAnchor();

    MouseEvent ev = e;
    ev.position -= pos;
    for ( auto&& it : m_children ) {
        auto ret = it->onEvent( ev );
        if ( ret == EventProcessing::eStop ) return ret;
    }
    return onMouseEvent( e );
}

void Widget::onRender( RenderContext rctx ) const
{
    const math::vec2 pos = position() + offsetByAnchor();
    rctx.model = math::translate( rctx.model, math::vec3{ pos.x, pos.y, 0.0f } );
    render( rctx );
    for ( auto&& it : m_children ) it->onRender( rctx );
}

void Widget::onUpdate( const UpdateContext& uctx )
{
    update( uctx );
    for ( auto&& it : m_children ) it->onUpdate( uctx );
}

void Widget::setPosition( math::vec2 v )
{
    m_position = v;
}

void Widget::setSize( math::vec2 v )
{
    m_size = v;
}

math::vec2 Widget::position() const
{
    return m_position;
}

math::vec2 Widget::size() const
{
    return m_size;
}

void Widget::setAnchor( Anchor a )
{
    m_anchor = a;
}

uint16_t Widget::tabOrder() const
{
    return m_tabOrder;
}

void Widget::setTabOrder( uint16_t v )
{
    m_tabOrder = v;
}

EventProcessing Widget::onMouseEvent( const MouseEvent& )
{
    return EventProcessing::eContinue;
}

void Widget::update( const UpdateContext& )
{
}

void Widget::render( const RenderContext& ) const
{
}

bool Widget::isEnabled() const
{
    return m_enabled;
}

void Widget::setEnabled( bool b )
{
    m_enabled = b;
}

bool Widget::isFocused() const
{
    return m_focused;
}

void Widget::setFocused( bool b )
{
    m_focused = b;
}

bool Widget::testRect( math::vec2 p, math::vec2 pos, math::vec2 size )
{
    const math::vec2 br = pos + size;
    return ( p.x >= pos.x )
        && ( p.y >= pos.y )
        && ( p.x < br.x )
        && ( p.y < br.y );
}

bool Widget::testRect( math::vec2 p, const math::vec4& xywh )
{
    return ( p.x >= xywh.x )
        && ( p.y >= xywh.y )
        && ( p.x < xywh.x + xywh.z )
        && ( p.y < xywh.y + xywh.w );
}

bool Widget::testRect( math::vec2 p ) const
{
    return testRect( p, position() + offsetByAnchor(), size() );
}

math::vec2 Widget::offsetByAnchor() const
{
    math::vec2 ret{};

    if ( m_anchor && Anchor::fCenter ) {
        ret.x = m_size.x * -0.5f;
    }
    else if ( m_anchor && Anchor::fRight ) {
        ret.x = -m_size.x;
    }
    // else ret.x = 0.0f;

    if ( m_anchor && Anchor::fMiddle ) {
        ret.y = m_size.y * -0.5f;
    }
    else if ( m_anchor && Anchor::fBottom ) {
        ret.y = -m_size.y;
    }
    // else ret.x = 0.0f;

    return ret;
}

EventProcessing Widget::onAction( ui::Action )
{
    return EventProcessing::eContinue;
}

}

void Layout::operator() ( std::span<Widget*> wgts ) const
{
    math::vec2 position = m_position;
    switch ( m_flow ) {
    case eHorizontal:
        for ( Widget* it : wgts ) {
            const math::vec2 size = it->size();
            it->setPosition( position );
            position.x += size.x;
        }
        break;
    case eVertical:
        for ( Widget* it : wgts ) {
            const math::vec2 size = it->size();
            it->setPosition( position );
            position.y += size.y;
        }
        break;
    default:
        assert( !"unhandled enum" );
        break;
    }
}
