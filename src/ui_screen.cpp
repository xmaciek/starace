#include "ui_screen.hpp"

#include "button.hpp"
#include "colors.hpp"
#include "game_callbacks.hpp"
#include "label.hpp"
#include "nineslice.hpp"
#include "ui_combobox.hpp"
#include "ui_image.hpp"
#include <ui/property.hpp>
#include "ui_spinbox.hpp"

#include <Tracy.hpp>

#include <unordered_map>
#include <string_view>

using std::literals::string_view_literals::operator""sv;

static auto fnKeyToFunction( std::string_view strv )
{
    auto it = g_gameCallbacks.find( strv );
    assert( it != g_gameCallbacks.end() );
    return it == g_gameCallbacks.end() ? [](){} : it->second;
}

static auto dataKeyToModel( std::string_view strv )
{
    auto it = g_gameUiDataModels.find( strv );
    assert( it != g_gameUiDataModels.end() );
    assert( it->second );
    return it != g_gameUiDataModels.end() ? it->second : nullptr;
}


namespace ui {

static UniquePointer<Widget> makeButton( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    std::pmr::u32string text{};
    auto button = UniquePointer<Button>{ alloc, [](){} };
    button->setAnchor( Anchor::fTop | Anchor::fLeft );
    button->setTabOrder( tabOrder );

    math::vec2 pos = button->position();
    math::vec2 size = button->size();
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { text = g_uiProperty.localize( it.toString() ); continue; }
        if ( propName == "x"sv ) { pos.x = it.toFloat(); continue; }
        if ( propName == "y"sv ) { pos.y = it.toFloat(); continue; }
        if ( propName == "width"sv ) { size.x = it.toFloat(); continue; }
        if ( propName == "height"sv ) { size.y = it.toFloat(); continue; }
        if ( propName == "trigger"sv ) { button->setTrigger( fnKeyToFunction( it.toString() ) ); continue; }
        assert( !"unhandled Button element" );
    }
    button->setText( text );
    button->setPosition( pos );
    button->setSize( size );
    return button;
}

static UniquePointer<Widget> makeImage( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    math::vec2 position{};
    math::vec2 extent{};
    DataModel* model = nullptr;
    for ( const auto& property : entry ) {
        auto propName = *property;
        if ( propName == "x"sv ) { position.x = property.toFloat(); continue; }
        if ( propName == "y"sv ) { position.y = property.toFloat(); continue; }
        if ( propName == "width"sv ) { extent.x = property.toFloat(); continue; }
        if ( propName == "height"sv ) { extent.y = property.toFloat(); continue; }
        if ( propName == "data"sv ) { model = dataKeyToModel( property.toString() ); continue; }
        assert( !"unhandled Image property" );
    }
    return UniquePointer<Image>{ alloc, position, extent, model };
}

static UniquePointer<Widget> makeNineSlice( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    static constexpr std::array<uint32_t, 9> c_slices = {
        ui::AtlasSprite::eTopLeft,
        ui::AtlasSprite::eTop,
        ui::AtlasSprite::eTopRight,
        ui::AtlasSprite::eLeft,
        ui::AtlasSprite::eMid,
        ui::AtlasSprite::eRight,
        ui::AtlasSprite::eBotLeft2,
        ui::AtlasSprite::eBot,
        ui::AtlasSprite::eBotRight2,
    };
    math::vec2 position{};
    math::vec2 extent{};

    for ( const auto& property : entry ) {
        auto propName = *property;
        if ( propName == "x"sv ) { position.x = property.toFloat(); continue; }
        if ( propName == "y"sv ) { position.y = property.toFloat(); continue; }
        if ( propName == "width"sv ) { extent.x = property.toFloat(); continue; }
        if ( propName == "height"sv ) { extent.y = property.toFloat(); continue; }
        assert( !"unhandled NineSlice property" );
    }

    return UniquePointer<NineSlice>{ alloc
        , position
        , extent
        , Anchor::fTop | Anchor::fLeft
        , g_uiProperty.atlas()
        , c_slices
        , g_uiProperty.atlasTexture()
    };
}

