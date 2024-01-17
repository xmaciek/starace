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

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

[[noreturn]]
static bool exitOnFailed( std::string_view msg, const auto& arg )
{
    std::cout << FAIL << msg << " " << arg << std::endl;
    std::exit( 1 );
}
[[noreturn]]
static bool exitOnFailed( std::string_view msg )
{
    std::cout << FAIL << msg << std::endl;
    std::exit( 1 );
};

struct Image {
    dds::dxgi::Format format;
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
    ifs.is_open() || exitOnFailed( "cannot open file:", srcFile );

    size_t fileSize = static_cast<size_t>( ifs.tellg() );
    ( fileSize >= sizeof( tga::Header ) ) || exitOnFailed( "file too short", srcFile );

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
            ret.resize( header.width * header.height );
            unmap( reinterpret_cast<B*>( colorMap.data() ), indexes, reinterpret_cast<B*>( ret.data() ) );
            return { .format = dds::dxgi::Format::R8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 24: {
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
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            unmap( reinterpret_cast<BGRA*>( colorMap.data() ), indexes, reinterpret_cast<BGRA*>( ret.data() ) );
            return { .format = dds::dxgi::Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        default:
            exitOnFailed( "unhandled colormap bpp", srcFile );
        }
    }


    case tga::ImageType::eTrueColor:
    case tga::ImageType::eGrayscale: {
        switch ( header.bitsPerPixel ) {
        case 8:
            ret.resize( header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { .format = dds::dxgi::Format::R8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 32:
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { .format = dds::dxgi::Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };

        case 24: {
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            std::pmr::vector<uint8_t> tmp;
            tmp.resize( fileSize );
            ifs.read( reinterpret_cast<char*>( tmp.data() ), static_cast<std::streamsize>( fileSize ) );

            const BGR* srcBegin = reinterpret_cast<const BGR*>( tmp.data() );
            const BGR* srcEnd = srcBegin + header.width * header.height;
            BGRA* dataOut = reinterpret_cast<BGRA*>( ret.data() );
            std::transform( srcBegin, srcEnd, dataOut, []( auto bgr ) { return BGRA{ bgr.b, bgr.g, bgr.r, 255u }; } );
            return { .format = dds::dxgi::Format::B8G8R8A8_UNORM, .width = header.width, .height = header.height, .pixels = std::move( ret ) };
        }
        default:
            exitOnFailed( "unknown pixel format, todo maybe later:", srcFile );
        }
    }

    default:
        exitOnFailed( "unhandled image type:", srcFile );
    }
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

    [[maybe_unused]]
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

    [[maybe_unused]]
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
static std::pmr::vector<uint8_t> gib( uint32_t srcWidth, uint32_t srcHeight, std::span<const uint8_t> data )
{
    assert( srcWidth );
    assert( srcHeight );
    assert( !data.empty() );
    const size_t mipPixelCount = ( srcWidth * srcHeight ) >> 2u;
    std::pmr::vector<uint8_t> ret( sizeof( T ) * mipPixelCount );

    std::span<const T> pixels{ reinterpret_cast<const T*>( data.data() ), data.size() / sizeof( T ) };
    std::span<T> mip{ reinterpret_cast<T*>( ret.data() ), mipPixelCount };

    std::generate( mip.begin(), mip.end(), Generator<T>{ pixels, srcWidth } );
    return ret;
}

static uint32_t genMips( std::pmr::list<std::pmr::vector<uint8_t>>& ret, uint32_t width, uint32_t height, std::span<const uint8_t> pixels, uint32_t pixelSize )
{
    assert( width );
    assert( height );
    assert( !pixels.empty() );
    uint32_t mipToGen = 0u;
    auto test = []( uint32_t d ) { return ( d % 8u ) == 0u; };
    while ( test( width >> mipToGen ) && test( height >> mipToGen ) ) {
        mipToGen++;
    }
    for ( uint32_t i = 0u; i < mipToGen; ++i ) {
        switch ( pixelSize ) {
        case 1: ret.emplace_back() = gib<uint8_t>( width >> i, height >> i, pixels ); break;
        case 4: ret.emplace_back() = gib<uint32_t>( width >> i, height >> i, pixels ); break;
        }
        pixels = ret.back();
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
            "\t--mipgen \u2012 generate mipmaps when cooking\n"
            ;
        return !args;
    }
    std::string_view argSrc{};
    std::string_view argDst{};

    args.read( "--src", argSrc ) || exitOnFailed( "--src <file/path.tga> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || exitOnFailed( "--dst <file/path.dds> \u2012 argument not specified" );
    bool argMipgen = args.read( "--mipgen" );

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
    src.empty() && exitOnFailed( "--src argument requires valid paths" );
    std::filesystem::path dst = argDst;
    std::list<Image> images;
    {
        uint32_t pendingWidth = 0;
        uint32_t pendingHeight = 0;
        dds::dxgi::Format pendingFormat{};
        for ( auto&& file : src ) {
            images.emplace_back() = convertTga( file );
            if ( images.back().pixels.empty() ) exitOnFailed( "image must have pixels" );
            uint32_t width = std::exchange( pendingWidth, images.back().width );
            uint32_t height = std::exchange( pendingHeight, images.back().height );
            dds::dxgi::Format format = std::exchange( pendingFormat, images.back().format );
            if ( width && ( width != pendingWidth ) ) exitOnFailed( "images cannot have different width when combining into array" );
            if ( height && ( height != pendingHeight ) ) exitOnFailed( "images cannot have different height when combining into array" );
            if ( ( format != dds::dxgi::Format::UNKNOWN ) && ( format != pendingFormat ) ) exitOnFailed( "images cannot have different format when combining into array" );
            if ( pendingFormat == dds::dxgi::Format::UNKNOWN ) exitOnFailed( "image must have format" );
        }
    }

    std::pmr::list<std::pmr::vector<uint8_t>> mips{};
    auto formatToPixelSize = []( auto f ) -> uint32_t
    {
        switch ( f ) {
        case dds::dxgi::Format::R8_UNORM: return 1u;
        default: return 4u;
        }
    };
    uint32_t mipCount = 0;
    for ( auto&& image : images ) {
        mips.emplace_back( std::move( image.pixels ) );
        if ( argMipgen ) {
            mipCount = mipgen::genMips( mips, image.width, image.height, mips.back(), formatToPixelSize( image.format ) );
        }
    }

    using Flags = dds::Header::Flags;
    using Caps = dds::Header::Caps;
    dds::Header header{
        .flags = Flags::fCaps | Flags::fWidth | Flags::fHeight | Flags::fPixelFormat,
        .height = images.front().height,
        .width = images.front().width,
        .pitchOrLinearSize = static_cast<uint32_t>( mips.front().size() ),
        .mipMapCount = mipCount + 1,
        .pixelFormat{
            .flags = dds::PixelFormat::Flags::fFourCC,
            .fourCC = dds::DXGI,
        },
        .caps = dds::Header::Caps::fTexture,
    };

    dds::dxgi::Header dxgiHeader{
        .format = images.front().format,
        .dimension = dds::dxgi::Dimension::eTexture2D,
        .arraySize = static_cast<uint32_t>( images.size() ),
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
    ofs.is_open() || exitOnFailed( "cannot open file:", dst );

    ofs.write( reinterpret_cast<const char*>( &header ), static_cast<std::streamsize>( sizeof( header ) ) );
    ofs.write( reinterpret_cast<const char*>( &dxgiHeader ), static_cast<std::streamsize>( sizeof( dxgiHeader ) ) );
    for ( auto&& mip : mips ) {
        ofs.write( reinterpret_cast<const char*>( mip.data() ), static_cast<std::streamsize>( mip.size() ) );
    }
    return 0;
}
