#include <unicode/unicode.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <numeric>

namespace unicode {


BOM guessBOM( std::span<const uint8_t> data )
{
    static constexpr std::array<uint8_t, 4> utf32be{ (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0xFE, (uint8_t)0xFF };
    static constexpr std::array<uint8_t, 4> utf32le{ (uint8_t)0xFF, (uint8_t)0xFE, (uint8_t)0x00, (uint8_t)0x00 };
    static constexpr std::array<uint8_t, 2> utf16le{ (uint8_t)0xFF, (uint8_t)0xFE };
    static constexpr std::array<uint8_t, 2> utf16be{ (uint8_t)0xFE, (uint8_t)0xFF };

    if ( data.size() >= 4 ) {
        if ( std::equal( utf32be.begin(), utf32be.end(), data.begin() ) ) return BOM::UTF32BE;
        if ( std::equal( utf32le.begin(), utf32le.end(), data.begin() ) ) return BOM::UTF32LE;
    }
    if ( data.size() >= 2 ) {
        if ( std::equal( utf16be.begin(), utf16be.end(), data.begin() ) ) return BOM::UTF16BE;
        if ( std::equal( utf16le.begin(), utf16le.end(), data.begin() ) ) return BOM::UTF16LE;
    }
    return BOM::UTF8;
}


Transcoder::Transcoder( std::string_view s )
: Transcoder{ std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>( s.data() ), s.size() } }
{}

Transcoder::Transcoder( std::span<const uint8_t> s )
: data{ s }
{}


void Transcoder::advance()
{
    assert( !data.empty() );
    data = data.subspan( std::clamp((uint32_t)std::countl_one( data[ 0 ] ), 1u, 4u ) );
}

uint32_t Transcoder::length() const
{
    return std::accumulate( data.begin(), data.end(), 0u, []( uint32_t ret, uint8_t c )
    {
        switch ( std::countl_one( c ) ) {
        case 0:
        case 2:
        case 3:
        case 4:
            return ret + 1;
        default:
            return ret;
        }
    } );
}

char32_t Transcoder::value() const
{
    assert( !data.empty() );
    char32_t ret = 0;
    switch ( std::countl_one( data[ 0 ] ) ) {
    [[unlikely]]
    default:
        return data[ 0 ];
    [[likely]]
    case 0: return data[ 0 ];
    [[unlikely]]
    case 1: return data[ 0 ]; // error
    case 2:
        ret = data[ 0 ] & 0b0001'1111;
        ret <<= 6; ret |= data[ 1 ] & 0b00111111;
        return ret;
    case 3:
        ret = data[ 0 ] & 0b0000'1111;
        ret <<= 6; ret |= data[ 1 ] & 0b00111111;
        ret <<= 6; ret |= data[ 2 ] & 0b00111111;
        return ret;
    case 4:
        ret = data[ 0 ] & 0b0000'0111;
        ret <<= 6; ret |= data[ 1 ] & 0b00111111;
        ret <<= 6; ret |= data[ 2 ] & 0b00111111;
        ret <<= 6; ret |= data[ 3 ] & 0b00111111;
        return ret;
    }
}

}
