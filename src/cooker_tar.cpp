#include <extra/tar.hpp>

#include <algorithm>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static std::string prepareTarPath( const std::string& str, int skipCount )
{
    auto stat = std::filesystem::status( str );
    if ( !std::filesystem::exists( stat ) ) {
        std::cout << "[ FAIL ] path don't exists: " << str << "\"";
        return {};
    }
    if ( !std::filesystem::is_regular_file( stat ) ) {
        std::cout << "[ FAIL ] path not file: " << str << "\"";
        return {};
    }

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
    if ( dist >= 100 ) {
        std::cout << "[ FAIL ] path too long to fit into tar Header.name field: " << str << "\"";
        return {};
    }
    std::string ret;
    ret.resize( static_cast<std::string::size_type>( dist ) );
    std::copy( it, str.end(), ret.begin() );
    return ret;
}

int main( int argc, char** argv )
{
    if ( argc != 3 ) {
        std::cout << "[ FAIL ] expected 2 arguments: archive.tar \"semi;colon;separated;list;of;files\"\n";
        return 1;
    }

    std::istringstream input;
    input.str( argv[ 2 ] );
    std::vector<std::pair<std::string,std::string>> fileList;
    for ( std::string filePath; std::getline( input, filePath, ';' ); ) {
        std::string tarPath = prepareTarPath( filePath, 1 );
        if ( tarPath.empty() ) {
            return 1;
        }
        fileList.emplace_back( std::make_pair( filePath, std::move( tarPath ) ) );
    }
    std::sort( fileList.begin(), fileList.end(), []( const auto& lhs, const auto& rhs ) { return lhs.second < rhs.second; } );

    std::ofstream ofs( argv[ 1 ], std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << "[ FAIL ] failed to open \"" << argv[ 1 ] << "\" for writing\n";
        return 1;
    }

    for ( const auto& pair : fileList ) {
        assert( pair.second.size() < 100 );

        tar::Header header{};
        std::copy( pair.second.begin(), pair.second.end(), std::begin( header.name ) );
        std::filesystem::path srcPath = pair.first;
        auto fileSize = std::filesystem::file_size( srcPath );
        auto mtime = std::filesystem::last_write_time( srcPath );
        auto systime = std::chrono::file_clock::to_sys( mtime );
        auto mstime = std::chrono::duration_cast<std::chrono::seconds>( systime.time_since_epoch() );

        header.size = static_cast<std::size_t>( fileSize );
        header.mode = 0'100'666u;
        header.uid = 0;
        header.gid = 0;
        header.mtime = static_cast<uint32_t>( mstime.count() );
        header.chksum = tar::checksum( header );
        assert( tar::isHeaderValid( &header ) );
        ofs.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );

        std::vector<char> tmp( tar::alignedSize( fileSize ) );
        std::ifstream ifs( pair.first, std::ios::binary );
        if ( !ifs.is_open() ) {
            std::cout << "[ FAIL ] failed to open \"" << pair.first << "\" for reading\n";
            return 1;
        }
        ifs.read( tmp.data(), static_cast<std::streamsize>( fileSize ) );
        ifs.close();
        ofs.write( tmp.data(), static_cast<std::streamsize>( tmp.size() ) );
    }
    tar::Eof eof{};
    ofs.write( eof.data(), eof.size() );
    return 0;
}
