#pragma once

#include <cstdint>
#include <string_view>

namespace csg {

struct Header {
    static const uint32_t MAGIC = ' GSC';
    static const uint32_t VERSION = 1;
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t count;
    char _[ 52 ];
};
static_assert( sizeof( Header ) == 64 );

struct Callsign {
    static const uint32_t CAPACITY = 31;
    char32_t str[ CAPACITY ]{};
    char32_t nullterm{};
    inline operator std::u32string_view () const
    {
        return std::u32string_view{ std::begin( str ) };
    }
};
static_assert( sizeof( Callsign ) == 128 );

}
