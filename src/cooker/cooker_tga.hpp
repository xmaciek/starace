#pragma once

#include <extra/dds.hpp>
#include <extra/tga.hpp>

#include "cooker_common.hpp"

#include <fstream>
#include <filesystem>

struct Image {
    dds::dxgi::Format format;
    uint32_t width = 0;
    uint32_t height = 0;
    std::pmr::vector<uint8_t> pixels;
};

namespace tga::detail {

using B = uint8_t;
struct BGR { uint8_t b, g, r; };
struct alignas( 4 ) BGRA { uint8_t b, g, r, a; };

static inline auto convert_BGRA_B( const BGRA& c ) { return c.b; }
static inline auto convert_BGRA_BGR( const BGRA& c ) { return BGR{ c.b, c.g, c.r }; }
static inline auto convert_BGR_B( const BGR& c ) { return c.b; }
static inline auto convert_BGR_BGRA( const BGR& c ) { return BGRA{ c.b, c.g, c.r, 0xFF }; };
static inline auto convert_B_BGR( B c ) { return BGR{ c, c, c }; }
static inline auto convert_B_BGRA( B c ) { return BGRA{ c, c, c, 0xFF }; };

template <typename T>
struct RLE {
    std::span<const uint8_t> m_data{};
    T m_value{};
    int16_t m_pixelCount = -1;
    bool m_compressed = false;

    RLE( std::span<const uint8_t> data )
    : m_data{ data }
    {}

    T operator () ()
    {
        if ( m_pixelCount == -1 ) {
            if ( m_data.size() < 1 ) cooker::error( "unexpected end of RLE stream", __LINE__ );
            m_pixelCount = m_data.front();
            m_compressed = m_pixelCount & 128;
            m_pixelCount &= ~128;
            if ( m_compressed ) {
                if ( m_data.size() < sizeof( T ) + 1 ) cooker::error( "unexpected end of RLE stream", __LINE__ );
                std::memcpy( &m_value, m_data.data() + 1, sizeof( T ) );
                m_data = m_data.subspan( sizeof( T ) + 1 );
            }
            else {
                m_data = m_data.subspan( 1 );
            }
        }

        m_pixelCount--;
        if ( m_compressed ) {
            return m_value;
        }
        T ret{};
        if ( m_data.size() < sizeof( T ) ) cooker::error( "unexpected end of RLE stream", __LINE__ );
        std::memcpy( &ret, m_data.data(), sizeof( T ) );
        m_data = m_data.subspan( sizeof( T ) );
        return ret;
    }
};

template <typename T>
struct Unmap {
    std::span<T> data;
    T operator () ( uint8_t i ) { return data[ i ]; }
};

}

