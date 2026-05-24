#include "decorator.hpp"

#include <ui/font.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <profiler.hpp>

#include <cassert>

namespace ui {

Decorator::Decorator( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
{
    m_pipeline = g_uiProperty.findMaterial( "9slice"_hash );
    switch ( ci.style ) {
    default: assert( !"unknown Decorator style" ); [[fallthrough]];
    case "flat"_hash: setSprite( g_uiProperty.sprite( "white"_hash ) ); return;
    case "image"_hash: setSprite( g_uiProperty.sprite( ci.path ) ); return;
    case "box"_hash:
    case "button"_hash:
        setSprite( g_uiProperty.sprite( ci.style ), 0.4375f );
        return;
    }
}

void Decorator::setSprite( const Sprite& sprite, float range )
{
    m_data = {
        .m_xywh = math::vec4{ math::vec2{}, size() },
        .m_uvwh = sprite,
        .m_spriteExtent{ sprite.w, sprite.h },
        .m_sampleRGBA = textureIs4Channel( sprite.texture ),
        .m_range = range,
    };
    m_texture = sprite.texture;
}

math::vec4 Decorator::color( const RenderContext& rctx ) const
{
    if ( m_color ) return *m_color;
    if ( isFocused() ) return rctx.colorFocus;
    return rctx.colorMain;
}

void Decorator::render( const RenderContext& rctx ) const
{
    ZoneScoped;

    PushConstant<Pipeline::e9Slice> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = color( rctx ),
        .m_instances{ m_data },
    };
    RenderInfo ri{
        .m_pipeline = m_pipeline,
        .m_verticeCount = PushConstant<Pipeline::e9Slice>::VERTICES,
        .m_uniform = pushConstant,
        .m_fragmentTexture = { m_texture },
    };
    rctx.renderer->render( ri );
}

}
