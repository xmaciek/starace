#include "screen_title.hpp"

#include "game_action.hpp"
#include "ui_property.hpp"

static constexpr Anchor c_defaultAnchor = Anchor::fCenter | Anchor::fMiddle;

ScreenTitle::ScreenTitle(
      Widget* rings
    , std::u32string_view mission, std::function<void()>&& mt
    , std::u32string_view customize, std::function<void()>&& ct
    , std::u32string_view settings, std::function<void()>&& st
    , std::u32string_view quit, std::function<void()>&& qt
) noexcept
: m_rings{ rings }
, m_glow{ color::dodgerBlue }
, m_newMission{ mission, c_defaultAnchor, std::move( mt ) }
, m_customize{ customize, c_defaultAnchor, std::move( ct ) }
, m_settings{ settings, c_defaultAnchor, std::move( st ) }
, m_quit{ quit, c_defaultAnchor, std::move( qt ) }
{
    assert( rings );
    uint16_t tab = 0;
    auto btns = widgets();
    m_currentButton = TabOrder<>{ 0, btns.size() };
    for ( auto* it : btns ) {
        it->setTabOrder( tab++ );
        it->setSize( { 320, 48 } );
    }
}

std::array<const Widget*, 4> ScreenTitle::widgets() const
{
    return {
        &m_newMission,
        &m_customize,
        &m_settings,
        &m_quit,
    };
}

std::array<Widget*, 4> ScreenTitle::widgets()
{
    return {
        &m_newMission,
        &m_customize,
        &m_settings,
        &m_quit,
    };
}

void ScreenTitle::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );

    auto btns = widgets();
    for ( const auto* it : btns ) {
        it->render( rctx );
    }
}

void ScreenTitle::update( const UpdateContext& uctx )
{
    assert( m_rings );
    m_rings->update( uctx );
}

bool ScreenTitle::onMouseEvent( const MouseEvent& event )
{
    auto btns = widgets();
    for ( auto* w : btns ) {
        auto* it = reinterpret_cast<Button*>( w );
        const bool b = it->isFocused();
        const bool c = it->onMouseEvent( event );
        if ( c == b ) { continue; }
        if ( b ) {
            it->setFocused( false );
        }
        else {
            m_currentButton = it->tabOrder();
            return true;
        }
    }
    m_currentButton.invalidate();
    return false;
}

void ScreenTitle::resize( math::vec2 s )
{
    auto btns = widgets();
    setSize( s );
    m_glow.setSize( s );
    Layout{ math::vec2{ s.x * 0.5f, s.y * 0.5f - 48 }, Layout::Flow::eVertical }( btns );
}

void ScreenTitle::onAction( Action a )
{
    if ( !a.digital ) { return; }

    auto btns = widgets();

    switch ( a.toA<GameAction>() ) {
    default: return;

    case GameAction::eMenuConfirm: {
        for ( auto* w : btns ) {
            auto* it = reinterpret_cast<Button*>( w );
            if ( it->isFocused() ) {
                it->trigger();
                return;
            }
        }
    } break;

    case GameAction::eMenuDown: {
        m_currentButton++;
        for ( auto* w : btns ) {
            auto* it = reinterpret_cast<Button*>( w );
            it->setFocused( it->tabOrder() == *m_currentButton );
        }
    } break;

    case GameAction::eMenuUp: {
        m_currentButton--;
        for ( auto* w : btns ) {
            auto* it = reinterpret_cast<Button*>( w );
            it->setFocused( it->tabOrder() == *m_currentButton );
        }
    } break;
    }
}
