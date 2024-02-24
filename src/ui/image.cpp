#include <ui/image.hpp>

#include <ui/atlas.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size }
, m_color{ ci.color }
, m_texture{ ci.texture }
, m_dataModel{ ci.model }
{
    if ( m_dataModel ) {
        m_pipeline = Pipeline::eSpriteSequenceRGBA;
        m_pipelineSlot = g_uiProperty.pipelineSpriteSequenceRGBA();
        m_current = m_dataModel->current();
        setTexture( m_dataModel->texture( m_current ) );
    }
    else if ( ci.sprite ) {
        m_pipeline = Pipeline::eSpriteSequence;
        m_pipelineSlot = g_uiProperty.pipelineSpriteSequence();
        m_uvwh = (*g_uiProperty.atlas())[ ci.sprite ] / g_uiProperty.atlas()->extent();
        setTexture( g_uiProperty.atlasTexture() );
    }
    else {
        assert( !"expected data model or sprite id when creating image" );
    }
};

void Image::render( RenderContext rctx ) const
{
    assert( m_texture );
    PushBuffer pushBuffer{
        .m_pipeline = m_pipelineSlot,
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    switch ( m_pipeline ) {
    case ui::Pipeline::eSpriteSequenceRGBA: {
        PushConstant<Pipeline::eSpriteSequenceRGBA> pushConstant{
            .m_model = rctx.model,
            .m_view = rctx.view,
            .m_projection = rctx.projection,
            .m_color = m_color,
        };
        pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ pos.x, pos.y, m_size.x, m_size.y };
        pushConstant.m_sprites[ 0 ].m_uvwh = m_uvwh;
        rctx.renderer->push( pushBuffer, &pushConstant );
    } break;

    case ui::Pipeline::eSpriteSequence: {
        PushConstant<Pipeline::eSpriteSequence> pushConstant{
            .m_model = rctx.model,
            .m_view = rctx.view,
            .m_projection = rctx.projection,
            .m_color = m_color,
        };
        pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ pos.x, pos.y, m_size.x, m_size.y };
        pushConstant.m_sprites[ 0 ].m_uvwh = m_uvwh;
        pushBuffer.m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        rctx.renderer->push( pushBuffer, &pushConstant );
    } break;

    default:
        assert( !"wrong pipeline selected" );
        break;
    }
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
