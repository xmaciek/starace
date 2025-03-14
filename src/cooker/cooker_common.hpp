#pragma once

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <iostream>
#include <fstream>
#include <span>
#include <memory_resource>
#include <vector>

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

[[nodiscard]]
std::ofstream openWrite( std::string_view path )
{
    std::ofstream ofs( (std::string)path, std::ios::binary );
    ofs.is_open() || cooker::error( "cannot open file for writing", path );
    return ofs;
}

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
void write( std::ofstream& ofs, const T& t )
{
    ofs.write( reinterpret_cast<const char*>( &t ), (std::streamsize)sizeof( T ) );
}

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
void write( std::ofstream& ofs, std::span<const T> t )
{
    ofs.write( reinterpret_cast<const char*>( t.data() ), (std::streamsize)( sizeof( T ) * t.size() ) );
}

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
void write( std::ofstream& ofs, const std::pmr::vector<T>& t )
{
    ofs.write( reinterpret_cast<const char*>( t.data() ), (std::streamsize)( sizeof( T ) * t.size() ) );
}


}
