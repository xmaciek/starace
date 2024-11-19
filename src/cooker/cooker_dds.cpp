#include "cooker_dds.hpp"
#include "cooker_common.hpp"

#include <extra/dds.hpp>
#include <extra/tga.hpp>
#include <extra/args.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory_resource>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using Format = dds::dxgi::Format;

struct Image {
    Format format;
    uint32_t width = 0;
    uint32_t height = 0;
    std::pmr::vector<uint8_t> pixels;
};

Image convertTga( const std::filesystem::path& srcFile )
{
    using B = uint8_t;
    struct BGR { uint8_t b, g, r; };
    struct BGRA { uint8_t b, g, r, a; };

    std::ifstream ifs( srcFile, std::ios::binary | std::ios::ate );
    ifs.is_open() || cooker::error( "cannot open file:", srcFile );

    size_t fileSize = static_cast<size_t>( ifs.tellg() );
    ( fileSize >= sizeof( tga::Header ) ) || cooker::error( "file too short", srcFile );

    tga::Header header{};
    ifs.seekg( 0 );
    ifs.read( reinterpret_cast<char*>( &header ), sizeof( tga::Header ) );
    fileSize -= sizeof( tga::Header );

    std::pmr::vector<uint8_t> ret;

    switch ( header.imageType ) {
    case tga::ImageType::eColorMap: {
        uint16_t colorMapOffset = 0; std::memcpy( &colorMapOffset, header.colorMap, 2 );
        uint16_t colorMapCount = 0; std::memcpy( &colorMapCount, header.colorMap + 2, 2 );
        std::pmr::vector<uint8_t> colorMap;
        colorMap.resize( (std::size_t)colorMapCount * ( header.colorMap[ 4 ] / 8 ) );
        ifs.read( reinterpret_cast<char*>( colorMap.data() ), static_cast<std::streamsize>( colorMap.size() ) );

        std::pmr::vector<uint8_t> indexes;
        indexes.resize( header.width * header.height );
        ifs.read( reinterpret_cast<char*>( indexes.data() ), static_cast<std::streamsize>( indexes.size() ) );
        ifs.close();

        auto unmap = []( const auto* colorMap, const auto& indexes, auto* dataOut )
        {
            auto convert = [colorMap]( auto idx )
            {
                return colorMap[ idx ];
            };
            std::transform( indexes.begin(), indexes.end(), dataOut, convert );
        };
        switch ( header.colorMap[ 4 ] ) {
        case 8:
            if ( header.bitsPerPixel != 8 ) cooker::error( "color map pixel type mismatch" );
            ret.resize( header.width * header.height );
            unmap( reinterpret_cast<B*>( colorMap.data() ), indexes, reinterpret_cast<B*>( ret.data() ) );
            return { .format = Format::R8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 24: {
            if ( header.bitsPerPixel != 24 ) cooker::error( "color map pixel type mismatch" );
            std::pmr::vector<uint8_t> tmp;
            std::swap( tmp, colorMap );

            colorMap.resize( colorMapCount * sizeof( uint32_t ) );
            const BGR* srcBegin = reinterpret_cast<const BGR*>( tmp.data() );
            const BGR* srcEnd = srcBegin + colorMapCount;
            BGRA* dataOut = reinterpret_cast<BGRA*>( colorMap.data() );
            std::transform( srcBegin, srcEnd, dataOut, []( auto bgr ) { return BGRA{ bgr.b, bgr.g, bgr.r, 255u }; } );
        }
        [[fallthrough]];

        case 32:
            if ( header.bitsPerPixel != 32 ) cooker::error( "color map pixel type mismatch" );
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            unmap( reinterpret_cast<BGRA*>( colorMap.data() ), indexes, reinterpret_cast<BGRA*>( ret.data() ) );
            return { .format = Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        default:
            cooker::error( "unhandled colormap bpp", srcFile );
        }
    }


    case tga::ImageType::eTrueColor:
    case tga::ImageType::eGrayscale: {
        switch ( header.bitsPerPixel ) {
        case 8:
            ret.resize( header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { .format = Format::R8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 32:
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { .format = Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 24: {
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            std::pmr::vector<uint8_t> tmp;
            tmp.resize( fileSize );
            ifs.read( reinterpret_cast<char*>( tmp.data() ), static_cast<std::streamsize>( fileSize ) );

            const BGR* srcBegin = reinterpret_cast<const BGR*>( tmp.data() );
            const BGR* srcEnd = srcBegin + header.width * header.height;
            BGRA* dataOut = reinterpret_cast<BGRA*>( ret.data() );
            std::transform( srcBegin, srcEnd, dataOut, []( auto bgr ) { return BGRA{ bgr.b, bgr.g, bgr.r, 255u }; } );
            return { .format = Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };
        }
        default:
            cooker::error( "unknown pixel format, todo maybe later:", srcFile );
        }
    }

    default:
        cooker::error( "unhandled image type:", srcFile );
    }
}

void compressToBC4( Image& img )
{
    if ( img.format != Format::R8_UNORM ) cooker::error( "Expected format R8" );
    if ( img.pixels.empty() ) cooker::error( "no pixels to convert" );

    const uint32_t blockCount = (uint32_t)img.pixels.size() / 16;
    std::pmr::vector<uint8_t> imageOut( sizeof( dds::BC4 ) * blockCount );
    std::pmr::vector<dds::Swizzler<>::BlockType> tmp( blockCount );

    dds::Swizzler<uint8_t> swizzler{ img.pixels, img.width / 4 };
    std::generate( tmp.begin(), tmp.end(), swizzler );
    std::transform( tmp.begin(), tmp.end(), reinterpret_cast<dds::BC4*>( imageOut.data() ), &dds::compressor_bc4 );
    std::swap( img.pixels, imageOut );
    img.format = Format::BC4_UNORM;
}

namespace mipgen {

template <typename T>
requires ( std::is_same_v<T, uint32_t> || std::is_same_v<T, uint8_t> )
struct Generator {
    std::span<const T> pixels{};
    uint32_t srcWidth = 0;
    uint32_t idx = 0;

    Generator( std::span<const T> p, uint32_t w )
    : pixels{ p }
    , srcWidth{ w }
    {
        assert( !pixels.empty() );
        assert( srcWidth > 0 );
    }

    static std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> indexes( uint32_t n, uint32_t srcWidth )
    {
        n <<= 1u;
        n += ( n / srcWidth ) * srcWidth;
        return { n, n + 1u, n + srcWidth, n + srcWidth + 1u };
    }

    static uint32_t average( uint32_t a, uint32_t b, uint32_t c, uint32_t d )
    {
        auto sampler = []( uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t mask, uint32_t shift )
        {
            uint32_t ret = ( ( a & mask ) >> shift )
                + ( ( b & mask ) >> shift )
                + ( ( c & mask ) >> shift )
                + ( ( d & mask ) >> shift )
                ;
            return ( ret >> 2u ) << shift;
        };
        return sampler( a, b, c, d, 0xFFu, 0u )
            |  sampler( a, b, c, d, 0xFF00u, 8u )
            |  sampler( a, b, c, d, 0xFF0000u, 16u )
            |  sampler( a, b, c, d, 0xFF000000u, 24u )
            ;
    };

    static uint8_t average( uint8_t a, uint8_t b, uint8_t c, uint8_t d )
    {
        uint32_t r = a;
        r += b;
        r += c;
        r += d;
        return static_cast<uint8_t>( r >> 2u );
    }
    T operator () ()
    {
        auto [ a, b, c, d ] = indexes( idx++, srcWidth );
        assert( a < pixels.size() );
        assert( b < pixels.size() );
        assert( c < pixels.size() );
        assert( d < pixels.size() );
        return average( pixels[ a ], pixels[ b ], pixels[ c ], pixels[ d ] );
    }

};

template <typename T>
static Image downscale( const Image& img )
{
    assert( img.width );
    assert( img.height );
    assert( !img.pixels.empty() );
    const size_t mipPixelCount = ( img.width * img.height ) >> 2u;
    std::pmr::vector<uint8_t> ret( sizeof( T ) * mipPixelCount );

    std::span<const T> pixels{ reinterpret_cast<const T*>( img.pixels.data() ), img.pixels.size() / sizeof( T ) };
    std::span<T> mip{ reinterpret_cast<T*>( ret.data() ), mipPixelCount };

    std::generate( mip.begin(), mip.end(), Generator<T>{ pixels, img.width } );
    return Image{
        .format = img.format,
        .width = img.width >> 1,
        .height = img.height >> 1,
        .pixels = std::move( ret ),
    };
}

static uint32_t genMips( std::pmr::list<Image>& ret )
{
    assert( !ret.empty() );
    assert( ret.back().width );
    assert( ret.back().height );
    assert( !ret.back().pixels.empty() );
    uint32_t mipToGen = 0u;
    uint32_t width = ret.back().width;
    uint32_t height = ret.back().height;
    uint32_t pixelSize = 0u;
    switch ( ret.back().format ) {
    case Format::R8_UNORM: pixelSize = 1u; break;
    default: pixelSize = 4u; break;
    }
    auto test = []( uint32_t d ) { return ( d % 8u ) == 0u; };
    while ( test( width >> mipToGen ) && test( height >> mipToGen ) ) {
        mipToGen++;
    }
    for ( uint32_t i = 0u; i < mipToGen; ++i ) {
        switch ( pixelSize ) {
        case 1: ret.emplace_back( downscale<uint8_t>( ret.back() ) ); break;
        case 4: ret.emplace_back( downscale<uint32_t>( ret.back() ) ); break;
        }
    }
    return mipToGen;
}

}

inline dds::Header::Flags operator | ( dds::Header::Flags a, dds::Header::Flags b )
{
    using T = std::underlying_type_t<dds::Header::Flags>;
    return static_cast<dds::Header::Flags>( static_cast<T>( a ) | static_cast<T>( b ) );
}

inline dds::Header::Flags& operator |= ( dds::Header::Flags& a, dds::Header::Flags b )
{
    a = a | b;
    return a;
}

inline dds::Header::Caps& operator |= ( dds::Header::Caps& a, dds::Header::Caps b )
{
    using T = std::underlying_type_t<dds::Header::Flags>;
    a = static_cast<dds::Header::Caps>( static_cast<T>( a ) | static_cast<T>( b ));
    return a;
}

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--src \"<file/path.tga>,<file/path2.tga>\" \u2012 specifies source images to cook (comma separated)\n"
            "\t--dst \"<file/path.dds>\" \u2012 specifies destination of cooked image\n"
            "\nOptional arguments:\n"
            "\t-h --help \u2012 prints this message and exits\n"
            "\t--mipgen \u2012 generate mipmaps\n"
            "\t--format <value> \u2012 specifiy output image format, supported formats: BC4\n"
            ;
        return !args;
    }
    std::string_view argSrc{};
    std::string_view argDst{};

    args.read( "--src", argSrc ) || cooker::error( "--src <file/path.tga> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || cooker::error( "--dst <file/path.dds> \u2012 argument not specified" );
    const bool argMipgen = args.read( "--mipgen" );
    const Format argsFormat = [&args]()
    {
        std::string_view argsFormat{};
        if ( !args.read( "--format", argsFormat ) ) return Format::UNKNOWN;
        if ( argsFormat == "BC4" ) return Format::BC4_UNORM;
        cooker::error( "--format has unsupported value \u2012", argsFormat );
    }();

    auto commaSeparate = []( std::string_view sv ) -> std::pmr::vector<std::filesystem::path>
    {
        const char separator = ',';
        std::pmr::vector<std::filesystem::path> ret;
        ret.reserve( 6 );
        size_t i = 0;
        do {
            size_t end = sv.find( separator, i );
            ret.emplace_back( sv.substr( i, end - i ) );
            i = sv.find_first_not_of( separator, end );
        } while ( i != sv.npos );
        return ret;
    };
    std::pmr::vector<std::filesystem::path> src = commaSeparate( argSrc );
    src.empty() && cooker::error( "--src argument requires valid paths" );
    std::filesystem::path dst = argDst;
    std::list<Image> images;
    {
        uint32_t pendingWidth = 0;
        uint32_t pendingHeight = 0;
        Format pendingFormat{};
        for ( auto&& file : src ) {
            images.emplace_back() = convertTga( file );
            if ( images.back().pixels.empty() ) cooker::error( "image must have pixels" );
            uint32_t width = std::exchange( pendingWidth, images.back().width );
            uint32_t height = std::exchange( pendingHeight, images.back().height );
            Format format = std::exchange( pendingFormat, images.back().format );
            if ( width && ( width != pendingWidth ) ) cooker::error( "images cannot have different width when combining into array" );
            if ( height && ( height != pendingHeight ) ) cooker::error( "images cannot have different height when combining into array" );
            if ( ( format != Format::UNKNOWN ) && ( format != pendingFormat ) ) cooker::error( "images cannot have different format when combining into array" );
            if ( pendingFormat == Format::UNKNOWN ) cooker::error( "image must have format" );
        }
    }
    const uint32_t arrayCount = static_cast<uint32_t>( images.size() );
    std::pmr::list<Image> mips{};
    uint32_t mipCount = 0;
    for ( auto&& image : images ) {
        mips.emplace_back( std::move( image ) );
        if ( argMipgen ) {
            mipCount = mipgen::genMips( mips );
        }
    }
    images.clear();
    switch ( argsFormat ) {
    case Format::BC4_UNORM: std::for_each( mips.begin(), mips.end(), &compressToBC4 ); break;
    default: break;
    };

    using Flags = dds::Header::Flags;
    using Caps = dds::Header::Caps;
    dds::Header header{
        .flags = Flags::fCaps | Flags::fWidth | Flags::fHeight | Flags::fPixelFormat,
        .height = mips.front().height,
        .width = mips.front().width,
        .pitchOrLinearSize = static_cast<uint32_t>( mips.front().pixels.size() ),
        .mipMapCount = mipCount + 1,
        .pixelFormat{
            .flags = dds::PixelFormat::Flags::fFourCC,
            .fourCC = dds::DXGI,
        },
        .caps = dds::Header::Caps::fTexture,
    };

    dds::dxgi::Header dxgiHeader{
        .format = mips.front().format,
        .dimension = dds::dxgi::Dimension::eTexture2D,
        .arraySize = arrayCount,
    };

    if ( header.mipMapCount > 1 ) {
        header.flags |= Flags::fMipMapCount;
        header.caps |= Caps::fMipMap;
        header.caps |= Caps::fComplex;
    }

    if ( dxgiHeader.arraySize > 1 ) {
        header.caps |= Caps::fComplex;
    }
    std::ofstream ofs( dst, std::ios::binary );
    ofs.is_open() || cooker::error( "cannot open file:", dst );

    ofs.write( reinterpret_cast<const char*>( &header ), static_cast<std::streamsize>( sizeof( header ) ) );
    ofs.write( reinterpret_cast<const char*>( &dxgiHeader ), static_cast<std::streamsize>( sizeof( dxgiHeader ) ) );
    for ( auto&& mip : mips ) {
        ofs.write( reinterpret_cast<const char*>( mip.pixels.data() ), static_cast<std::streamsize>( mip.pixels.size() ) );
    }
    return 0;
}
