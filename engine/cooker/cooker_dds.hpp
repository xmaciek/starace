#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <array>
#include <span>

namespace dds {

template <size_t TWeight>
inline uint8_t lerp( uint16_t e0, uint16_t e1 )
{
    static_assert( TWeight <= 64 );
    return static_cast<uint8_t>( ( ( 64 - TWeight ) * e0 + TWeight * e1 + 32 ) >> 6 );
}

struct BC4 {
    uint64_t alpha0 : 8;
    uint64_t alpha1 : 8;
    uint64_t aindexes : 48;
};

template <typename TPixel>
struct Swizzler {
    using PixelType = TPixel;
    using BlockType = std::array<PixelType, 16>;

    std::span<const PixelType> data;
    uint32_t blocksInRow = 0;
    uint32_t col = 0;

    Swizzler( std::span<const TPixel> d, uint32_t width )
    : data{ d }
    , blocksInRow( width / 4 )
    {
        assert( width % 4 == 0 );
        assert( data.size() % 16 == 0 );
    }

    inline BlockType operator () ()
    {
        assert( data.size() >= 16 );
        BlockType ret{};
        auto it = data.begin() + col * 4;
        auto advance = blocksInRow * 4;
        std::copy_n( it + advance * 0, 4, ret.begin() );
        std::copy_n( it + advance * 1, 4, ret.begin() + 4 );
        std::copy_n( it + advance * 2, 4, ret.begin() + 8 );
        std::copy_n( it + advance * 3, 4, ret.begin() + 12 );
        ++col;
        if ( col == blocksInRow ) {
            col = 0;
            data = data.subspan( blocksInRow * 16 );
        }
        return ret;
    }
};

inline BC4 compressor_bc4( std::span<const uint8_t, 16> block )
{
    BC4 ret{};
    auto [ min, max ] = std::minmax_element( block.begin(), block.end() );
    ret.alpha0 = *max;
    ret.alpha1 = *min;
    const std::array<uint8_t, 8> arr1{
        static_cast<uint8_t>( ret.alpha0 ),
        static_cast<uint8_t>( ret.alpha1 ),
        lerp<9>( ret.alpha0, ret.alpha1 ),
        lerp<18>( ret.alpha0, ret.alpha1 ),
        lerp<27>( ret.alpha0, ret.alpha1 ),
        lerp<37>( ret.alpha0, ret.alpha1 ),
        lerp<46>( ret.alpha0, ret.alpha1 ),
        lerp<55>( ret.alpha0, ret.alpha1 ),
    };
    auto nearestIndice = []( const std::array<uint8_t, 8>& alphas, uint8_t ref ) -> uint8_t
    {
        int dist = 0xFFFF;
        uint8_t indice = 0;
        for ( uint8_t i = 0; i < alphas.size(); ++i ) {
            int d = std::abs( (int)ref - (int)alphas[ i ] );
            if ( d == 0 ) return i;
            if ( d >= dist ) continue;
            dist = d;
            indice = i;
        }
        return indice;
    };

    // computing in reverse to preserve 3-bit indice ordering
    for ( auto it = block.rbegin(); it != block.rend(); ++it ) {
        ret.aindexes <<= 3;
        ret.aindexes |= nearestIndice( arr1, *it );
    }

    return ret;
};
}
