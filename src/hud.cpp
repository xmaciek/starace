#include "hud.hpp"

#include "colors.hpp"
#include <iostream>

Hud::Hud( const HudData* displayData, Font* font ) noexcept
: m_displayData{ displayData }
, m_score{ "Score: ", font, {}, color::winScreen }
, m_scoreValue{ "", font, {}, color::winScreen }
, m_shots{ "Shots: ", font, {}, color::winScreen }
, m_shotsValue{ "", font, {}, color::winScreen }
, m_pool{ "Pool: ", font, {}, color::winScreen }
, m_poolValue{ "", font, {}, color::winScreen }
, m_fps{ "FPS: ", font, {}, color::winScreen }
, m_fpsValue{ "", font, {}, color::winScreen }
, m_calc{ "Calculated: ", font, {}, color::winScreen }
, m_calcValue{ "", font, {}, color::winScreen }
{
    m_layout.setPosition( { 4.0f, 4.0f } );

    m_line1.add( &m_score );
    m_line1.add( &m_scoreValue );

    m_line2.add( &m_shots );
    m_line2.add( &m_shotsValue );

    m_line3.add( &m_pool );
    m_line3.add( &m_poolValue );

    m_line4.add( &m_fps );
    m_line4.add( &m_fpsValue );

    m_line5.add( &m_calc );
    m_line5.add( &m_calcValue );

    m_layout.add( &m_line1 );
    m_layout.add( &m_line2 );
    m_layout.add( &m_line3 );
    m_layout.add( &m_line4 );
    m_layout.add( &m_line5 );
}

void Hud::render( RenderContext rctx ) const
{
    m_layout.render( rctx );
}

void Hud::update( const UpdateContext& uctx )
{
    m_scoreValue.arg( m_displayData->score );
    m_shotsValue.arg( m_displayData->shots );
    m_poolValue.arg( m_displayData->pool );
    m_fpsValue.arg( m_displayData->fps );
    m_calcValue.arg( m_displayData->calc );
    m_layout.update( uctx );
}

