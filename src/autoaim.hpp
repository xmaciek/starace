#pragma once

#include <engine/math.hpp>

class AutoAim {
    float m_angle = 15.0_deg;

public:

    inline bool matches( const math::vec3& position
    , const math::vec3& direction
    , const math::vec3& tgtPosition
    ) const
    {
        const math::vec3 directionToTgt = math::normalize( tgtPosition - position );
        const float angleToTarget = math::abs( math::acos( math::dot( direction, directionToTgt ) ) );
        return angleToTarget <= m_angle;
    }


    inline math::vec3 operator () ( float speed
        , const math::vec3& position
        , const math::vec3& tgtPosition
        , const math::vec3& tgtVelocity
    ) const
    {
        math::vec3 tgtMaybePos = tgtPosition;
        // 6 iterations is perfect enough
        for ( int i = 0; i < 10; ++i ) {
            const math::vec3 distance = tgtMaybePos - position;
            const float time = math::length( distance ) / speed;
            tgtMaybePos = tgtPosition + tgtVelocity * time;
        }

        return math::normalize( tgtMaybePos - position );
    }

};
