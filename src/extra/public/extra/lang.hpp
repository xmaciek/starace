#pragma once

#include <shared/hash.hpp>
#include <cstdint>
#include <tuple>

namespace lang {
struct KeyType {
    Hash::value_type hash = 0;
    uint32_t offset = 0;
    uint32_t size = 0;

    inline KeyType() = default;
    inline KeyType( Hash::value_type h, uint32_t o, uint32_t s )
    : hash{ h }
    , offset{ o }
    , size{ s }
    {}
    constexpr inline bool operator < ( const KeyType& rhs ) const noexcept
    {
        return hash < rhs.hash;
    }
};
static_assert( sizeof( KeyType ) == 12 );

struct Header {
    static constexpr inline uint32_t MAGIC = 'GNAL';
    static constexpr inline uint32_t VERSION = 2;

    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    char id[ 8 ]{};
    uint32_t count = 0;
    uint32_t string = 0;
};
static_assert( sizeof( Header ) == 24 );

}
