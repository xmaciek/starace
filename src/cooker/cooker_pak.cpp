#include <extra/pak.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

[[noreturn]]
static bool exitOnFailed( std::string_view msg, std::string_view arg )
{
    std::cout << FAIL << msg << " " << arg << std::endl;
    std::exit( 1 );
}

static std::string preparePakPath( const std::string& str, int skipCount )
{
    auto stat = std::filesystem::status( str );
    std::filesystem::exists( stat ) || exitOnFailed( "path don't exists:", str );
    std::filesystem::is_regular_file( stat ) || exitOnFailed( "path is not file:", str );

    auto rit = std::find_if( str.rbegin(), str.rend(), [skipCount]( char c ) mutable
    {
        switch ( c ) {
        case '/':
        case '\\': // TODO test pathing on windows
        return skipCount-- == 0;
        default: return false;
        }
    } );
    auto seekit = std::distance( rit, str.rend() );
    auto it = str.begin();
    std::advance( it, seekit );
    auto dist = std::distance( it, str.end() );
    ( static_cast<uint32_t>( dist ) < sizeof( pak::Entry::name ) ) || exitOnFailed( "path too long to fit into Header.path field", str );

    std::string ret;
    ret.resize( static_cast<std::string::size_type>( dist ) );
    std::copy( it, str.end(), ret.begin() );
    return ret;
}

std::vector<std::pair<std::string, std::string>> preparePathPairs( std::string_view list )
{
    std::vector<std::pair<std::string,std::string>> ret;
    std::istringstream input;
    input.str( (std::string)list );

    for ( std::string filePath; std::getline( input, filePath, ';' ); ) {
        std::string pakPath = preparePakPath( filePath, 1 );
        ret.emplace_back( std::make_pair( filePath, std::move( pakPath ) ) );
    }
    auto cmp = []( const auto& lhs, const auto& rhs ) { return lhs.second < rhs.second; };
    std::sort( ret.begin(), ret.end(), cmp );
    return ret;
}

template <typename T>
requires std::is_trivially_copyable_v<T>
std::ofstream& operator <<= ( std::ofstream& o, const std::vector<T>& v )
{
    auto size = static_cast<std::streamsize>( sizeof( T ) * v.size() );
    o.write( reinterpret_cast<const char*>( v.data() ), size );
    return o;
}

std::ofstream& operator <<= ( std::ofstream& o, const pak::Header& v )
{
    o.write( reinterpret_cast<const char*>( &v ), static_cast<std::streamsize>( sizeof( v ) ) );
    return o;
}

bool strcpyIntoBuffer( const auto& src, auto& dst )
{
    if ( std::size( src ) > std::size( dst ) ) return false;
    std::copy( std::begin( src ), std::end( src ), std::begin( dst ) );
    return true;
}

int main( int argc, const char** argv )
{
    ( argc == 3 ) || exitOnFailed( "expected 2 arguments: archive.pak \"semi;colon;separated;list;of;files\"", "" );

    auto fileList = preparePathPairs( argv[ 2 ] );
    pak::Header header{};
    std::vector<pak::Entry> entries;
    entries.reserve( fileList.size() );

    std::ofstream ofs( argv[ 1 ], std::ios::binary );
    ofs.is_open() || exitOnFailed( "failed to open file for writing", argv[ 1 ] );
    ofs <<= header;

    std::vector<char> tmp;
    char padding[ 16 ]{};
    for ( auto&& [ src, dst ] : fileList ) {
        auto& entry = entries.emplace_back();
        strcpyIntoBuffer( dst, entry.name ) || exitOnFailed( "path too long to fit into .name field", dst );

        std::ifstream ifs( src, std::ios::binary | std::ios::ate );
        ifs.is_open() || exitOnFailed( "cannot open file for reading:", src );

        const std::streamsize size = ifs.tellg();
        tmp.resize( static_cast<std::vector<char>::size_type>( size ) );
        ifs.seekg( 0 );
        ifs.read( tmp.data(), size );
        ifs.close();

        const uint32_t paddingPos = static_cast<uint32_t>( ofs.tellp() );
        ofs.write( padding, static_cast<std::streamsize>( ( ( paddingPos + 15ul ) & ~15ul ) - paddingPos ) );

        entry.offset = static_cast<uint32_t>( ofs.tellp() );
        entry.size = static_cast<uint32_t>( size );
        ofs <<= tmp;
        header.offset = static_cast<uint32_t>( ofs.tellp() );
    }
    ofs <<= entries;
    ofs.seekp( 0 );
    header.size = static_cast<uint32_t>( entries.size() * sizeof( pak::Entry ) );
    ofs <<= header;
    return 0;
}
