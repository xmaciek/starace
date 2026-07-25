#include <cooker/common.hpp>

#include <extra/pak.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>

static std::string preparePakPath( const std::string& str, uint32_t includeDirectoryCount )
{
    auto stat = std::filesystem::status( str );
    std::filesystem::exists( stat ) || cooker::error( "path don't exists:", str );
    std::filesystem::is_regular_file( stat ) || cooker::error( "path is not file:", str );

    auto pakName = cooker::baseName( str, includeDirectoryCount );
    if ( pakName.size() >= sizeof( pak::Entry::name ) )
        cooker::error( "path too long to fit into Header.path field", str );

    return std::string( pakName );
}

static std::vector<std::pair<std::string, std::string>> preparePathPairs( std::string_view list )
{
    std::vector<std::pair<std::string,std::string>> ret;
    std::istringstream input;
    input.str( (std::string)list );

    for ( std::string filePath; std::getline( input, filePath, ';' ); ) {
        std::string pakPath = preparePakPath( filePath, 1 );
        ret.emplace_back( std::make_pair( filePath, std::move( pakPath ) ) );
    }
    auto cmp = []( const auto& lhs, const auto& rhs ) { return lhs.second < rhs.second; };
    std::ranges::sort( ret, cmp );
    return ret;
}

static bool strcpyIntoBuffer( const auto& src, auto& dst )
{
    if ( std::size( src ) > std::size( dst ) ) return false;
    std::ranges::copy( src, std::begin( dst ) );
    return true;
}

static std::ofstream& align( std::ofstream& o, size_t a )
{
    uint64_t pos = static_cast<uint64_t>( o.tellp() );
    pos = ( pos + ( a - 1 ) ) & ~( a - 1 );
    o.seekp( static_cast<std::streamoff>( pos ) );
    return o;
}

int main( int argc, const char** argv )
{
    ( argc == 3 ) || cooker::error( "expected 2 arguments: archive.pak \"semi;colon;separated;list;of;files\"", "" );

    auto fileList = preparePathPairs( argv[ 2 ] );
    pak::Header header{};
    std::pmr::vector<pak::Entry> entries;
    entries.reserve( fileList.size() );

    auto ofs = cooker::openWrite( argv[ 1 ] );
    cooker::write( ofs, header );

    std::pmr::vector<char> tmp;
    for ( auto&& [ src, dst ] : fileList ) {
        auto& entry = entries.emplace_back();
        strcpyIntoBuffer( dst, entry.name ) || cooker::error( "path too long to fit into .name field", dst );
        auto&& [ifs, size] = cooker::openRead( src );
        tmp.resize( size );
        cooker::read( ifs, tmp );
        align( ofs, 16 );

        entry.offset = static_cast<uint32_t>( ofs.tellp() );
        entry.size = static_cast<uint32_t>( size );
        cooker::write( ofs, tmp );
    }
    align( ofs, 64 );
    header.offset = static_cast<uint32_t>( ofs.tellp() );
    cooker::write( ofs, entries );
    ofs.seekp( 0 );
    header.size = static_cast<uint32_t>( entries.size() * sizeof( pak::Entry ) );
    cooker::write( ofs, header );
    return 0;
}
