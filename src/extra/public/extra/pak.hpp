#pragma once

// pak format reference
// https://quakewiki.org/wiki/.pak

#include <cstdint>

namespace pak {

struct Header {
    static constexpr inline uint32_t MAGIC = 'KCAP';

    uint32_t magic = MAGIC;
    uint32_t offset = sizeof( Header );
    uint32_t size = 0;
};
static_assert( sizeof( Header ) == 12 );

struct alignas( 64 ) Entry {
    char name[ 56 ]{};
    uint32_t offset = 0;
    uint32_t size = 0;
};
static_assert( sizeof( Entry ) == 64 );

}
