#include "tga.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <vector>

enum EvalHeader {
    eUnknown,
    eCopyOnly,
    eMissingAlpha,
    eUnmap,
};

static EvalHeader evalHeader( const tga::Header& header )
{
    if ( header.imageType > tga::eGrayscale ) {
        return eUnknown;
    }

    if ( header.hasColorMap ) {
        return eUnmap;
    }

    if ( header.bitsPerPixel == 24 ) {
        return eMissingAlpha;
    }

    if ( header.bitsPerPixel == 32 ) {
        return eCopyOnly;
    }

    return eUnknown;
}

struct BGR { uint8_t b, g, r; };
struct BGRA { uint8_t b, g, r, a; };
using B = uint8_t;

template <typename TMap, typename TDst>
TDst convert( TMap );

template <> B convert<B,B>( B color )               { return color; }
template <> B convert<BGR, B>( BGR color )          { return color.b; }
template <> BGRA convert<B,BGRA>( B color )         { return BGRA{ color, color, color, 255u }; }
template <> BGRA convert<BGR, BGRA>( BGR color )    { return BGRA{ color.b, color.g, color.r, 255u }; }
template <> BGRA convert<BGRA, BGRA>( BGRA color )  { return color; }

static std::vector<uint8_t> missingAlpha( const tga::Header& h, std::vector<uint8_t>&& srcIn )
{
    tga::Header header = h;
    header.bitsPerPixel = 32;
    std::vector<uint8_t> ret( sizeof( header ) + 4u * header.width * header.height );

    uint8_t* ptr = ret.data();
    std::memcpy( ptr, &header, sizeof( header ) );
    std::advance( ptr, sizeof( header ) );

    const BGR* srcBegin = reinterpret_cast<const BGR*>( srcIn.data() );
    const BGR* srcEnd = srcBegin + header.width * header.height;
    BGRA* dataOut = reinterpret_cast<BGRA*>( ptr );
    std::transform( srcBegin, srcEnd, dataOut, convert<BGR,BGRA> );
    return ret;
}

template <typename T>
std::span<T> viewColorMap( const tga::Header& header, uint8_t* headerlessData )
{
    assert( header.hasColorMap );
    assert( sizeof( T ) * 8 == header.colorMap[ 4 ] );
    uint16_t colorMapOffset = 0; std::memcpy( &colorMapOffset, header.colorMap, 2 );
    uint16_t colorMapCount = 0; std::memcpy( &colorMapCount, header.colorMap + 2, 2 );
    assert( colorMapCount < 256 );

    T* ptr = reinterpret_cast<T*>( headerlessData + colorMapOffset );
    return { ptr, ptr + colorMapCount };
}

template <typename T>
std::span<uint8_t> viewIndexes( const tga::Header& header, uint8_t* headerlessData )
{
    std::span span = viewColorMap<T>( header, headerlessData );
    uint8_t* begin = reinterpret_cast<uint8_t*>( &*span.end() );
    return { begin, begin + header.width * header.height };
}

template <typename TMap, typename TDst>
std::vector<uint8_t> unmap( tga::Header header, std::vector<uint8_t>&& srcIn )
{
    static_assert( alignof( TMap ) == 1 );
    static_assert( alignof( TDst ) == 1 );

    std::span<TMap> colorMap = viewColorMap<TMap>( header, srcIn.data() );
    std::span<uint8_t> indexes = viewIndexes<TMap>( header, srcIn.data() );
    std::vector<uint8_t> ret( sizeof( header ) + sizeof( TDst ) * header.width * header.height );

    header.hasColorMap = false;
    header.imageType = std::is_same_v<TDst, B> ? tga::ImageType::eGrayscale : tga::ImageType::eTrueColor;
    header.bitsPerPixel = std::is_same_v<TDst, B> ? 8u : 32u;
    header.imageDescriptor = 0;
    std::fill( std::begin( header.colorMap ), std::end( header.colorMap ), 0 );

    uint8_t* dst = ret.data();
    std::memcpy( dst, &header, sizeof( header ) );
    std::advance( dst, sizeof( header ) );

    TDst* colors = reinterpret_cast<TDst*>( dst );
    std::transform( indexes.begin(), indexes.end(), colors,
        [colorMap]( uint8_t idx ) -> TDst { return convert<TMap, TDst>( colorMap[ idx ] ); }
    );

    return ret;
}

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

    size_t fileSize = static_cast<size_t>( ifs.tellg() );
    if ( fileSize < sizeof( tga::Header ) ) {
        std::cout << "[ FAIL ] invalid file " << src << std::endl;
        return 1;
    }

    tga::Header header{};
    ifs.seekg( 0 );
    ifs.read( reinterpret_cast<char*>( &header ), sizeof( tga::Header ) );
    fileSize -= sizeof( tga::Header );

    std::vector<uint8_t> srcIn;
    std::vector<uint8_t> ret;

    switch ( evalHeader( header ) ) {
    default:
        std::cout << "[ FAIL ] tga image type not supported " << src << std::endl;
        return 1;

    case eCopyOnly: {
        ifs.close();
        const bool copyOK = std::filesystem::copy_file( src, dst, std::filesystem::copy_options::overwrite_existing );
        if ( copyOK ) {
            return 0;
        }
        std::cout << "[ FAIL ] failed to copy file " << src << " -> " << dst << std::endl;
        return 1;
    }

    case eMissingAlpha:
        srcIn.resize( fileSize );
        ifs.read( reinterpret_cast<char*>( srcIn.data() ), static_cast<std::streamsize>( fileSize ) );
        ifs.close();
        ret = missingAlpha( header, std::move( srcIn ) );
        break;

    case eUnmap:
        srcIn.resize( fileSize );
        ifs.read( reinterpret_cast<char*>( srcIn.data() ), static_cast<std::streamsize>( fileSize ) );
        ifs.close();
        switch ( ( static_cast<uint32_t>( header.colorMap[ 4 ] ) << 8 ) | header.bitsPerPixel ) {
        case 0x8'08: ret = unmap<B,B>( header, std::move( srcIn ) ); break;

        case 0x8'18: [[fallthrough]];
        case 0x8'20: ret = unmap<B,BGRA>( header, std::move( srcIn ) ); break;

        case 0x18'08: ret = unmap<BGR, B>( header, std::move( srcIn ) ); break;

        case 0x18'18: [[fallthrough]];
        case 0x18'20: ret = unmap<BGR,BGRA>( header, std::move( srcIn ) ); break;

        case 0x20'18: [[fallthrough]];
        case 0x20'20: ret = unmap<BGRA,BGRA>( header, std::move( srcIn ) ); break;

        default:
            std::cout << "[ FAIL ] don't know how to convert colormap "
                << static_cast<int>( header.colorMap[ 4 ] )
                << " -> "
                << static_cast<int>( header.bitsPerPixel )
                << std::endl;
            return 1;
        }
    }

    std::ofstream ofs( dst, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << "[ FAIL ] failed to open " << dst << std::endl;
        return 1;
    }

    ofs.write( reinterpret_cast<const char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
    return 0;
}
