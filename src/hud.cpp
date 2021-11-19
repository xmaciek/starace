#include "hud.hpp"

#include "colors.hpp"
#include "utils.hpp"

#include <iostream>

Hud::Hud( const HudData* displayData, Font* font ) noexcept
: m_displayData{ displayData }
, m_score{ U"Score: ", font, {}, color::winScreen }
, m_scoreValue{ U"", font, {}, color::winScreen }
, m_shots{ U"Shots: ", font, {}, color::winScreen }
, m_shotsValue{ U"", font, {}, color::winScreen }
, m_pool{ U"Pool: ", font, {}, color::winScreen }
, m_poolValue{ U"", font, {}, color::winScreen }
, m_fps{ U"FPS: ", font, {}, color::winScreen }
, m_fpsValue{ U"", font, {}, color::winScreen }
, m_calc{ U"Calculated: ", font, {}, color::winScreen }
, m_calcValue{ U"", font, {}, color::winScreen }
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
    m_scoreValue.setText( intToUTF32( m_displayData->score ) );
    m_shotsValue.setText( intToUTF32( m_displayData->shots ) );
    m_poolValue.setText( intToUTF32( m_displayData->pool ) );
    m_fpsValue.setText( intToUTF32( m_displayData->fps ) );
    m_calcValue.setText( intToUTF32( m_displayData->calc ) );
    m_layout.update( uctx );
}

