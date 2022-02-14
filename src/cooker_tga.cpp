#include "tga.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main( int argc, char** argv )
{
    if ( argc != 3 ) {
        std::cout << "[ FAIL ] invalid number of arguments" << std::endl;
        return 1;
    }

    std::filesystem::path src{ argv[ 1 ] };
    std::filesystem::path dst{ argv[ 2 ] };

    if ( std::filesystem::exists( dst ) ) {
        const std::filesystem::file_status dstStat = std::filesystem::status( dst );
        const std::filesystem::perms perm = dstStat.permissions();
        if ( ( perm & std::filesystem::perms::owner_write ) == std::filesystem::perms::none ) {
            std::cout << "[ FAIL ] file not writable " << dst << std::endl;
            return 1;
        }
    }

    std::ifstream ifs( src, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) {
        std::cout << "[ FAIL ] failed to open file " << src << std::endl;
        return 1;
    }

    const size_t size = ifs.tellg();
    if ( size < sizeof( tga::Header ) ) {
        std::cout << "[ FAIL ] invalid file " << src << std::endl;
        return 1;
    }

    ifs.seekg( 0 );
    tga::Header header{};
    ifs.read( reinterpret_cast<char*>( &header ), sizeof( tga::Header ) );
    switch ( header.imageType ) {
    case tga::ImageType::eTrueColor:
    case tga::ImageType::eGrayscale:
        break;
    default:
        std::cout << "[ FAIL ] tga image type not supported " << src << std::endl;
        return 1;
    }

    if ( header.bitsPerPixel != 24 ) {
        ifs.close();
        const bool copyOK = std::filesystem::copy_file( src, dst, std::filesystem::copy_options::overwrite_existing );
        if ( copyOK ) {
            return 0;
        }
        std::cout << "[ FAIL ] failed to copy file " << src << " -> " << dst << std::endl;
        return 1;
    }

    header.bitsPerPixel = 32;
    std::vector<uint8_t> srcIn;
    srcIn.resize( 3u * header.width * header.height );
    ifs.read( reinterpret_cast<char*>( srcIn.data() ), srcIn.size() );
    ifs.close();
    std::vector<uint32_t> dstOut;
    dstOut.resize( 1u * header.width * header.height );

    struct BGR { uint8_t b, g, r; };
    struct BGRA { uint8_t b, g, r, a; };
    const BGR* dataBegin = reinterpret_cast<BGR*>( srcIn.data() );
    const BGR* dataEnd = dataBegin + header.width * header.height;
    BGRA* dataOut = reinterpret_cast<BGRA*>( dstOut.data() );
    std::transform( dataBegin, dataEnd, dataOut, []( const BGR& bgr ) -> BGRA
    {
        return BGRA {
            .b = bgr.b,
            .g = bgr.g,
            .r = bgr.r,
            .a = 255u,
        };
    } );

    std::ofstream ofs( dst, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << "[ FAIL ] failed to open " << dst << std::endl;
        return 1;
    }

    ofs.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );
    ofs.write( reinterpret_cast<const char*>( dstOut.data() ), dstOut.size() * sizeof( uint32_t ) );
    return 0;
}