static UniquePointer<Widget> makeLabel( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    Label::CreateInfo ci{
        .font = g_uiProperty.fontMedium(),
    };
    std::u32string text{};
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { text = g_uiProperty.localize( it.toString() ); continue; }
        if ( propName == "x"sv ) { ci.position.x = it.toFloat(); continue; }
        if ( propName == "y"sv ) { ci.position.y = it.toFloat(); continue; }
        if ( propName == "data"sv ) { ci.dataModel = dataKeyToModel( it.toString() ); continue; }
        assert( !"unhandled Label property" );
    }
    ci.text = text;
    return UniquePointer<Label>{ alloc, ci };
}

static UniquePointer<Widget> makeSpinBox( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    math::vec2 pos{};
    math::vec2 size{};
    DataModel* dataModel = nullptr;
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "x"sv ) { pos.x = it.toFloat(); continue; }
        if ( propName == "y"sv ) { pos.y = it.toFloat(); continue; }
        if ( propName == "width"sv ) { size.x = it.toFloat(); continue; }
        if ( propName == "height"sv ) { size.y = it.toFloat(); continue; }
        if ( propName == "data"sv ) { dataModel = dataKeyToModel( it.toString() ); continue; }
        assert( !"unhandled SpinBox element" );
    }

    assert( dataModel );
    UniquePointer<SpinBox> spinbox{ alloc, dataModel };
    spinbox->setTabOrder( tabOrder );
    spinbox->setPosition( pos );
    spinbox->setSize( size );
    return spinbox;
}


static UniquePointer<Widget> makeComboBox( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    ComboBox::CreateInfo ci{};
    Hash hash{};
    for ( const auto& it : entry ) {
        switch ( auto h = hash( *it ) ) {
        case "x"_hash: ci.position.x = it.toFloat(); continue;
        case "y"_hash: ci.position.y = it.toFloat(); continue;
        case "width"_hash: ci.size.x = it.toFloat(); continue;
        case "height"_hash: ci.size.y = it.toFloat(); continue;
        case "data"_hash: ci.model = dataKeyToModel( it.toString() ); continue;
        case "text"_hash: ci.text = g_uiProperty.localize( it.toString() ); continue;
        default:
            assert( !"unhandled Entry element" );
            continue;
        }
    }

    assert( ci.model );
    UniquePointer<ComboBox> ptr{ alloc, ci };
    ptr->setTabOrder( tabOrder );
    return ptr;
}

Screen::Screen( const cfg::Entry& entry ) noexcept
{
    ZoneScoped;
    std::pmr::memory_resource* alloc = std::pmr::get_default_resource();
    uint16_t tabOrderCount = 0;

    for ( const auto& it : entry ) {
        auto str = *it;
        if ( str == "width"sv ) { m_extent.x = it.toFloat(); continue; }
        if ( str == "height"sv ) { m_extent.y = it.toFloat(); continue; }
        if ( str == "Label"sv ) { m_widgets.emplace_back( makeLabel( alloc, it ) ); continue; }
        if ( str == "Button"sv ) { m_widgets.emplace_back( makeButton( alloc, it, tabOrderCount++ ) ); continue; }
        if ( str == "SpinBox"sv ) { m_widgets.emplace_back( makeSpinBox( alloc, it, tabOrderCount++ ) ); continue; }
        if ( str == "NineSlice"sv ) { m_widgets.emplace_back( makeNineSlice( alloc, it ) ); continue; }
        if ( str == "ComboBox"sv ) { m_widgets.emplace_back( makeComboBox( alloc, it, tabOrderCount++ ) ); continue; }
        if ( str == "Image"sv ) { m_widgets.emplace_back( makeImage( alloc, it ) ); continue; }
        assert( !"unhandled ui element" );
    }
    m_tabOrder = TabOrder<>{ 0, 0, tabOrderCount };
    if ( tabOrderCount == 0 ) { return; }
    changeFocus( Widget::c_invalidTabOrder, 0 );
}

