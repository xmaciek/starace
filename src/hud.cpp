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
    Widget* arr[] = {
        &m_score,
        &m_shots,
        &m_pool,
        &m_fps,
        &m_calc,
    };
    Layout{ { 4.0f, 4.0f }, Layout::eVertical }( std::begin( arr ), std::end( arr ) );

    const auto rightOf = []( const Label& w ) {
        return w.position() + glm::vec2{ w.size().x, 0.0f };
    };

    m_scoreValue.setPosition( rightOf( m_score ) );
    m_shotsValue.setPosition( rightOf( m_shots ) );
    m_poolValue.setPosition( rightOf( m_pool ) );
    m_fpsValue.setPosition( rightOf( m_fps ) );
    m_calcValue.setPosition( rightOf( m_calc ) );
}

void Hud::render( RenderContext rctx ) const
{
    const std::array arr = {
        &m_score,
        &m_scoreValue,
        &m_shots,
        &m_shotsValue,
        &m_pool,
        &m_poolValue,
        &m_fps,
        &m_fpsValue,
        &m_calc,
        &m_calcValue,
    };
    for ( const auto* it : arr ) {
        it->render( rctx );
    }
}

void Hud::update( const UpdateContext& )
{
    m_scoreValue.setText( intToUTF32( m_displayData->score ) );
    m_shotsValue.setText( intToUTF32( m_displayData->shots ) );
    m_poolValue.setText( intToUTF32( m_displayData->pool ) );
    m_fpsValue.setText( intToUTF32( m_displayData->fps ) );
    m_calcValue.setText( intToUTF32( m_displayData->calc ) );
}
