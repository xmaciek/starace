#include "ui_screen.hpp"

#include "colors.hpp"
#include "game_callbacks.hpp"
#include "progressbar.hpp"

#include <ui/button.hpp>
#include <ui/combobox.hpp>
#include <ui/footer.hpp>
#include <ui/image.hpp>
#include <ui/label.hpp>
#include <ui/nineslice.hpp>
#include <ui/property.hpp>
#include <ui/spinbox.hpp>

#include <Tracy.hpp>

#include <optional>
#include <string_view>

namespace ui {

template <typename T> void setX( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->position.x = e.toFloat(); };
template <typename T> void setY( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->position.y = e.toFloat(); };
template <typename T> void setW( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->size.x = e.toFloat(); };
template <typename T> void setH( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->size.y = e.toFloat(); };
template <typename T> void setData( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->data = Hash{}( e.toString() ); };
template <typename T> void setText( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->text = Hash{}( e.toString() ); };
template <typename T> void setFont( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->font = Hash{}( e.toString() ); };
template <typename T> void setSpriteId( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->spriteId = Hash{}( e.toString() ); };
template <typename T> void setSpriteSpacing( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->spriteSpacing = e.toFloat(); };
template <typename T> void setColor( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->color = Hash{}( e.toString() ); };
template <typename T> void setTrigger( void* ci, const cfg::Entry& e ) { reinterpret_cast<typename T::CreateInfo*>( ci )->trigger = Hash{}( e.toString() ); };

using F = std::tuple<Hash::value_type, void(*)( void*, const cfg::Entry& )>;
using W = std::tuple<Hash::value_type, UniquePointer<Widget>(*)( std::pmr::memory_resource*, const cfg::Entry&, std::span<const F>, uint16_t& ),std::span<const F>>;

template <typename T>
UniquePointer<Widget> makeWidget( std::pmr::memory_resource* allocator, const cfg::Entry& entry, std::span<const F> fields, uint16_t& tabOrder )
{
    typename T::CreateInfo ci{};
    if constexpr ( TabOrdering<T>::value ) {
        ci.tabOrder = tabOrder++;
    }
    Hash hash{};

    for ( auto&& property : entry ) {
        const auto h = hash( *property );
        for ( auto&& [ hh, set ] : fields ) {
            if ( hh != h ) continue;
            set( &ci, property );
        }
    }

    return UniquePointer<T>{ allocator, ci };
}

inline constexpr std::array PROGRESSBAR_FIELDS = {
    F{ "x"_hash, &setX<Progressbar> },
    F{ "y"_hash, &setY<Progressbar> },
    F{ "data"_hash, &setData<Progressbar> },
    F{ "spriteId"_hash, &setSpriteId<Progressbar> },
    F{ "spriteSpacing"_hash, &setSpriteSpacing<Progressbar> },
};

inline constexpr std::array IMAGE_FIELDS = {
    F{ "data"_hash, &setData<Image> },
    F{ "sprite"_hash, &setSpriteId<Image> },
    F{ "width"_hash, &setW<Image> },
    F{ "height"_hash, &setH<Image> },
    F{ "x"_hash, &setX<Image> },
    F{ "y"_hash, &setY<Image> },
    F{ "color"_hash, &setColor<Image> },
};

inline constexpr std::array LABEL_FIELDS = {
    F{ "data"_hash, &setData<Label> },
    F{ "text"_hash, &setText<Label> },
    F{ "x"_hash, &setX<Label> },
    F{ "y"_hash, &setY<Label> },
};

inline constexpr std::array SPINBOX_FIELDS = {
    F{ "data"_hash, &setData<SpinBox> },
    F{ "height"_hash, &setH<SpinBox> },
    F{ "width"_hash, &setW<SpinBox> },
    F{ "x"_hash, &setX<SpinBox> },
    F{ "y"_hash, &setY<SpinBox> },
};

inline constexpr std::array COMBOBOX_FIELDS = {
    F{ "data"_hash, &setData<ComboBox> },
    F{ "height"_hash, &setH<ComboBox> },
    F{ "text"_hash, &setText<ComboBox> },
    F{ "width"_hash, &setW<ComboBox> },
    F{ "x"_hash, &setX<ComboBox> },
    F{ "y"_hash, &setY<ComboBox> },
};

inline constexpr std::array BUTTON_FIELDS = {
    F{ "height"_hash, &setH<Button> },
    F{ "text"_hash, &setText<Button> },
    F{ "trigger"_hash, &setTrigger<Button> },
    F{ "width"_hash, &setW<Button> },
    F{ "x"_hash, &setX<Button> },
    F{ "y"_hash, &setY<Button> },
};

inline constexpr std::array WIDGETS = {
    W{ "Button"_hash, &makeWidget<Button>, BUTTON_FIELDS },
    W{ "ComboBox"_hash, &makeWidget<ComboBox>, COMBOBOX_FIELDS },
    W{ "Image"_hash, &makeWidget<Image>, IMAGE_FIELDS },
    W{ "Label"_hash, &makeWidget<Label>, LABEL_FIELDS },
    W{ "Progressbar"_hash, &makeWidget<Progressbar>, PROGRESSBAR_FIELDS },
    W{ "SpinBox"_hash, &makeWidget<SpinBox>, SPINBOX_FIELDS },
};

static UniquePointer<Widget> makeNineSlice( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    static constexpr std::array<ui::Atlas::hash_type, 9> SLICES = {
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
    math::vec2 position{};
    math::vec2 extent{};

    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "height"_hash: extent.y = property.toFloat(); continue;
        case "width"_hash: extent.x = property.toFloat(); continue;
        case "x"_hash: position.x = property.toFloat(); continue;
        case "y"_hash: position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled NineSlice property" );
            continue;
        }
    }

