#include "targeting.hpp"

#include "circle.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>
#include <ui/atlas.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <array>

Targeting::Targeting( const Targeting::CreateInfo& ci )
{
    m_texture = g_uiProperty.atlasTexture();
    for ( uint32_t i = 0; i < 4; ++i ) {
        std::tie( m_xyuv[ i ], std::ignore, std::ignore, m_texture ) = g_uiProperty.sprite( ci.reticleSprites[ i ] );
    }
}

void Targeting::render( const RenderContext& rctx ) const
{
    if ( !m_pos ) {
        return;
    }
    const math::vec3 pos2d = project3dTo2d( rctx.camera3d, *m_pos, rctx.viewport );
    if ( !isOnScreen( pos2d, rctx.viewport ) ) {
        return;
    }
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = 6,
        .m_instanceCount = 4,
    };
    pushData.m_fragmentTexture[ 1 ] = m_texture;

    const float rotAngle = math::lerp( 0.0f, 45.0_deg, m_state.value() );
    math::mat4 modelMat = rctx.model;
    modelMat = math::translate( modelMat, math::vec3{ pos2d.x, rctx.viewport.y - pos2d.y, 0.0f } );
    modelMat = math::rotate( modelMat, rotAngle, axis::z );

    ui::PushConstant<ui::Pipeline::eSpriteSequence> pushConstant{
        .m_model = modelMat,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = math::lerp( color::winScreen, color::crimson, m_state.value() ),
    };

    static const std::array<math::vec4, 4> b{
        math::vec4{ -16.0f, -16.0, 16.0f, 16.0f },
        math::vec4{   0.0f, -16.0, 16.0f, 16.0f },
        math::vec4{ -16.0f,   0.0, 16.0f, 16.0f },
        math::vec4{   0.0f,   0.0, 16.0f, 16.0f },
    };

    static const std::array<math::vec4, 4> a{
        math::vec4{ -68.0f, -68.0f, 16.0f, 16.0f },
        math::vec4{  52.0f, -68.0f, 16.0f, 16.0f },
        math::vec4{ -68.0f,  52.0f, 16.0f, 16.0f },
        math::vec4{  52.0f,  52.0f, 16.0f, 16.0f },
    };
    using Sprite = ui::PushConstant<ui::Pipeline::eSpriteSequence>::Sprite;
    const float value = m_state.value();
    pushConstant.m_sprites[ 0 ] = Sprite{ .m_xywh = math::lerp( a[ 0 ], b[ 0 ], value ), .m_uvwh = m_xyuv[ 0 ] };
    pushConstant.m_sprites[ 1 ] = Sprite{ .m_xywh = math::lerp( a[ 1 ], b[ 1 ], value ), .m_uvwh = m_xyuv[ 1 ] };
    pushConstant.m_sprites[ 2 ] = Sprite{ .m_xywh = math::lerp( a[ 2 ], b[ 2 ], value ), .m_uvwh = m_xyuv[ 2 ] };
    pushConstant.m_sprites[ 3 ] = Sprite{ .m_xywh = math::lerp( a[ 3 ], b[ 3 ], value ), .m_uvwh = m_xyuv[ 3 ] };

    rctx.renderer->push( pushData, &pushConstant );
}

void Targeting::setPos( const math::vec3& v )
{
    m_pos = v;
}

void Targeting::hide()
{
    m_pos.reset();
}

void Targeting::update( const UpdateContext& uctx )
{
    m_state.update( uctx.deltaTime );
}

void Targeting::setState( float f )
{
    m_state.setTarget( f );
}

const math::vec3* Targeting::target() const
{
    return m_pos ? &*m_pos : nullptr;
}
