#include <ui/image.hpp>

#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <iostream>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_color{ g_uiProperty.color( ci.color ) }
{
    m_pipelineSlot = g_uiProperty.pipelineSpriteSequence();
    if ( ci.data ) {
        m_dataModel = g_uiProperty.dataModel( ci.data );
        m_revision = m_dataModel->revision();
        setTexture( m_dataModel->texture( m_dataModel->current() ) );
    }
    else if ( ci.path ) setTexture( g_uiProperty.sprite( ci.path ) );
    else {
        assert( !"expected data model or sprite id when creating image" );
    }
};

void Image::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eSpriteSequence>;
    assert( m_sprite );
    RenderInfo ri{
        .m_pipeline = m_pipelineSlot,
        .m_verticeCount = PushConstant::VERTICES,
    };
    ri.m_fragmentTexture[ 0 ] = m_sprite;

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };
    pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ 0.0f, 0.0f, m_size.x, m_size.y };
    pushConstant.m_sprites[ 0 ].m_uvwh = m_sprite;
    pushConstant.m_sprites[ 0 ].m_sampleRGBA = m_sampleRGBA;
    ri.m_uniform = pushConstant;
    rctx.renderer->render( ri );
}

void Image::update( const UpdateContext& )
{
    if ( !m_dataModel ) {
        return;
    }
    DataModel::size_type rev = m_dataModel->revision();
    if ( rev == m_revision ) {
        return;
    }
    m_revision = rev;
    setTexture( m_dataModel->texture( m_dataModel->current() ) );
}

void Image::setColor( math::vec4 c )
{
    m_color = c;
}

void Image::setTexture( Sprite s )
{
    assert( s.texture );
    m_sprite = s;
    m_sampleRGBA = Renderer::instance()->channelCount( s ) == 4;
}

}
