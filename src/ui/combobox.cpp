#include <ui/combobox.hpp>

#include <ui/font.hpp>
#include <ui/spritegen.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <string_view>

namespace ui {

class ComboBoxList : public Widget {
    DataModel* m_model = nullptr;
    ScrollIndex m_index{};
    float m_lineHeight = 0.0f;
    float m_topPadding = 0.0f;
    float m_botHeight = 8.0f;

    DataModel::size_type visibleCount() const;
    DataModel::size_type selectedIndex() const;

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        math::vec2 position{};
        math::vec2 size{};
    };


    ~ComboBoxList() noexcept = default;
    ComboBoxList() noexcept = default;
    ComboBoxList( const CreateInfo& p ) noexcept;


    virtual void render( RenderContext ) const override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
};

ComboBox::ComboBox( const ComboBox::CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .anchor = Anchor::fTop| Anchor::fLeft } }
, m_label{ Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .anchor = Anchor::fLeft | Anchor::fMiddle, } }
, m_value{ Label::CreateInfo{ .data = ci.data, .font = "medium"_hash, .anchor = Anchor::fRight | Anchor::fMiddle, } }
, m_data{ ci.data }
{
    math::vec2 s = size();
    m_label.setPosition( math::vec2{ 16.0f, s.y * 0.5f } );
    m_value.setPosition( math::vec2{ s.x - 16.0f, s.y * 0.5f } );
    setTabOrder( ci.tabOrder );
}

void ComboBox::open()
{
    g_uiProperty.requestModalWidget( UniquePointer<ComboBoxList>(
        std::pmr::get_default_resource(),
        ComboBoxList::CreateInfo{
            .model = g_uiProperty.dataModel( m_data ),
            .position = position() + offsetByAnchor(),
            .size = size(),
        }
    ) );
}

EventProcessing ComboBox::onAction( ui::Action action )
{
    switch ( action.a ) {
    case ui::Action::eMenuConfirm:
        open();
        return EventProcessing::eStop;
    default:
        return EventProcessing::eContinue;
    }
}

void ComboBox::update( const UpdateContext& uctx )
{
    m_value.update( uctx );
}

EventProcessing ComboBox::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    setFocused( testRect( p, pos, s ) );
    if ( !isFocused() ) {
        return EventProcessing::eContinue;
    }
    if ( event.type == MouseEvent::eClick ) {
        open();
    }

    return EventProcessing::eStop;
}

void ComboBox::render( RenderContext rctx ) const
{
    rctx.colorMain = isFocused() ? rctx.colorFocus : rctx.colorMain;
    NineSlice::render( rctx );
    const math::vec2 pos = position() + offsetByAnchor();

    rctx.model = math::translate( rctx.model, math::vec3{ pos.x, pos.y, 0.0f } );

    m_label.render( rctx );
    m_value.render( rctx );
}

ComboBoxList::ComboBoxList( const ComboBoxList::CreateInfo& ci ) noexcept
: Widget{}
, m_model{ ci.model }
{
    assert( ci.model );
    setPosition( ci.position + ci.size * math::vec2{ 0.0f, 1.0f } );

    float fSize = static_cast<float>( g_uiProperty.fontSmall()->height() );
    m_lineHeight = fSize + 4.0f;
    m_topPadding = fSize * 0.5f;
    m_index = ScrollIndex{ m_model->current(), m_model->size() };
    auto count = visibleCount();
    float midHeight = m_topPadding + m_lineHeight * static_cast<float>( count );

    setSize( math::vec2{ ci.size.x, midHeight + m_botHeight } );
}