    return UniquePointer<NineSlice>{ alloc
        , position
        , extent
        , Anchor::fTop | Anchor::fLeft
        , SLICES
    };
}

static UniquePointer<Widget> makeFooter( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    ZoneScoped;
    auto makeInput = []( Hash::value_type h ) -> Action::Enum
    {
        switch ( h ) {
        case "eMenuApply"_hash: return Action::eMenuApply;
        case "eMenuCancel"_hash: return Action::eMenuCancel;
        default:
            assert( !"unknown input action" );
            return {};
        }
    };
    Hash hash{};
    std::pmr::vector<Footer::Entry> entries;
    Footer::CreateInfo ci{};
    for ( auto&& property : entry ) {
        switch ( hash( *property ) ) {
        case "Button"_hash: {
            auto& button = entries.emplace_back();
            for ( auto&& props : property ) {
                switch ( hash( *props ) ) {
                case "text"_hash: button.textId = hash( props.toString() ); continue;
                case "input"_hash: button.action = makeInput( hash( props.toString() ) ); continue;
                case "trigger"_hash: button.triggerId = hash( props.toString() ); continue;
                default:
                    assert( !"Footer.Button unhandled property" );
                    continue;
                }
            }
        }}
    }
    ci.entries = entries;
    return UniquePointer<Footer>{ alloc, ci };
}

