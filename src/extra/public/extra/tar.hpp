#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <span>
#include <array>

namespace tar::detail {

template <typename T, std::size_t TSize, bool TNullterm = true>
requires ( std::is_integral_v<T> && TSize > 1 )
struct alignas( 1 ) Octal {
    static constexpr std::size_t BUFFSIZE = TSize - (std::size_t)!!TNullterm;
    char data[ BUFFSIZE ]{};

    struct _{};
    [[no_unique_address]]
    std::conditional_t<TNullterm, char, _> nullterm{};

    constexpr inline void fromT( T t ) noexcept
    {
        for ( std::size_t i = BUFFSIZE; i--; ) {
            data[ i ] = '0' + ( t % 8 );
            t >>= 3;
        }
    }

    ~Octal() noexcept = default;
    constexpr Octal() noexcept = default;
    constexpr Octal( T t ) noexcept { fromT( t ); }
    constexpr Octal& operator = ( T t ) noexcept { fromT( t ); return *this; };

    constexpr bool operator == ( T t ) const noexcept { return static_cast<T>( *this ) == t; }

    constexpr operator T () const noexcept
    {
        T t{};
        for ( char c : data ) {
            if ( c < '0' ) [[unlikely]] break;
            if ( c > '7' ) [[unlikely]] break;
            t <<= 3;
            t += static_cast<T>( c - '0' );
        }
        return t;
    }

};
}

namespace tar {

static constexpr std::array<char, 6> MAGIC = { 'u', 's', 't', 'a', 'r', '\0' };
static constexpr std::array<char, 2> VERSION = { '0', '0' };

struct alignas( 1 ) Header {
    char name[ 100 ]{};
    detail::Octal<uint32_t, 8> mode;
    detail::Octal<uint32_t, 8> uid;
    detail::Octal<uint32_t, 8> gid;
    detail::Octal<std::size_t, 12> size;
    detail::Octal<uint32_t, 12> mtime;
    detail::Octal<uint32_t, 8> chksum;

    char typeflag = '0';
    char linkname[ 100 ]{};
    std::array<char, 6> magic = MAGIC;
    std::array<char, 2> version = VERSION;
    char uname[ 32 ]{};
    char gname[ 32 ]{};
    char devmajor[ 8 ]{};
    char devminor[ 8 ]{};
    char prefix[ 155 ]{};
    char padding[ 12 ]{};
};
static_assert( sizeof( Header ) == 512 );

using Eof = std::array<char, sizeof( Header )>;

inline uint32_t checksum( const Header& h ) noexcept
{
    auto chunkSum = []( const auto& chunk )
    {
        using char_type = unsigned char;
        const char_type* c = reinterpret_cast<const char_type*>( &chunk );
        const char_type* end = c + sizeof( chunk );
        uint32_t sum = 0;
        for ( ; c != end; ++c ) {
            sum += *c;
        }
        return sum;
    };
    uint32_t sum = 0;
    sum += chunkSum( h.name );
    sum += chunkSum( h.mode );
    sum += chunkSum( h.uid );
    sum += chunkSum( h.gid );
    sum += chunkSum( h.size );
    sum += chunkSum( h.mtime );
    sum += 256;
    sum += chunkSum( h.typeflag );
    sum += chunkSum( h.linkname );
    sum += chunkSum( h.magic );
    sum += chunkSum( h.version );
    sum += chunkSum( h.uname );
    sum += chunkSum( h.gname );
    sum += chunkSum( h.devmajor );
    sum += chunkSum( h.devminor );
    sum += chunkSum( h.prefix );

    return sum;
}

inline std::span<const uint8_t> data( const Header* h ) noexcept
{
    const uint8_t* begin = reinterpret_cast<const uint8_t*>( h + 1 );
    return std::span{ begin, begin + h->size };
}

inline bool isHeaderValid( const Header* h )
{
    return h
        && h->magic == MAGIC
        && h->version == VERSION
        && h->chksum == checksum( *h )
        ;
}

inline constexpr std::size_t alignedSize( std::size_t s ) noexcept
{
    return ( s + 511u ) & ~static_cast<std::size_t>( 511u );
}

inline const Header* next( const Header* h ) noexcept
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>( h + 1 );
    ptr += alignedSize( h->size );
    return reinterpret_cast<const Header*>( ptr );
}


}