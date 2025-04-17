#include <ui/animframe.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

namespace ui {

AnimFrame::AnimFrame( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_color{ g_uiProperty.color( ci.color ) }
{
    if ( ci.data ) { m_dataModel = g_uiProperty.dataModel( ci.data ); }
    for ( auto&& it : ci.frames ) {
        if ( !it ) continue;
        std::tie( m_uvwh[ m_count++ ], std::ignore, std::ignore, std::ignore ) = g_uiProperty.sprite( it );
    }
    m_texture = g_uiProperty.atlasTexture();
    assert( m_count );
}

void AnimFrame::render( const RenderContext& rctx ) const
{
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = 6u,
    };
    pushData.m_fragmentTexture[ 1 ] = m_texture;

    using PushConstant = ui::PushConstant<ui::Pipeline::eSpriteSequence>;
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };

    math::vec2 s = size();
    pushConstant.m_sprites[ 0 ] = PushConstant::Sprite{
        .m_xywh{ 0.0f, 0.0f, s.x, s.y },
        .m_uvwh = m_uvwh[ m_index ],
    };

    rctx.renderer->push( pushData, &pushConstant );
}

void AnimFrame::update( const UpdateContext& uctx )
{
    if ( m_dataModel ) {
        const auto rev = m_dataModel->revision();
        if ( rev == m_revision ) { return; }
        m_revision = rev;
        m_spinner = m_dataModel->atF( m_dataModel->current() );
    } else {
        m_spinner = std::fmod( m_spinner + uctx.deltaTime * 2.0f, 1.0f );
    }
    auto f = static_cast<float>( m_count ) * m_spinner;
    m_index = std::min<uint32_t>( static_cast<uint32_t>( f ), m_count - 1 );
}

}
