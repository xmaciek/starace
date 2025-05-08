#include "targeting.hpp"

#include "colors.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>
#include <ui/atlas.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>
#include <ui/label.hpp>

#include <array>

static constexpr std::array targetSprites{
    "reticle.t.topleft"_hash,
    "reticle.t.topright"_hash,
    "reticle.t.botleft"_hash,
    "reticle.t.botright"_hash,
};

static constexpr std::array targetSprites2{
    "reticle.t.up"_hash,
    "reticle.t.down"_hash,
    "reticle.t.left"_hash,
    "reticle.t.right"_hash,
};

static constexpr std::array reticleSprites{
    "reticle.r.topleft"_hash,
    "reticle.r.topright"_hash,
    "reticle.r.botleft"_hash,
    "reticle.r.botright"_hash,
};

Targeting::Targeting( const Targeting::CreateInfo& ci )
: m_callsigns{ ci.callsigns }
{
    m_texture = g_uiProperty.atlasTexture();
    auto unpack = []( Hash::value_type hash )
    {
        return std::get<0>( g_uiProperty.sprite( hash ) );
    };
    std::ranges::transform( targetSprites, m_xyuvTarget.begin(), unpack );
    std::ranges::transform( targetSprites2, m_xyuvTarget2.begin(), unpack );
    std::ranges::transform( reticleSprites, m_xyuvReticle.begin(), unpack );
}

