#include "ui_spinbox.hpp"

#include "ui_property.hpp"
#include "spritegen.hpp"
#include "game_pipeline.hpp"
#include "linear_atlas.hpp"
#include "colors.hpp"

#include <renderer/renderer.hpp>

#include <cassert>

static constexpr std::array<uint32_t, 9> c_slices = {
    ui::AtlasSprite::eTopLeft,
    ui::AtlasSprite::eTop,
    ui::AtlasSprite::eTopRight,
    ui::AtlasSprite::eLeft,
    ui::AtlasSprite::eMid,
    ui::AtlasSprite::eRight,
    ui::AtlasSprite::eBotLeft,
    ui::AtlasSprite::eBot,
    ui::AtlasSprite::eBotRight,
};

namespace ui {

SpinBox::SpinBox( DataModel* dataModel ) noexcept
: Widget{ {}, { 240.0f, 48.0f } }
, m_index{ 0, 0, dataModel->size() }
, m_model{ dataModel }
, m_colorL{ g_uiProperty.colorA() }
, m_colorR{ g_uiProperty.colorA() }
, m_label{ g_uiProperty.fontSmall(), Anchor::fCenter | Anchor::fMiddle, color::white }
{
    m_label.setText( m_model->at( value() ) );
}

SpinBox::SpinBox( char ) noexcept
: Widget{ {}, { 240.0f, 48.0f } }
, m_colorL{ g_uiProperty.colorA() }
, m_colorR{ g_uiProperty.colorA() }
, m_label{ g_uiProperty.fontSmall(), Anchor::fCenter | Anchor::fMiddle, color::white }
{
}

void SpinBox::render( RenderContext rctx ) const
{
    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eSpriteSequenceColors ),
        .m_verticeCount = spritegen::NineSlice2::count() + 12,
        .m_texture = g_uiProperty.atlasTexture(),
    };

    PushConstant<Pipeline::eSpriteSequenceColors> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };


    auto colorIt = pushConstant.m_color.begin();
    colorIt = std::fill_n( colorIt, 6, m_colorL );
    colorIt = std::fill_n( colorIt, spritegen::NineSlice2::count(), isFocused() ? color::lightSkyBlue : g_uiProperty.colorA() );
              std::fill_n( colorIt, 6, m_colorR );

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    const math::vec4 left = arrowLeft();
    const math::vec4 right = arrowRight();

    math::vec4 mid{ left.x + left.z, pos.y, s.x - ( left.z + right.z ), s.y };

    auto it = pushConstant.m_xyuv.begin();
    it = std::generate_n( it, 6, spritegen::Vert6{
        .m_xywh = left,
        .m_uvwh = g_uiProperty.atlas()->sliceUV( ui::AtlasSprite::eArrowLeft ),
    } );

    it = std::generate_n( it, spritegen::NineSlice2::count(), spritegen::NineSlice2{
        mid,
        g_uiProperty.atlas(),
        c_slices
    } );

    std::generate_n( it, 6, spritegen::Vert6{
        .m_xywh = right,
        .m_uvwh = g_uiProperty.atlas()->sliceUV( ui::AtlasSprite::eArrowRight ),
    } );

    rctx.renderer->push( pushBuffer, &pushConstant );

    const math::vec2 mv = pos + s * 0.5f;
    RenderContext r = rctx;
    r.model = math::translate( rctx.model, math::vec3{ mv.x, mv.y, 0.0f } );
    m_label.render( r );
}

bool SpinBox::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    setFocused( testRect( p, pos, s ) );
    if ( !isFocused() ) {
        m_colorL = g_uiProperty.colorA();
        m_colorR = g_uiProperty.colorA();
        return false;
    }


    const bool left = testRect( p, arrowLeft() );
    const bool right = testRect( p, arrowRight() );
    switch ( event.type ) {
    case MouseEvent::eMove:
        m_colorL = left ? color::lightSkyBlue : g_uiProperty.colorA();
        m_colorR = right ? color::lightSkyBlue : g_uiProperty.colorA();
        break;
    case MouseEvent::eClick:
        if ( left ) {
            m_index--;
            m_label.setText( m_model->at( value() ) );
            break;
        }
        if ( right ) {
            m_index++;
            m_label.setText( m_model->at( value() ) );
            break;
        }
        m_model->activate( value() );
        break;
    default:
        break;
    }

    return true;
}

void SpinBox::setModel( DataModel* model )
{
    m_index = TabOrder<DataModel::size_type>{ 0, 0, model->size() };
    m_model = model;
    m_label.setText( m_model->at( value() ) );
}

math::vec4 SpinBox::arrowLeft() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    Sprite arrow = g_uiProperty.atlas()->sprite( ui::AtlasSprite::eArrowLeft );
    return { pos.x, pos.y, arrow[ 2 ], arrow[ 3 ] };
}

math::vec4 SpinBox::arrowRight() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    Sprite arrow = g_uiProperty.atlas()->sprite( ui::AtlasSprite::eArrowRight );
    return { pos.x + s.x - static_cast<float>( arrow[ 2 ] ), pos.y, arrow[ 2 ], arrow[ 3 ] };
}

DataModel::size_type SpinBox::value() const
{
    assert( m_model );
    assert( *m_index < m_model->size() );
    return *m_index;
}

bool SpinBox::onAction( Action a )
{
    if ( !a.digital ) { return false; }
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuLeft: m_index--; break;
    case GameAction::eMenuRight: m_index++; break;
    case GameAction::eMenuConfirm:
        m_model->activate( value() );
        break;
    default:
        return false;
    }
    m_label.setText( m_model->at( value() ) );
    return true;
}


}
