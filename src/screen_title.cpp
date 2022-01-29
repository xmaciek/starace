#include "screen_title.hpp"

#include "game_action.hpp"

ScreenTitle::ScreenTitle(
    Font* f
    , Texture t
    , Widget* rings
    , std::u32string_view mission, std::function<void()>&& mt
    , std::u32string_view customize, std::function<void()>&& ct
    , std::u32string_view quit, std::function<void()>&& qt
) noexcept
: m_rings{ rings }
, m_glow{ color::dodgerBlue }
, m_newMission{ mission, f, t, std::move( mt ) }
, m_customize{ customize, f, t, std::move( ct ) }
, m_quit{ quit, f, t, std::move( qt ) }
{
    assert( rings );
    uint16_t tab = 0;
    Button* btns[] = {
        &m_newMission,
        &m_customize,
        &m_quit,
    };
    for ( auto* it : btns ) {
        it->setAnchor( Anchor::fCenter | Anchor::fMiddle );
        it->setTabOrder( tab++ );
        it->setSize( { 320, 48 } );
    }
    btns[ *m_currentButton ]->setFocused( true );
}

void ScreenTitle::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );

    const Button* btns[] = {
        &m_newMission,
        &m_customize,
        &m_quit,
    };
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
    Button* btns[] = {
        &m_newMission,
        &m_customize,
        &m_quit,
    };
    bool ret = false;
    for ( auto* it : btns ) {
        const bool b = it->isFocused();
        const bool c = it->onMouseEvent( event );
        if ( c == b ) { continue; }
        if ( b ) {
            it->setFocused( false );
        }
        else {
            m_currentButton = it->tabOrder();
            ret = true;
        }
    }
    return ret;
}

void ScreenTitle::resize( math::vec2 s )
{
    Widget* btns[] = {
        &m_newMission,
        &m_customize,
        &m_quit,
    };
    setSize( s );
    m_glow.setSize( s );
    Layout{ math::vec2{ s.x * 0.5f, s.y * 0.5f - 48 }, Layout::Flow::eVertical }( std::begin( btns ), std::end( btns ) );
}

void ScreenTitle::onAction( Action a )
{
    if ( !a.digital ) { return; }

    Button* btns[] = {
        &m_newMission,
        &m_customize,
        &m_quit,
    };

    switch ( a.toA<GameAction>() ) {
    default: return;

    case GameAction::eMenuConfirm: {
        for ( auto* it : btns ) {
            if ( it->isFocused() ) {
                it->trigger();
                return;
            }
        }
    } break;

    case GameAction::eMenuDown: {
        m_currentButton++;
        for ( auto* it : btns ) {
            it->setFocused( it->tabOrder() == *m_currentButton );
        }
    } break;

    case GameAction::eMenuUp: {
        m_currentButton--;
        for ( auto* it : btns ) {
            it->setFocused( it->tabOrder() == *m_currentButton );
        }
    } break;
    }
}