static void makeList( std::pmr::memory_resource* alloc, const cfg::Entry& entry, std::pmr::vector<UniquePointer<Widget>>& retList, uint16_t& tabOrder )
{
    ZoneScoped;
    struct Element {
        enum class Type : uint32_t {
            invalid = 0,
            eButton,
            eComboBox,
        };
        Hash::value_type trigger{};
        Hash::value_type text{};
        Hash::value_type data{};
        Type type{};
        uint16_t tabOrder = 0;
    };

    math::vec2 xy{};
    math::vec2 elementSize{};
    math::vec2 advance{};
    float spacing = 0.0f;
    Hash::value_type direction = "down"_hash;

    std::pmr::vector<Element> elements;
    auto readElement = [&tabOrder]( const cfg::Entry& entry ) -> Element
    {
        Element e{};
        Hash hash{};
        switch ( hash( *entry ) ) {
        case "ComboBox"_hash:
            e.tabOrder = tabOrder++;
            e.type = Element::Type::eComboBox;
            for ( auto&& property : entry ) {
                switch ( hash( *property ) ) {
                case "text"_hash: e.text = hash( property.toString() ); continue;
                case "data"_hash: e.data = hash( property.toString() ); continue;
                default: assert( !"unhandled enum when reading List.entries.ComboBox" ); continue;
                }
            }
            return e;
        case "Button"_hash:
            e.tabOrder = tabOrder++;
            e.type = Element::Type::eButton;
            for ( auto&& property : entry ) {
                switch ( hash( *property ) ) {
                case "text"_hash: e.text = hash( property.toString() ); continue;
                case "trigger"_hash: e.trigger = hash( property.toString() ); continue;
                }
            }
            return e;
        default:
            assert( !"unhandled enum when reading List.entries" );
            break;
        }
        return e;
    };

    Hash hash{};
    for ( auto&& property : entry ) {
        switch ( hash( *property ) ) {
        case "x"_hash: xy.x = property.toFloat(); continue;
        case "y"_hash: xy.y = property.toFloat(); continue;
        case "spacing"_hash: spacing = property.toFloat(); continue;
        case "elementWidth"_hash: elementSize.x = property.toFloat(); continue;
        case "elementHeight"_hash: elementSize.y = property.toFloat(); continue;
        case "direction"_hash: direction = hash( property.toString() ); continue;
        case "entries"_hash:
            for ( auto&& e : property ) {
                elements.emplace_back( readElement( e ) );
            }
            continue;
        default:
            assert( !"unhandled enum when reading List" );
        }
    }

    switch ( direction ) {
        default:
            assert( !"unhandled enum List.direction" );
            [[fallthrough]];
        case "down"_hash: advance = { 0.0f, spacing + elementSize.y }; break;
        case "up"_hash: advance = { 0.0f, -( spacing + elementSize.y ) }; break;
        case "left"_hash: advance = { -( spacing + elementSize.x ), 0.0f }; break;
        case "right"_hash: advance = { spacing + elementSize.x, 0.0f }; break;
    };


    for ( auto&& it : elements ) {
        switch ( it.type ) {
        case Element::Type::eComboBox: {
            ComboBox::CreateInfo ci{
                .data = it.data,
                .text = it.text,
                .position = xy,
                .size = elementSize,
                .tabOrder = it.tabOrder
            };
            retList.emplace_back( UniquePointer<ComboBox>( alloc, ci ) );
        } break;
        case Element::Type::eButton: {
            Button::CreateInfo ci{
                .text = it.text,
                .trigger = it.trigger,
                .position = xy,
                .size = elementSize,
                .tabOrder = it.tabOrder
            };
            retList.emplace_back( UniquePointer<Button>( alloc, ci ) );
        } break;
        default:
            assert( !"Unhandled elemet type" );
            continue;
        }
        xy += advance;
    }
}

Screen::Screen( const cfg::Entry& entry ) noexcept
{
    ZoneScoped;
    std::pmr::memory_resource* alloc = std::pmr::get_default_resource();
    uint16_t tabOrderCount = 0;

    Hash hash{};
    for ( const auto& property : entry ) {
        const auto h = hash( *property );
        switch ( h ) {
        case "Footer"_hash: m_footer = makeFooter( alloc, property ); continue;
        case "List"_hash: makeList( alloc, property, m_widgets, tabOrderCount ); continue;
        case "NineSlice"_hash: m_widgets.emplace_back( makeNineSlice( alloc, property ) ); continue;
        case "height"_hash: m_extent.y = property.toFloat(); continue;
        case "width"_hash: m_extent.x = property.toFloat(); continue;
        default:
            break;
        }
        for ( auto&& [ hh, makeWgt, fields ] : WIDGETS ) {
            if ( h != hh ) continue;
            m_widgets.emplace_back( makeWgt( alloc, property, fields, tabOrderCount ) );
        }
    }
    if ( tabOrderCount == 0 ) { return; }
    m_tabOrder = TabOrder<>{ 0, 0, tabOrderCount };
    changeFocus( Widget::c_invalidTabOrder, 0 );
    if ( m_footer ) {
        m_footer->setPosition( math::vec2{ 48.0f, m_extent.y - 48.0f * 2.0f } );
        m_footer->setSize( math::vec2{ m_extent.x - 48.0f * 2.0f, 48.0f } );
    }
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

    if ( m_footer ) m_footer->render( r );
    if ( m_comboBoxList ) m_comboBoxList->render( r );
}

