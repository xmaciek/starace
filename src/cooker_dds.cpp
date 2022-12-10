#include <extra/dds.hpp>
#include <extra/tga.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <string>
#include <tuple>
#include <vector>

std::tuple<dds::dxgi::Format, tga::Header, std::pmr::vector<uint8_t>> convertTga( const std::filesystem::path& srcFile )
{
    using B = uint8_t;
    struct BGR { uint8_t b, g, r; };
    struct BGRA { uint8_t b, g, r, a; };

    std::ifstream ifs( srcFile, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) {
        std::cout << "[ FAIL ] failed to open file " << srcFile << std::endl;
        return { {}, {}, {} };
    }

    size_t fileSize = static_cast<size_t>( ifs.tellg() );
    if ( fileSize < sizeof( tga::Header ) ) {
        std::cout << "[ FAIL ] invalid file " << srcFile << std::endl;
        return { {}, {}, {} };
    }

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
            return { dds::dxgi::Format::R8_UNORM, header, ret };

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
            return { dds::dxgi::Format::B8G8R8A8_UNORM, header, ret };

        default:
            std::cout << "[ FAIL ] unhandled colormap bpp (" << (int)header.colorMap[ 4 ] << ") " << srcFile << std::endl;
            return { {}, {}, {} };
        }
    }


    case tga::ImageType::eTrueColor:
    case tga::ImageType::eGrayscale: {
        switch ( header.bitsPerPixel ) {
        case 8:
            ret.resize( header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { dds::dxgi::Format::R8_UNORM, header, ret };

        case 32:
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            ifs.read( reinterpret_cast<char*>( ret.data() ), static_cast<std::streamsize>( ret.size() ) );
            return { dds::dxgi::Format::B8G8R8A8_UNORM, header, ret };

        case 24: {
            ret.resize( sizeof( uint32_t ) * header.width * header.height );
            std::pmr::vector<uint8_t> tmp;
            tmp.resize( fileSize );
            ifs.read( reinterpret_cast<char*>( tmp.data() ), static_cast<std::streamsize>( fileSize ) );

            const BGR* srcBegin = reinterpret_cast<const BGR*>( tmp.data() );
            const BGR* srcEnd = srcBegin + header.width * header.height;
            BGRA* dataOut = reinterpret_cast<BGRA*>( ret.data() );
            std::transform( srcBegin, srcEnd, dataOut, []( auto bgr ) { return BGRA{ bgr.b, bgr.g, bgr.r, 255u }; } );
            return { dds::dxgi::Format::B8G8R8A8_UNORM, header, ret };
        }
        default:
            std::cout << "[ FAIL ] unknown pixel format: todo maybe later: " << srcFile << std::endl;
            return { {}, {}, {} };
        }
    }

    default:
        std::cout << "[ FAIL ] unhandled image type (" << (int)header.imageType << "), todo maybe later: " << srcFile << std::endl;
        return { {}, {}, {} };
    }
}

struct MipGen {
    const uint32_t* pixels = nullptr;
    uint32_t width = 0;
    uint32_t idx = 0;
    uint32_t operator () ()
    {
        auto indexes = []( uint32_t n, uint32_t width ) -> std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>
        {
            n <<= 1u;
            n += ( n / width ) * width;
            return { n, n + 1u, n + width, n + width + 1u };
        };

        auto average = []( uint32_t a, uint32_t b, uint32_t c, uint32_t d ) -> uint32_t
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

        auto [ a, b, c, d ] = indexes( idx++, width );
        return average( pixels[ a ], pixels[ b ], pixels[ c ], pixels[ d ] );
    }

    static std::pmr::vector<uint32_t> gib( uint32_t width, uint32_t height, const uint32_t* pixels )
    {
        assert( width );
        assert( height );
        assert( pixels );
        std::pmr::vector<uint32_t> mip;
        mip.resize( ( width * height ) >> 2u );
        std::generate( mip.begin(), mip.end(), MipGen{ pixels, width } );
        return mip;
    }

    static std::pmr::vector<std::pmr::vector<uint32_t>> genMips( uint32_t width, uint32_t height, const uint32_t* pixels )
    {
        assert( width );
        assert( height );
        assert( pixels );
        uint32_t mipToGen = 0;
        auto test = []( uint32_t d ) { return ( ( d % 4 ) == 0 ) && ( d > 4 ); };
        while ( test( width >> mipToGen ) && test( height >> mipToGen ) ) {
            mipToGen++;
        }
        std::pmr::vector<std::pmr::vector<uint32_t>> ret;
        ret.resize( mipToGen );
        for ( uint32_t i = 0; i < mipToGen; ++i ) {
            ret[ i ] = gib( width >> i, height >> i, pixels );
            pixels = ret[ i ].data();
        }
        return ret;
    }
};


int main( int argc, char** argv )
{
    if ( argc < 3 ) {
        std::cout << "[ FAIL ] invalid number of arguments, expected >= 2, got " << argc - 1 << std::endl;
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

    auto [ format, tgaHeader, pixels ] = convertTga( src );

    if ( format == dds::dxgi::Format::UNKNOWN ) { return 1; }

    auto findParam = []( int argc, char** argv, const char* value )
    {
        char** end = argv + argc;
        char** ret = std::find_if( argv, end, [value](const char* v) { return std::strcmp( v, value ) == 0; } );
        return ret != end ? ret : nullptr;
    };

    std::pmr::vector<std::pmr::vector<uint32_t>> mips = findParam( argc - 3, argv + 3, "mipgen" )
        ? MipGen::genMips( tgaHeader.width, tgaHeader.height, reinterpret_cast<const uint32_t*>( pixels.data() ) )
        : std::pmr::vector<std::pmr::vector<uint32_t>>();

    dds::Header header{
        .flags = dds::constants::uncompressed,
        .height = tgaHeader.height,
        .width = tgaHeader.width,
        .pitchOrLinearSize = static_cast<uint32_t>( pixels.size() ),
        .mipMapCount = static_cast<uint32_t>( mips.size() ) + 1u,
        .pixelFormat = dds::constants::DXGI,
        .caps = static_cast<dds::Header::Caps>( dds::Header::Caps::fTexture | dds::Header::Caps::fMipMap ),
    };

    dds::dxgi::Header dxgiHeader{
        .format = format,
        .dimension = dds::dxgi::Dimension::eTexture2D,
    };
    std::ofstream ofs( dst, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << "[ FAIL ] failed to open " << dst << std::endl;
        return 1;
    }

    ofs.write( reinterpret_cast<const char*>( &header ), static_cast<std::streamsize>( sizeof( header ) ) );
    ofs.write( reinterpret_cast<const char*>( &dxgiHeader ), static_cast<std::streamsize>( sizeof( dxgiHeader ) ) );
    ofs.write( reinterpret_cast<const char*>( pixels.data() ), static_cast<std::streamsize>( pixels.size() ) );
    for ( const auto& mip : mips ) {
        ofs.write( reinterpret_cast<const char*>( mip.data() ), static_cast<std::streamsize>( mip.size() * 4 ) );
    }
    return 0;
}