void ComboBoxList::render( RenderContext rctx ) const
{
    const math::vec2 pos = position() + offsetByAnchor();
    const float width = size().x;
    rctx.model = math::translate( rctx.model, math::vec3{ pos.x, pos.y, 0.0f } );

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = 6u,
        .m_instanceCount = 7u,
    };
    pushData.m_fragmentTexture[ 1 ] = g_uiProperty.atlasTexture();
    PushConstant<Pipeline::eSpriteSequenceColors> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    auto count = visibleCount();
    float midSize = m_topPadding + m_lineHeight * static_cast<float>( count );

    using S = PushConstant<Pipeline::eSpriteSequenceColors>::Sprite;
    auto gibLine = []( float y, float w, float h, auto color, auto left, auto mid, auto right ) -> std::tuple<S,S,S>
    {
        const Font& atlasRef = *g_uiProperty.atlas();
        math::vec2 atlasExtent = atlasRef.extent();
        return {
            { .m_color = color, .m_xywh{ 4.0f, y, 8.0f, h },        .m_uvwh = atlasRef[ left ] / atlasExtent },
            { .m_color = color, .m_xywh{ 12.0f, y, w - 24.0f, h },  .m_uvwh = atlasRef[ mid ] / atlasExtent },
            { .m_color = color, .m_xywh{ w - 12.0f, y, 8.0f, h },   .m_uvwh = atlasRef[ right ] / atlasExtent },
        };
    };


    float yOffset = 0.0f;
    std::tie( pushConstant.m_sprites[ 0 ], pushConstant.m_sprites[ 1 ], pushConstant.m_sprites[ 2 ] )
        = gibLine( yOffset, width, midSize, rctx.colorMain, "left"_hash, "mid"_hash, "right"_hash );
    yOffset += midSize;

    std::tie( pushConstant.m_sprites[ 3 ], pushConstant.m_sprites[ 4 ], pushConstant.m_sprites[ 5 ] )
        = gibLine( yOffset, width, m_botHeight, rctx.colorMain, "botLeft2"_hash, "bot"_hash, "botRight2"_hash );

    const Font& atlasRef = *g_uiProperty.atlas();
    math::vec2 atlasExtent = atlasRef.extent();
    pushConstant.m_sprites[ 6 ] = {
        .m_color = rctx.colorFocus,
        .m_xywh{ 12.0f, m_topPadding + m_lineHeight * static_cast<float>( m_index.currentVisible() ), width - 24.0f, m_lineHeight },
        .m_uvwh = atlasRef[ "mid"_hash ] / atlasExtent,
    };
    rctx.renderer->push( pushData, &pushConstant );

    Label::CreateInfo ci{
        .font = "small"_hash,
        .position{ width - 16.0f, m_topPadding },
        .anchor = Anchor::fRight | Anchor::fTop,
    };
    for ( decltype( count ) i = 0; i < count; ++i ) {
        Label l{ ci };
        l.setText( m_model->at( i + m_index.offset() ) );
        l.render( rctx );
        ci.position.y += m_lineHeight;
    }
}

static auto pointToIndex( math::vec2 p, math::vec2 xy, float width, float lineHeight, auto count ) -> decltype( count )
{
    using IDX = decltype( count );
    math::vec2 pos = p - xy;
    if ( pos.x < 0.0f || pos.y < 0.0f ) { return 0; }
    if ( pos.x >= width ) { return 0; }

    IDX idx = static_cast<IDX>( pos.y / lineHeight );
    return ( idx >= count ) ? 0 : ( idx + 1 );
}

EventProcessing ComboBoxList::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    math::vec2 pos = position() + offsetByAnchor();
    math::vec2 s = size();
    const bool hitTest = testRect( p, pos, s );
    if ( !hitTest ){
        switch ( event.type ) {
        default: assert( !"unhandled mouse event" );
        case MouseEvent::eClick: return EventProcessing::eStop;
        case MouseEvent::eMove: return EventProcessing::eContinue;
        }
    }

    pos.y += m_topPadding;
    auto idx = pointToIndex( p, pos, s.x, m_lineHeight, visibleCount() );
    if ( idx ) {
        m_index.selectVisible( idx - 1 );
    }

    switch ( event.type ) {
    case MouseEvent::eClick:
        if ( idx ) m_model->select( m_index.current() );
        return EventProcessing::eStop;

    case MouseEvent::eMove:
        return EventProcessing::eContinue;

    default:
        assert( !"unhandled mouse event" );
        return EventProcessing::eStop;
    }

}

EventProcessing ComboBoxList::onAction( ui::Action action )
{
    if ( action.value == 0 ) { return EventProcessing::eContinue; }
    switch ( action.a ) {
    case ui::Action::eMenuConfirm:
        m_model->select( m_index.current() );
        [[fallthrough]];
    case ui::Action::eMenuCancel:
        return EventProcessing::eStop;

    case ui::Action::eMenuDown:
        m_index.increase();
        return EventProcessing::eContinue;

    case ui::Action::eMenuUp:
        m_index.decrease();
        return EventProcessing::eContinue;

    default:
        return EventProcessing::eContinue;
    }
}


DataModel::size_type ComboBoxList::visibleCount() const
{
    return std::min( m_model->size(), m_index.maxVisible() );
}

}
