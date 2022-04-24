#include "ui_screen.hpp"

#include "button.hpp"
#include "colors.hpp"
#include "game_action.hpp"
#include "game_callbacks.hpp"
#include "label.hpp"
#include "nineslice.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"
#include "ui_spinbox.hpp"
#include "ui_image.hpp"

#include <Tracy.hpp>

#include <unordered_map>
#include <string_view>
using std::literals::string_view_literals::operator""sv;

// TODO load from file
static const std::unordered_map<std::string_view, std::u32string_view> g_locMap{
      { "$loc:gamename", ui::loc::title }
    , { "$loc:newgame", ui::loc::missionSelect }
    , { "$loc:customize", ui::loc::customize }
    , { "$loc:settings", ui::loc::settings }
    , { "$loc:quit", ui::loc::quit }
    , { "$loc:vsync", ui::loc::vsync }
    , { "$loc:return", ui::loc::return2 }
    , { "$loc:resolution", U"RESOLUTION" }
    , { "$loc:weaponPrimary", U"Primary weapon" }
    , { "$loc:weaponSecondary", U"Secondary weapon" }
    , { "$loc:jet", U"Frame" }
    , { "$loc:mission", U"Mission" }
    , { "$loc:missionSelect", U"Mission Select" }
    , { "$loc:missionCancel", U"Cancel Mission" }
    , { "$loc:missionReturn", U"Return" }
    , { "$loc:engage", U"Engage" }
    , { "$loc:resume", U"Resume" }
    , { "$loc:pause", U"PAUSE" }
};

static std::pmr::u32string locKeyToString( std::string_view strv )
{
    auto it = g_locMap.find( strv );
    assert( it != g_locMap.end() );
    return it == g_locMap.end() ? std::pmr::u32string{ strv.begin(), strv.end() } : std::pmr::u32string{ it->second.begin(), it->second.end() };
}

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

static UniquePointer<Widget> makeButton( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    auto button = UniquePointer<Button>{ alloc, [](){} };
    button->setAnchor( Anchor::fTop | Anchor::fLeft );
    button->setTabOrder( tabOrder );

    math::vec2 pos = button->position();
    math::vec2 size = button->size();
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { button->setText( locKeyToString( it.toString() ) ); continue; }
        if ( propName == "x"sv ) { pos.x = it.toFloat(); continue; }
        if ( propName == "y"sv ) { pos.y = it.toFloat(); continue; }
        if ( propName == "width"sv ) { size.x = it.toFloat(); continue; }
        if ( propName == "height"sv ) { size.y = it.toFloat(); continue; }
        if ( propName == "trigger"sv ) { button->setTrigger( fnKeyToFunction( it.toString() ) ); continue; }
        assert( !"unhandled Button element" );
    }
    button->setPosition( pos );
    button->setSize( size );
    return button;
}

namespace ui {

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
    DataModel* model = nullptr;
    const Font* fnt = g_uiProperty.fontMedium();
    math::vec2 pos{};
    std::pmr::u32string txt{};

    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { txt = locKeyToString( it.toString() ); continue; }
        if ( propName == "x"sv ) { pos.x = it.toFloat(); continue; }
        if ( propName == "y"sv ) { pos.y = it.toFloat(); continue; }
        if ( propName == "data"sv ) { model = dataKeyToModel( it.toString() ); continue; }
        assert( !"unhandled Label property" );
    }
    if ( model ) {
        return UniquePointer<Label>{ alloc, model, fnt, pos };
    }
    return UniquePointer<Label>{ alloc, txt, fnt, pos, color::white };
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
}

void Screen::update( const UpdateContext& uctx )
{
    ZoneScoped;
    m_anim = std::clamp( m_anim + uctx.deltaTime * 5.0f, 0.0f, 1.0f );
    for ( const auto& it : m_widgets ) {
        it->update( uctx );
    }
}

void Screen::onMouseEvent( const MouseEvent& e )
{
    MouseEvent event = e;
    event.position /= m_resize;
    event.position *= m_viewport;
    event.position -= m_offset;
    m_tabOrder.invalidate();
    for ( const auto& it : m_widgets ) {
        if ( it->onMouseEvent( event ) ) {
            m_tabOrder = it->tabOrder();
            return;
        }
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
    return ( wgt != m_widgets.end() ) ? **wgt : nullptr;
}

void Screen::changeFocus( uint16_t from, uint16_t to )
{
    Widget* wgt = findWidgetByTabOrder( from );
    if ( wgt ) { wgt->setFocused( false ); }

    wgt = findWidgetByTabOrder( to );
    if ( wgt ) { wgt->setFocused( true ); }
}

void Screen::onAction( Action a )
{
    ZoneScoped;
    if ( !a.digital ) { return; }
    const uint16_t prevIndex = *m_tabOrder;
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuUp: changeFocus( prevIndex, *--m_tabOrder ); break;
    case GameAction::eMenuDown: changeFocus( prevIndex, *++m_tabOrder ); break;
    default:
        if ( Widget* wgt = findWidgetByTabOrder( *m_tabOrder ) ) {
            wgt->onAction( a );
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
