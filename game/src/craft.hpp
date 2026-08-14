#pragma once

#include "units.hpp"

#include <math.hpp>

#include <array>

struct Craft {
    std::array<float, 3> m_enginePower{ 1.235_kN, 20_kN, 85_kN };
    float m_enginePowerTransferRate = 10_kN;
    float m_enginePowerCurrent = m_enginePower[ 1 ];
    float m_enginePowerTarget =  m_enginePower[ 1 ];
    float m_mass = 16875_kg;

    float acceleration( float drag ) const
    {
        return (float)meter * ( ( m_enginePowerCurrent - drag ) / m_mass );
    }

    void setEngineThrottle( float t )
    {
        m_enginePowerTarget = math::lerp( m_enginePower.front(), m_enginePower.back(), t );
    }

    void setEngineTrigger( float t )
    {
        if ( t < 0.0f ) {
            m_enginePowerTarget = math::lerp( m_enginePower[ 0 ], m_enginePower[ 1 ], 1.0f + t );
        }
        else {
            m_enginePowerTarget = math::lerp( m_enginePower[ 1 ], m_enginePower[ 2 ], t );
        }
    }

    void update( float dt )
    {
        float powerTransfer = dt * m_enginePowerTransferRate * math::clamp( m_enginePowerTarget - m_enginePowerCurrent, -1.0f, 1.0f );
        m_enginePowerCurrent = math::clamp( m_enginePowerCurrent + powerTransfer, m_enginePower[ 0 ], m_enginePower[ 2 ] );
    }
};