namespace tga {

static Image read( const std::filesystem::path& filePath )
{
    using enum dds::dxgi::Format;
    using namespace tga::detail;
    std::ifstream ifs( filePath, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) cooker::error( "cannot open file:", filePath );
    size_t fileSize = static_cast<size_t>( ifs.tellg() );
    ifs.seekg( 0 );

    tga::Header header;
    if ( fileSize < sizeof( header ) ) cooker::error( "cannot read tga header:", filePath );
    cooker::read( ifs, header );
    fileSize -= sizeof( header );

    // skip ID length
    ifs.seekg( header.idLength, std::ios::cur );
    fileSize -= header.idLength;

    Image ret{
        .width = header.width,
        .height = header.height,
    };
    switch ( header.bitsPerPixel ) {
    case 8: ret.format = R8_UNORM; break;
    case 24: [[fallthrough]];
    case 32: ret.format = B8G8R8A8_UNORM; break;
    default: cooker::error( "unable to guess image format" ); break;
    }

    const uint32_t pixelCount = header.width * header.height;
    uint16_t colorMapOffset = 0; std::memcpy( &colorMapOffset, header.colorMap, 2 );
    uint16_t colorMapCount = 0; std::memcpy( &colorMapCount, header.colorMap + 2, 2 );
    uint16_t colorMapBpp = header.colorMap[ 4 ];
    if ( colorMapOffset ) cooker::error( "colormap with offset not implemented:", filePath );

    std::pmr::vector<uint8_t> colorMap( colorMapCount * colorMapBpp / 8 );
    cooker::read( ifs, colorMap );

    // NOTE: convert color map format to expected color format to preserve sanity
    if ( colorMapBpp && colorMapBpp != header.bitsPerPixel ) {
        std::pmr::vector<uint8_t> colorTmp( colorMapCount * header.bitsPerPixel / 8 );
        switch ( ( colorMapBpp << 8 ) | header.bitsPerPixel ) {
        case 0x08'18: std::ranges::transform( cooker::cast<B>( colorMap ), (BGR*)colorTmp.data(), &convert_B_BGR ); break;
        case 0x08'20: std::ranges::transform( cooker::cast<B>( colorMap ), (BGRA*)colorTmp.data(), &convert_B_BGRA ); break;
        case 0x18'08: std::ranges::transform( cooker::cast<BGR>( colorMap ), (B*)colorTmp.data(), &convert_BGR_B ); break;
        case 0x18'20: std::ranges::transform( cooker::cast<BGR>( colorMap ), (BGRA*)colorTmp.data(), &convert_BGR_BGRA ); break;
        case 0x20'08: std::ranges::transform( cooker::cast<BGRA>( colorMap ), (B*)colorTmp.data(), &convert_BGRA_B ); break;
        case 0x20'18: std::ranges::transform( cooker::cast<BGRA>( colorMap ), (BGR*)colorTmp.data(), &convert_BGRA_BGR ); break;
        default: cooker::error( "unknown colormap conversion:", filePath ); break;
        }
        std::swap( colorMap, colorTmp );
    }

    ret.pixels.resize( pixelCount * header.bitsPerPixel / 8 );
    switch ( header.imageType ) {
    case eGrayscale:
    case eTrueColor:
        cooker::read( ifs, ret.pixels );
        break;

    case eGrayscaleRLE:
    case eTrueColorRLE: {
        auto rle = cooker::read( ifs, fileSize );
        switch ( header.bitsPerPixel ) {
        case 8: std::ranges::generate( cooker::cast<B>( ret.pixels ), RLE<B>{ rle } ); break;
        case 24: std::ranges::generate( cooker::cast<BGR>( ret.pixels ), RLE<BGR>{ rle } ); break;
        case 32: std::ranges::generate( cooker::cast<BGRA>( ret.pixels ), RLE<BGRA>{ rle } ); break;
        default: cooker::error( "unknown bits per pixel for RLE decompression:", filePath ); break;
        }
        break;
    }

    case eColorMap:
    case eColorMapRLE: {
        std::pmr::vector<uint8_t> indexes( pixelCount );
        if ( header.imageType & fRLE ) {
            auto rle = cooker::read( ifs, fileSize );
            std::ranges::generate( indexes, RLE<uint8_t>{ rle } );
        }
        else {
            cooker::read( ifs, indexes );
        }

        switch ( header.bitsPerPixel ) {
        case 8: std::ranges::transform( indexes, (B*)ret.pixels.data(), Unmap{ cooker::cast<B>( colorMap ) } ); break;
        case 24: std::ranges::transform( indexes, (BGR*)ret.pixels.data(), Unmap{ cooker::cast<BGR>( colorMap ) } ); break;
        case 32: std::ranges::transform( indexes, (BGRA*)ret.pixels.data(),  Unmap{ cooker::cast<BGRA>( colorMap ) } ); break;
        default: cooker::error( "unknown bits per pixel for unmapping:", filePath ); break;
        }
        break;
    }
    [[unlikely]]
    default:
        cooker::error( "unknown image type, todo maybe later:", filePath ); break;
    }

    // NOTE: convert BGR to BGRA
    if ( header.bitsPerPixel == 24 ) {
        std::pmr::vector<uint8_t> pixels( pixelCount * sizeof( BGRA ) );
        std::ranges::transform( cooker::cast<BGR>( ret.pixels ), (BGRA*)pixels.data(), &convert_BGR_BGRA );
        ret.pixels = std::move( pixels );
        header.bitsPerPixel = 32;
    }

    // NOTE: vertical flip
    if ( ~header.imageDescriptor & tga::fTopLeft ) {
        const size_t laneWidth = ( header.bitsPerPixel / 8 ) * header.width;
        void* tmp = alloca( laneWidth );
        for ( auto i = 0u; i < header.height / 2; ++i ) {
            auto a = ret.pixels.data() + laneWidth * i;
            auto b = ( ret.pixels.data() + ret.pixels.size() ) - laneWidth * ( i + 1 );
            std::memcpy( tmp, a, laneWidth );
            std::memcpy( a, b, laneWidth );
            std::memcpy( b, tmp, laneWidth );
        }
    }

    return ret;
}

}
