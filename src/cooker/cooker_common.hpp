#pragma once

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <iostream>

namespace cooker {

static inline constexpr char FAIL[] = "[\x1b[31mFAIL\x1b[0m] ";
static inline constexpr char WARN[] = "[\x1b[33mWARN\x1b[0m] ";

static inline std::ostream& operator << ( std::ostream& o, char32_t ch )
{
    return o << std::hex << (uint32_t)ch;
}

[[noreturn]]
static inline bool error( std::string_view msg, const auto& arg )
{
    std::cout << FAIL << msg << " " << arg << std::endl;
    std::exit( 1 );
}

[[noreturn]]
static inline bool error( std::string_view msg )
{
    std::cout << FAIL << msg << std::endl;
    std::exit( 1 );
};

static inline void warning( std::string_view msg, const auto& arg )
{
    std::cout << WARN << msg << " " << arg << std::endl;
}

static inline void warning( std::string_view msg )
{
    std::cout << WARN << msg << std::endl;
}

}