void Screen::updateInputRepeat( float dt )
{
    if ( m_repeatDirection == none ) return;
    m_inputRepeatDelay -= dt;
    if ( m_inputRepeatDelay > 0.0f ) return;
    m_inputRepeatDelay = 0.1618f;
    switch ( m_repeatDirection ) {
    case eUp: onAction( ui::Action{ .a = ui::Action::eMenuUp, .value = 0x7FFF } ); break;
    case eDown: onAction( ui::Action{ .a = ui::Action::eMenuDown, .value = 0x7FFF } ); break;
    case eLeft: onAction( ui::Action{ .a = ui::Action::eMenuLeft, .value = 0x7FFF } ); break;
    case eRight: onAction( ui::Action{ .a = ui::Action::eMenuRight, .value = 0x7FFF } ); break;
    default: break;
    }
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
    if ( m_footer ) m_footer->update( uctx );
    if ( m_comboBoxList ) m_comboBoxList->update( uctx );

    updateInputRepeat( uctx.deltaTime );
}

void Screen::onMouseEvent( const MouseEvent& e )
{
    MouseEvent event = e;
    event.position /= m_resize;
    event.position *= m_viewport;
    event.position -= m_offset;
    if ( m_comboBoxList ) {
        auto mouseProcessing = m_comboBoxList->onMouseEvent( event );
        if ( mouseProcessing == EventProcessing::eStop ) {
            m_comboBoxList = {};
        }
        return;
    }
    uint16_t prevWidget = *m_tabOrder;
    m_tabOrder.invalidate();
    for ( const auto& it : m_widgets ) {
        auto mouseProcessing = it->onMouseEvent( event );
        if ( mouseProcessing == EventProcessing::eContinue ) continue;
        m_tabOrder = it->tabOrder();
        changeFocus( prevWidget, *m_tabOrder );
        return;
    }

    if ( m_footer ) m_footer->onMouseEvent( event );
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
    auto testRepeat = []( int16_t value, RepeatDirection& e, RepeatDirection eConst, float& delay )
    {
        if ( e != eConst && value != 0 ) delay = 0.4f;
        e = ( value != 0 ) ? eConst : RepeatDirection::none;
    };
    switch ( action.a ) {
    case ui::Action::eMenuUp: testRepeat( action.value, m_repeatDirection, eUp, m_inputRepeatDelay ); break;
    case ui::Action::eMenuDown: testRepeat( action.value, m_repeatDirection, eDown, m_inputRepeatDelay ); break;
    case ui::Action::eMenuLeft: testRepeat( action.value, m_repeatDirection, eLeft, m_inputRepeatDelay ); break;
    case ui::Action::eMenuRight: testRepeat( action.value, m_repeatDirection, eRight, m_inputRepeatDelay ); break;
    default: break;
    }
    if ( action.value == 0 ) { return; }

    if ( m_comboBoxList ) {
        auto actionProcessing = m_comboBoxList->onAction( action );
        if ( actionProcessing == EventProcessing::eStop ) {
            m_comboBoxList = {};
        }
        return;
    }

    const uint16_t prevIndex = *m_tabOrder;
    std::optional<EventProcessing> eventResult;
    switch ( action.a ) {
    case ui::Action::eMenuUp:
        changeFocus( prevIndex, *--m_tabOrder );
        eventResult = EventProcessing::eStop;
        break;
    case ui::Action::eMenuDown:
        changeFocus( prevIndex, *++m_tabOrder );
        eventResult = EventProcessing::eStop;
        break;
    default:
        if ( Widget* wgt = findWidgetByTabOrder( *m_tabOrder ) ) {
            eventResult = wgt->onAction( action );
        }
        break;
    }
    if ( eventResult && ( *eventResult == EventProcessing::eStop ) ) {
        return;
    }

    if ( m_footer ) m_footer->onAction( action );
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

void Screen::show( math::vec2 size )
{
    resize( size );
    m_anim = 0.0f;
}





}
