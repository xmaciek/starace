#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace unicode {

enum class BOM : uint32_t {
    UTF8,
    UTF16LE,
    UTF16BE,
    UTF32LE,
    UTF32BE,
};

BOM guessBOM( std::span<const uint8_t> );

struct Transcoder {
    std::span<const uint8_t> data{};

    Transcoder() = default;
    Transcoder( std::span<const uint8_t> );
    Transcoder( std::string_view );

    char32_t value() const;
    uint32_t length() const;
    void advance();

    inline char32_t operator () ()
    {
        char32_t ret = value();
        advance();
        return ret;
    }

    inline void operator () ( char32_t& out ) { out = value(); advance(); }
    inline void operator () ( wchar_t& out ) { out = static_cast<wchar_t>( value() ); advance(); }

};

}
