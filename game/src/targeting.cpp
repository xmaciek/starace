#include "targeting.hpp"

#include "colors.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>
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
    auto unpack = []( Hash::value_type hash )
    {
        return g_uiProperty.sprite( hash );
    };
    std::ranges::transform( targetSprites, m_xyuvTarget.begin(), unpack );
    std::ranges::transform( targetSprites2, m_xyuvTarget2.begin(), unpack );
    std::ranges::transform( reticleSprites, m_xyuvReticle.begin(), unpack );
}

static const std::array<math::vec4, 4> A{
    math::vec4{ -68.0f, -68.0f, 16.0f, 16.0f },
    math::vec4{  52.0f, -68.0f, 16.0f, 16.0f },
    math::vec4{ -68.0f,  52.0f, 16.0f, 16.0f },
    math::vec4{  52.0f,  52.0f, 16.0f, 16.0f },
};
static const std::array<math::vec4, 4> B{
    math::vec4{ -16.0f, -16.0, 16.0f, 16.0f },
    math::vec4{   0.0f, -16.0, 16.0f, 16.0f },
    math::vec4{ -16.0f,   0.0, 16.0f, 16.0f },
    math::vec4{   0.0f,   0.0, 16.0f, 16.0f },
};

static const std::array<math::vec4, 4> C{
    math::vec4{ -8.0f, 150.0, 16.0f, 16.0f },
    math::vec4{ -8.0f, -166.0, 16.0f, 16.0f },
    math::vec4{ -166.0f, -8.0, 16.0f, 16.0f },
    math::vec4{ 150.0f, -8.0, 16.0f, 16.0f },
};

static const std::array<math::vec4, 4> D{
    math::vec4{ -8.0f, 8.0, 16.0f, 16.0f },
    math::vec4{ -8.0f, -24.0, 16.0f, 16.0f },
    math::vec4{ -24.0f, -8.0, 16.0f, 16.0f },
    math::vec4{ 8.0f, -8.0, 16.0f, 16.0f },
};

static std::array<math::vec4, 4> lerp( const auto& a, const auto& b, float n )
{
    std::array<math::vec4, 4> ret;
    for ( uint32_t i = 0; i < 4; ++i ) ret[ i ] = math::lerp( a[ i ], b[ i ], n );
    return ret;
}

void Targeting::render( const RenderContext& rctx ) const
{
    using PushConstant = ui::PushConstant<ui::Pipeline::eSpriteSequenceColors>;
    static_assert( PushConstant::INSTANCES >= 8 );
    using Sprite = PushConstant::Sprite;

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    RenderInfo ri{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = 4,
        .m_uniform = pushConstant,
    };

    auto pushReticle = [&ri, &pushConstant, &rctx]( const std::array<ui::Sprite, 4>& sprite, const std::array<math::vec4, 4>& geo, math::vec2 pos2d, math::vec4 color )
    {
        math::vec4 offset{ pos2d.x, rctx.viewport.y - pos2d.y, 0.0f, 0.0f };
        std::array<Texture, 4> tex;
        std::ranges::transform( sprite, tex.begin(), []( const auto& s ) { return s.texture; } );
        std::fill( std::unique( tex.begin(), tex.end() ), tex.end(), Texture{} );
        std::ranges::copy( tex, ri.m_fragmentTexture.begin() );

        std::array<uint32_t, 4> tex2;
        std::ranges::transform( sprite, tex2.begin(), [&tex]( const auto& s )
        {
            return (uint32_t)std::distance( tex.begin(), std::ranges::find( tex, s.texture ) );
        });
        for ( uint32_t i = 0; i < 4; ++i ) {
            pushConstant.m_sprites[ i ] = Sprite{
                .m_color = color,
                .m_xywh = geo[ i ] + offset,
                .m_uvwh = sprite[ i ],
                .m_whichAtlas = tex2[ i ],
            };
        }
        rctx.renderer->render( ri );
    };

    const ui::RenderContext rr{
        .renderer = rctx.renderer,
        .model = rctx.model,
        .view = rctx.view,
        .projection = rctx.projection,
    };
    auto renderSignal = [this, &rr, &rctx, &pushReticle, value=m_state.value()]( Signal signal )
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
        if ( m_targetSignal.callsign != signal.callsign ) [[likely]] {
            pushReticle( m_xyuvReticle, A, math::vec2{ pos2d.x, pos2d.y }, color::lightSteelBlue );
            return;
        }
        static const math::vec4 white0 = color::white * math::vec4{ 1.0f, 1.0f, 1.0f, 0.0f };
        const math::vec4 targetColor = math::lerp( color::winScreen, color::crimson, value );
        const math::vec4 targetColor2 = math::lerp( white0, color::crimson, value );
        const math::vec2 position{ pos2d.x, pos2d.y };
        pushReticle( m_xyuvTarget2, lerp( C, D, value ), position, targetColor2 );
        pushReticle( m_xyuvTarget, lerp( A, B, value ), position, targetColor );
    };
    std::ranges::for_each( m_signals, std::move( renderSignal ) );
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
