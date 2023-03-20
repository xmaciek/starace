#include <ui/image.hpp>

#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size }
, m_dataModel{ ci.model }
, m_color{ ci.color }
, m_texture{ ci.texture }
{
    if ( m_dataModel ) {
        m_current = m_dataModel->current();
        setTexture( m_dataModel->texture( m_current ) );
    }
};

void Image::render( RenderContext rctx ) const
{
    assert( m_texture );
    PushBuffer pushBuffer{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceRGBA(),
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    PushConstant<Pipeline::eSpriteSequenceRGBA> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };
    pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ pos.x, pos.y, m_size.x, m_size.y };
    pushConstant.m_sprites[ 0 ].m_uvwh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };

    rctx.renderer->push( pushBuffer, &pushConstant );
}

void Image::update( const UpdateContext& )
{
    if ( !m_dataModel ) {
        return;
    }
    DataModel::size_type idx = m_dataModel->current();
    if ( idx == m_current ) {
        return;
    }
    m_current = idx;
    setTexture( m_dataModel->texture( m_current ) );
}

void Image::setColor( math::vec4 c )
{
    m_color = c;
}

void Image::setTexture( Texture t )
{
    assert( t );
    m_texture = t;
}

}
