#pragma once

#include <bit>
#include <cstdint>

class alignas( 16 ) Xoroshiro128pp {
public:
    using result_type = uint64_t;

private:
    result_type s0 = 0;
    result_type s1 = 1;

public:
    static constexpr inline result_type max() { return -1ull; }
    static constexpr inline result_type min() { return 0ull; }

    constexpr inline Xoroshiro128pp( result_type a0, result_type a1 = 1 ) noexcept
    : s0{ a0 }
    , s1{ a1 }
    {
    }

    constexpr inline void discard( size_t i )
    {
        while ( i-- ) {
            s1 ^= s0;
            s0 = std::rotl( s0, 24 ) ^ s1 ^ ( s1 << 16 );
            s1 = std::rotl( s1, 37 );
        }
    }

    constexpr inline result_type operator () () noexcept
    {
        result_type result = std::rotl( ( s0 << 2 ) + s0, 7 );
        result = ( result << 3 ) + result;
        discard( 1 );
        return result;
    }
};

using Random = Xoroshiro128pp;