void Screen::render( const RenderContext& rctx ) const
{
    ZoneScoped;
    auto r = rctx;
    r.projection = math::ortho( 0.0f, m_viewport.x, 0.0f, m_viewport.y, -1.0f, 1.0f );
    const math::mat4 view = math::translate( r.view, math::vec3{ m_offset.x, m_offset.y, 0.0f } );

    auto startPosistionForWidget = []( const auto& wgt, float viewportX, float extentX )
    {
        const math::vec2 pos = wgt->position() + wgt->offsetByAnchor();
        const float left = pos.x;
        const float right = extentX - ( pos.x + wgt->size().x );
        return ( left < right ) ? -viewportX : viewportX;
    };

    for ( const auto& it : m_widgets ) {
        const float startPos = startPosistionForWidget( it, m_viewport.x, m_extent.x );
        const float animX = math::nonlerp( startPos, 0.0f, m_anim );
        r.view = math::translate( view, math::vec3{ animX, 0.0f, 0.0f } );
        it->render( r );
    }

    if ( m_comboBoxList ) m_comboBoxList->render( r );
}

void Screen::update( const UpdateContext& uctx )
{
    ZoneScoped;
    if ( auto c = g_uiProperty.pendingModalComboBox(); c.model ) {
        g_uiProperty.requestModalComboBox( {}, {}, {} );
        ComboBoxList::CreateInfo ci{ .model = c.model, .position = c.position, .size = c.size };
        m_comboBoxList = UniquePointer<ComboBoxList>( std::pmr::get_default_resource(), ci );
    }
    m_anim = std::clamp( m_anim + uctx.deltaTime * 5.0f, 0.0f, 1.0f );
    for ( const auto& it : m_widgets ) {
        it->update( uctx );
    }
    if ( m_comboBoxList ) m_comboBoxList->update( uctx );
}

void Screen::onMouseEvent( const MouseEvent& e )
{
    MouseEvent event = e;
    event.position /= m_resize;
    event.position *= m_viewport;
    event.position -= m_offset;
    if ( m_comboBoxList ) {
        auto mouseProcessing = m_comboBoxList->onMouseEvent( event );
        if ( mouseProcessing == MouseEvent::eStop ) {
            m_comboBoxList = {};
        }
        return;
    }
    uint16_t prevWidget = *m_tabOrder;
    m_tabOrder.invalidate();
    for ( const auto& it : m_widgets ) {
        auto mouseProcessing = it->onMouseEvent( event );
        if ( mouseProcessing == MouseEvent::eContinue ) continue;
        m_tabOrder = it->tabOrder();
        changeFocus( prevWidget, *m_tabOrder );
        return;
    }

}

Widget* Screen::findWidgetByTabOrder( uint16_t tabOrder )
{
    if ( tabOrder == Widget::c_invalidTabOrder ) {
        return nullptr;
    }

    auto wgt = std::find_if( m_widgets.begin(), m_widgets.end()
        , [tabOrder]( const auto& wgt ) { return wgt->tabOrder() == tabOrder; }
    );
    return ( wgt != m_widgets.end() ) ? wgt->get() : nullptr;
}

void Screen::changeFocus( uint16_t from, uint16_t to )
{
    if ( from == to ) { return; }
    Widget* wgt = findWidgetByTabOrder( from );
    if ( wgt ) { wgt->setFocused( false ); }

    wgt = findWidgetByTabOrder( to );
    if ( wgt ) { wgt->setFocused( true ); }
}

void Screen::onAction( ui::Action action )
{
    ZoneScoped;
    if ( action.value == 0 ) { return; }

    if ( m_comboBoxList ) {
        if ( m_comboBoxList->onAction( action ) ) {
            m_comboBoxList = {};
        }
        return;
    }

    const uint16_t prevIndex = *m_tabOrder;
    switch ( action.a ) {
    case ui::Action::eMenuUp: changeFocus( prevIndex, *--m_tabOrder ); break;
    case ui::Action::eMenuDown: changeFocus( prevIndex, *++m_tabOrder ); break;
    default:
        if ( Widget* wgt = findWidgetByTabOrder( *m_tabOrder ) ) {
            wgt->onAction( action );
        }
        break;
    }
}

void Screen::resize( math::vec2 s )
{
    m_resize = s;
    const float aspect = m_resize.x / m_resize.y;
    const float srcAspect = m_extent.x / m_extent.y;
    m_viewport = ( aspect >= srcAspect )
        ? math::vec2{ m_extent.y * aspect, m_extent.y }
        : math::vec2{ m_extent.x, m_extent.x * ( m_resize.y / m_resize.x ) };
    m_offset = ( m_viewport - m_extent ) * 0.5f;
}

void Screen::show()
{
    m_anim = 0.0f;
}





}
