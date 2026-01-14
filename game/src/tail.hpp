#pragma once

#include "units.hpp"
#include <engine/math.hpp>

#include <algorithm>
#include <array>

struct Tail {
    std::array<math::vec3, 8> points{};

    inline Tail() = default;
    inline Tail( math::vec3 p ) { std::ranges::fill( points, p ); }
    inline void prepend( math::vec3 p )
    {
        if ( math::distance( points[ 1 ], p ) >= 16.0_m ) [[unlikely]] {
            std::ranges::rotate( points, points.end() - 1 );
        }
        points[ 0 ] = p;
    }
};
