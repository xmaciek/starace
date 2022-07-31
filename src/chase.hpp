#pragma once

#include <engine/math.hpp>

template <typename T = float, typename TVelocity = float>
class Chase {
public:
    T m_state{};
    T m_target{};
    TVelocity m_velocity{};

    void update( float dt )
    {
        T distance = m_target - m_state;
        TVelocity step = m_velocity * dt;
        m_state = math::length( distance ) <= math::length( step )
            ? m_target
            : m_state + math::normalize( distance ) * step;
    }

    T value() const
    {
        return m_state;
    }

    void setTarget( const T& t )
    {
        m_target = t;
    }
};