void Targeting::render( const RenderContext& rctx ) const
{
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = 6,
    };
    pushData.m_fragmentTexture[ 1 ] = m_texture;
    using Sprite = ui::PushConstant<ui::Pipeline::eSpriteSequenceColors>::Sprite;
    ui::PushConstant<ui::Pipeline::eSpriteSequenceColors> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    auto pushSprite = [&pushData, &pushConstant, &rctx]( math::vec4 xywh, math::vec4 uvwh, math::vec2 pos2d, math::vec4 color )
    {
        xywh.x += pos2d.x;
        xywh.y += rctx.viewport.y - pos2d.y;
        pushConstant.m_sprites[ pushData.m_instanceCount++ ] = Sprite{ .m_color = color, .m_xywh = xywh, .m_uvwh = uvwh, };
        if ( pushData.m_instanceCount != pushConstant.INSTANCES ) [[likely]] return;
        rctx.renderer->push( pushData, &pushConstant );
        pushData.m_instanceCount = 0;
    };
    // const float rotAngle = math::lerp( 0.0f, 45.0_deg, m_state.value() );
    // math::mat4 modelMat = rctx.model;
    // modelMat = math::translate( modelMat, math::vec3{ pos2d.x, rctx.viewport.y - pos2d.y, 0.0f } );
    // modelMat = math::rotate( modelMat, rotAngle, axis::z );

    static const std::array<math::vec4, 4> a{
        math::vec4{ -68.0f, -68.0f, 16.0f, 16.0f },
        math::vec4{  52.0f, -68.0f, 16.0f, 16.0f },
        math::vec4{ -68.0f,  52.0f, 16.0f, 16.0f },
        math::vec4{  52.0f,  52.0f, 16.0f, 16.0f },
    };
    static const std::array<math::vec4, 4> b{
        math::vec4{ -16.0f, -16.0, 16.0f, 16.0f },
        math::vec4{   0.0f, -16.0, 16.0f, 16.0f },
        math::vec4{ -16.0f,   0.0, 16.0f, 16.0f },
        math::vec4{   0.0f,   0.0, 16.0f, 16.0f },
    };

    static const std::array<math::vec4, 4> c{
        math::vec4{ -8.0f, 150.0, 16.0f, 16.0f },
        math::vec4{ -8.0f, -166.0, 16.0f, 16.0f },
        math::vec4{ -166.0f, -8.0, 16.0f, 16.0f },
        math::vec4{ 150.0f, -8.0, 16.0f, 16.0f },
    };

    static const std::array<math::vec4, 4> d{
        math::vec4{ -8.0f, 8.0, 16.0f, 16.0f },
        math::vec4{ -8.0f, -24.0, 16.0f, 16.0f },
        math::vec4{ -24.0f, -8.0, 16.0f, 16.0f },
        math::vec4{ 8.0f, -8.0, 16.0f, 16.0f },
    };

    if ( const math::vec3 pos2d = project3dTo2d( rctx.camera3d, m_targetSignal.position, rctx.viewport ); isOnScreen( pos2d, rctx.viewport ) ) {
        const float value = m_state.value();
        static const math::vec4 white0 = color::white * math::vec4{ 1.0f, 1.0f, 1.0f, 0.0f };
        const math::vec4 targetColor = math::lerp( color::winScreen, color::crimson, value );
        const math::vec4 targetColor2 = math::lerp( white0, color::crimson, value );
        const math::vec2 position{ pos2d.x, pos2d.y };
        pushSprite( math::lerp( c[ 0 ], d[ 0 ], value ), m_xyuvTarget2[ 0 ], position, targetColor2 );
        pushSprite( math::lerp( c[ 1 ], d[ 1 ], value ), m_xyuvTarget2[ 1 ], position, targetColor2 );
        pushSprite( math::lerp( c[ 2 ], d[ 2 ], value ), m_xyuvTarget2[ 2 ], position, targetColor2 );
        pushSprite( math::lerp( c[ 3 ], d[ 3 ], value ), m_xyuvTarget2[ 3 ], position, targetColor2 );
        pushSprite( math::lerp( a[ 0 ], b[ 0 ], value ), m_xyuvTarget[ 0 ], position, targetColor );
        pushSprite( math::lerp( a[ 1 ], b[ 1 ], value ), m_xyuvTarget[ 1 ], position, targetColor );
        pushSprite( math::lerp( a[ 2 ], b[ 2 ], value ), m_xyuvTarget[ 2 ], position, targetColor );
        pushSprite( math::lerp( a[ 3 ], b[ 3 ], value ), m_xyuvTarget[ 3 ], position, targetColor );
    }
    const ui::RenderContext rr{
        .renderer = rctx.renderer,
        .model = rctx.model,
        .view = rctx.view,
        .projection = rctx.projection,
    };
    auto pushReticle = [this, &rr, targ = &a, &rctx, &pushSprite]( Signal signal )
    {
        const math::vec3 pos2d = project3dTo2d( rctx.camera3d, signal.position, rctx.viewport );
        if ( !isOnScreen( pos2d, rctx.viewport ) ) return;
        ui::Label lbl{ ui::Label::CreateInfo{
            .font = "medium"_hash,
            .position{ pos2d.x, rctx.viewport.y - pos2d.y - 96.0f },
            .anchor = ui::Anchor::fBottom | ui::Anchor::fCenter
        } };
        std::u32string txt{ U"B.O.T. " };
        auto l = static_cast<std::u32string_view>( m_callsigns[ signal.callsign ] );
        txt.insert( txt.end(), l.begin(), l.end() );
        lbl.setText( std::move( txt ) );
        lbl.onRender( rr );
        if ( m_targetSignal.callsign == signal.callsign ) return;
        const math::vec2 position{ pos2d.x, pos2d.y };
        pushSprite( (*targ)[ 0 ], m_xyuvReticle[ 0 ], position, color::lightSteelBlue );
        pushSprite( (*targ)[ 1 ], m_xyuvReticle[ 1 ], position, color::lightSteelBlue );
        pushSprite( (*targ)[ 2 ], m_xyuvReticle[ 2 ], position, color::lightSteelBlue );
        pushSprite( (*targ)[ 3 ], m_xyuvReticle[ 3 ], position, color::lightSteelBlue );
    };
    std::for_each( m_signals.begin(), m_signals.end(), std::move( pushReticle ) );
    if ( pushData.m_instanceCount != 0 ) [[likely]] {
        rctx.renderer->push( pushData, &pushConstant );
    }
}

void Targeting::setSignals( std::pmr::vector<Signal>&& vec )
{
    m_signals = std::move( vec );
}

void Targeting::update( const UpdateContext& uctx )
{
    m_state.update( uctx.deltaTime );
}

void Targeting::setTarget( Signal signal, float state )
{
    m_targetSignal = signal;
    m_state.setTarget( state );
}
