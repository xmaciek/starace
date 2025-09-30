#include <ui/nineslice.hpp>

#include <ui/font.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <profiler.hpp>

#include <cassert>

namespace {
struct Generator {
    std::array<float, 3> m_w{};
    std::array<float, 3> m_h{};

    Generator( const math::vec2& wh, const ui::Sprite& s0, const ui::Sprite& s2, const ui::Sprite& s6 ) noexcept
    {
        m_w[ 0 ] = s0.w;
        m_w[ 1 ] = wh.x - ( s0.w + s2.w );
        m_w[ 2 ] = s2.w;

        m_h[ 0 ] = s0.h;
        m_h[ 1 ] = wh.y - ( s0.h + s6.h );
        m_h[ 2 ] = s6.h;
    }

    math::vec4 operator () ( uint32_t i ) const noexcept
    {
        switch ( i ) {
        case 0: return math::vec4{ 0.0f,                0.0f, m_w[ 0 ], m_h[ 0 ] }; break;
        case 1: return math::vec4{ m_w[ 0 ],            0.0f, m_w[ 1 ], m_h[ 0 ] }; break;
        case 2: return math::vec4{ m_w[ 0 ] + m_w[ 1 ], 0.0f, m_w[ 2 ], m_h[ 0 ] }; break;

        case 3: return math::vec4{ 0.0f,                m_h[ 0 ], m_w[ 0 ], m_h[ 1 ] }; break;
        case 4: return math::vec4{ m_w[ 0 ],            m_h[ 0 ], m_w[ 1 ], m_h[ 1 ] }; break;
        case 5: return math::vec4{ m_w[ 0 ] + m_w[ 1 ], m_h[ 0 ], m_w[ 2 ], m_h[ 1 ] }; break;

        case 6: return math::vec4{ 0.0f,                m_h[ 0 ] + m_h[ 1 ], m_w[ 0 ], m_h[ 2 ] }; break;
        case 7: return math::vec4{ m_w[ 0 ],            m_h[ 0 ] + m_h[ 1 ], m_w[ 1 ], m_h[ 2 ] }; break;
        case 8: return math::vec4{ m_w[ 0 ] + m_w[ 1 ], m_h[ 0 ] + m_h[ 1 ], m_w[ 2 ], m_h[ 2 ] }; break;

        default:
            assert( !"sprite id out of bounds" );
            return {};
        }
    }
};

constexpr std::array<Hash::value_type, 9> STYLE_BOX{
    "topLeft"_hash,
    "top"_hash,
    "topRight"_hash,
    "left"_hash,
    "mid"_hash,
    "right"_hash,
    "botLeft"_hash,
    "bot"_hash,
    "botRight"_hash,
};
constexpr std::array<Hash::value_type, 9> STYLE_BUTTON{
    "topLeft"_hash,
    "top"_hash,
    "topRight"_hash,
    "left"_hash,
    "mid"_hash,
    "right"_hash,
    "botLeft2"_hash,
    "bot"_hash,
    "botRight2"_hash,
};

}

namespace ui {

NineSlice::NineSlice( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
{
    std::array<Hash::value_type, 9> hashes;
    switch ( ci.style ) {
    default: assert( !"unknown NineSlice style" ); [[fallthrough]];
    case "box"_hash: hashes = STYLE_BOX; break;
    case "button"_hash: hashes = STYLE_BUTTON; break;
    }
    std::array<Sprite, 9> sprites;
    std::ranges::transform( hashes, sprites.begin(), []( auto h ) { return g_uiProperty.sprite( h ); } );
    std::ranges::transform( sprites, m_textures.begin(), []( const auto& s ) { return s.texture; } );
    std::ranges::sort( m_textures );
    std::fill( std::unique( m_textures.begin(), m_textures.end() ), m_textures.end(), Texture{} );

    Generator gen{ size(), sprites[ 0 ], sprites[ 2 ], sprites[ 6 ] };
    for ( uint32_t i = 0; i < 9; ++i ) {
        auto& sprite = m_sprites[ i ];
        sprite.m_xywh = gen( i );
        sprite.m_uvwh = sprites[ i ];
        sprite.m_whichAtlas = (uint32_t)std::distance( m_textures.begin(), std::ranges::find( m_textures, sprites[ i ].texture ) );
        // sprite.m_sampleRGBA = TODO
    }

}

void NineSlice::render( const RenderContext& rctx ) const
{
    ZoneScoped;

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = Uniform::VERTICES,
        .m_instanceCount = 9,
    };

    Uniform pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = isFocused() ? rctx.colorFocus : rctx.colorMain,
    };

    std::ranges::copy( m_textures, pushData.m_fragmentTexture.begin() );
    std::ranges::copy( m_sprites, pushConstant.m_sprites.begin() );

    rctx.renderer->push( pushData, &pushConstant );
}

}
