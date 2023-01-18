#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <string_view>

struct Hash {
    using value_type = uint64_t;

    // NOTE: 64 still good enough - ascii chars are going to be part of hashed strings anyway,
    // also too lazy to fill all the indexes, copy-pasta from md5
    static constexpr uint32_t c_crc32Table[ 64 ] = {
        0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu, 0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
        0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu, 0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
        0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau, 0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
        0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu, 0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
        0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu, 0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
        0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u, 0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
        0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u, 0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
        0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u, 0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u,
    };

    static constexpr uint32_t crc32( const char* str, std::size_t len, uint32_t seed = 0xd76aa478u ) noexcept
    {
        uint32_t ret = seed;
        const char* end = str + len;
        while ( str != end ) {
            const uint32_t index = ( ret ^ (uint32_t)*str++ ) & 0xffu;
            ret = ( ret >> 8 ) ^ c_crc32Table[ index % 64 ];
        }
        return ~ret;
    }

    static constexpr uint32_t murmur3( const char* str, std::size_t len, uint32_t seed = 0xd76aa478u ) noexcept
    {
        auto scramble = []( uint32_t k ) -> uint32_t
        {
            k *= 0xcc9e2d51u;
            k = std::rotl( k, 15 );
            k *= 0x1b873593u;
            return k;
        };

        auto fetch4 = []( const char* str ) -> uint32_t
        {
            const uint32_t a = (uint32_t)str[ 0 ];
            const uint32_t b = (uint32_t)str[ 1 ];
            const uint32_t c = (uint32_t)str[ 2 ];
            const uint32_t d = (uint32_t)str[ 3 ];
            return a | ( b << 8 ) | ( c << 16 ) | ( d << 24 );
        };

        uint32_t h = seed;
        for ( std::size_t i = len >> 2; i; --i ) {
            const uint32_t k = fetch4( str );
            str += 4;
            h ^= scramble( k );
            h = std::rotl( h, 13 );
            h = h * 5 + 0xe6546b64u;
        }


        uint32_t k = 0;
        switch ( uint32_t i = len & 3 ) {
        case 3: k |= (uint32_t)str[ i - 3 ]; k <<= 8; [[fallthrough]];
        case 2: k |= (uint32_t)str[ i - 2 ]; k <<= 8; [[fallthrough]];
        case 1: k |= (uint32_t)str[ i - 1 ];
        }

        h ^= scramble( k );

        h ^= len % 32;
        h ^= h >> 16;
        h *= 0x85ebca6bu;
        h ^= h >> 13;
        h *= 0xc2b2ae35u;
        h ^= h >> 16;
        return h;
    }

    static constexpr value_type calc( const char* str, std::size_t len ) noexcept
    {
        // NOTE: what are the odds that 2 different hash algorithms are going to collide silmutanously for same data?
        value_type ret = crc32( str, len );
        ret <<= 32;
        ret |= murmur3( str, len );
        return ret;
    }

    constexpr value_type operator () ( std::string_view str ) const noexcept
    {
        return calc( str.data(), str.size() );
    }
};

constexpr Hash::value_type operator ""_hash( const char* str, std::size_t len ) noexcept
{
    return Hash::calc( str, len );
}
