#pragma once

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <iostream>
#include <fstream>
#include <span>
#include <memory_resource>
#include <string>
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

[[nodiscard]]
std::pmr::string readText( std::string_view path )
{
    std::ifstream ifs( (std::string)path, std::ios::binary | std::ios::ate );
    ifs.is_open() || cooker::error( "cannot open file for reading", path );
    auto size = ifs.tellg();
    ifs.seekg( 0 );
    std::pmr::string ret;
    ret.resize( static_cast<size_t>( size ) );
    ifs.read( ret.data(), size );
    return ret;
}

[[nodiscard]]
std::pmr::vector<uint8_t> read( std::ifstream& ifs, size_t size )
{
    std::pmr::vector<uint8_t> tmp( size );
    ifs.read( reinterpret_cast<char*>( tmp.data() ), static_cast<std::streamsize>( size ) );
    return tmp;
}

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
void read( std::ifstream& ifs, T& t )
{
    ifs.read( reinterpret_cast<char*>( &t ), (std::streamsize)sizeof( T ) );
}

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
void read( std::ifstream& ifs, std::pmr::vector<T>& t )
{
    ifs.read( reinterpret_cast<char*>( t.data() ), (std::streamsize)( sizeof( T ) * t.size() ) );
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

template <typename T>
requires ( std::is_trivially_copyable_v<T> )
std::span<T> cast( std::span<uint8_t> data )
{
    if ( data.size() % sizeof( T ) ) cooker::error( "not enough data to cast" );
    return std::span<T>{ reinterpret_cast<T*>( data.data() ), data.size() / sizeof( T ) };
}

}
