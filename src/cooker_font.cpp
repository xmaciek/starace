#include <extra/dds.hpp>
#include <extra/fnta.hpp>
#include <extra/args.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#define RED "\x1b[31m"
#define YELLOW "\x1b[33m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";
static constexpr char WARN[] = "[ " YELLOW "WARN" DEFAULT " ] ";

static void exitOnFailed( FT_Error ec )
{
    if ( ec == 0 ) return;
    std::cout << FAIL << FT_Error_String( ec ) << std::endl;
    std::exit( 1 );
}

static uint8_t lerp( uint8_t min, uint8_t max, float n )
{
    return min + (uint8_t)((float)( max - min ) * n );
}


struct BlitIterator {
    using value_type = uint8_t;
    using size_type = uint32_t;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    pointer m_data = nullptr;
    pointer m_end = nullptr;
    size_type m_dstPitch = 0;
    size_type m_dstX = 0;
    size_type m_dstY = 0;
    size_type m_srcWidth = 0;
    size_type m_i = 0;

    BlitIterator() noexcept = default;
    BlitIterator( pointer p, pointer end, size_type dstPitch, size_type dstX, size_type dstY, size_type srcWidth ) noexcept
    : m_data{ p }
    , m_end{ end }
    , m_dstPitch{ dstPitch }
    , m_dstX{ dstX }
    , m_dstY{ dstY }
    , m_srcWidth{ srcWidth }
    {
        assert( p < end );
    }

    reference operator * () const
    {
        assert( m_data );
        assert( m_dstPitch );
        assert( m_srcWidth );
        const difference_type begin = m_dstPitch * m_dstY + m_dstX;
        const difference_type offset = m_dstPitch * ( m_i / m_srcWidth ) + ( m_i % m_srcWidth );
        pointer address = m_data + begin + offset;
        assert( address < m_end );
        return *address;
    }

    BlitIterator& operator ++ ()
    {
        ++m_i;
        return *this;
    }
    BlitIterator& operator -- ()
    {
        --m_i;
        return *this;
    }
    BlitIterator operator ++ ( int )
    {
        auto ret = *this;
        ++m_i;
        return ret;
    }
    BlitIterator operator -- ( int )
    {
        auto ret = *this;
        --m_i;
        return ret;
    }
};

struct AtlasComposer {
    using value_type = uint8_t;
    using pointer = value_type*;

    static constexpr uint32_t TILE_PADDING = 1u;

    pointer m_begin = nullptr;
    pointer m_end = nullptr;
    uint32_t m_width = 0u;
    uint32_t m_height = 0u;
    uint32_t m_currentX = TILE_PADDING;
    uint32_t m_currentY = TILE_PADDING;
    uint32_t m_nextY = 0;

    std::tuple<uint32_t, uint32_t> coords( uint32_t width, uint32_t height )
    {
        assert( width <= m_width );
        assert( height <= m_height );
        assert( m_currentY + height < m_height );

        if ( ( m_currentX + width ) > m_width ) {
            m_currentX = TILE_PADDING;
            m_currentY = m_nextY;
        }

        uint32_t retX = m_currentX;
        m_nextY = std::max( m_currentY + height + TILE_PADDING, m_nextY );
        m_currentX += width + TILE_PADDING;
        assert( m_currentY < m_height );
        return { retX, m_currentY };
    }

    std::tuple<BlitIterator, uint32_t, uint32_t > findPlace( uint32_t width, uint32_t height )
    {
        const auto [ x, y ] = coords( width, height );
        return { BlitIterator{ m_begin, m_end, m_width, x, y, width }, x, y };

    }
};

struct Slot {
    fnta::Glyph glyph{};
    char32_t ch{};
    uint32_t pitch{};
    uint32_t rows{};
    std::vector<uint8_t> bitmap{};
};

struct BC4unorm {
    uint8_t alpha0;
    uint8_t alpha1;
    uint8_t aindexes[ 6 ];
};

template <typename TPixel = uint8_t>
struct Swizzler {
    using PixelType = TPixel;
    using BlockType = std::array<PixelType, 16>;

    std::span<PixelType> data;
    uint32_t blocksInRow = 0;
    uint32_t col = 0;

    BlockType operator () ()
    {
        BlockType ret{};
        std::memcpy( &ret[ 0 ],  &data[ col * 4 + blocksInRow * 4 * 0 ], sizeof( PixelType ) * 4 );
        std::memcpy( &ret[ 4 ],  &data[ col * 4 + blocksInRow * 4 * 1 ], sizeof( PixelType ) * 4 );
        std::memcpy( &ret[ 8 ],  &data[ col * 4 + blocksInRow * 4 * 2 ], sizeof( PixelType ) * 4 );
        std::memcpy( &ret[ 12 ], &data[ col * 4 + blocksInRow * 4 * 3 ], sizeof( PixelType ) * 4 );
        ++col;
        if ( col == blocksInRow ) {
            col = 0;
            data = data.subspan( blocksInRow * sizeof( BlockType ) );
        }
        return ret;
    }
};

static fnta::Glyph getGlyphMetrics( FT_GlyphSlot slot, uint32_t lineHeight )
{
    assert( slot );
    auto metrics = slot->metrics;

    static constexpr uint32_t FONT_RESOLUTION_SCALE = 64u;
    metrics.width /= FONT_RESOLUTION_SCALE;
    metrics.height /= FONT_RESOLUTION_SCALE;
    metrics.horiAdvance /= FONT_RESOLUTION_SCALE;
    metrics.vertAdvance /= FONT_RESOLUTION_SCALE;
    metrics.horiBearingX /= FONT_RESOLUTION_SCALE;
    metrics.horiBearingY /= FONT_RESOLUTION_SCALE;
    metrics.horiBearingY = lineHeight - metrics.horiBearingY;

    fnta::Glyph glyph{};
    glyph.size[ 0 ] = static_cast<uint16_t>( metrics.width );
    glyph.size[ 1 ] = static_cast<uint16_t>( metrics.height );
    glyph.advance[ 0 ] = static_cast<int16_t>( metrics.horiAdvance );
    glyph.advance[ 1 ] = static_cast<int16_t>( metrics.vertAdvance );
    glyph.padding[ 0 ] = static_cast<int16_t>( metrics.horiBearingX );
    glyph.padding[ 1 ] = static_cast<int16_t>( metrics.horiBearingY );
    return glyph;
}

static std::vector<FT_Byte> loadFontFile( std::string_view path )
{
    std::ifstream ifs( (std::string)path, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) {
        std::cout << FAIL << "Font file cannot be open: " << path << std::endl;
        std::exit( 1 );
    }
    auto size = ifs.tellg();
    ifs.seekg( 0 );
    std::vector<FT_Byte> data{};
    data.resize( (decltype(data)::size_type)size );
    ifs.read( (char*)data.data(), size );
    return data;
}

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--px \"unsigned integer\" \u2012 glyph size in pixels, value must be > 0\n"
            "\t--src \"src/font/file/path.ext\" \u2012 source font file\n"
            "\t--out \"dst/font/atlas.fnta\" \u2012 destination of cooked atlas dataset\n"
            "\t--dds \"dst/font/image.dds\" \u2012 destination of cooked atlas image\n"
            "\nOptional Arguments:\n"
            "\t-h --help \u2012 prints this message and exit\n"
            ;
        return !args;
    }
    uint32_t size = 0;
    std::string_view argsSrcFont{};
    std::string_view argsDstFont{};
    std::string_view argsDstDDS{};

    auto err = []( auto&& msg ) -> bool
    {
        std::cout << FAIL << msg << std::endl;
        std::exit( 1 );
    };

    ( args.read( "--px", size ) && ( size > 0 ) ) || err( "--px \"unsigned integer\" > 0 \u2012 argument not specified or invalid" );
    args.read( "--src", argsSrcFont ) || err( "--src \"src/font/file/path.ext\" \u2012 argument not specified" );
    args.read( "--out", argsDstFont ) || err( "--out \"dst/font/atlas.fnta\" \u2012 argument not specified" );
    args.read( "--dds", argsDstDDS ) || err( "--dds \"dst/font/image.dds\" \u2012 argument not specified" );

    auto fontData = loadFontFile( argsSrcFont );

    struct DestroyOnExit {
        FT_Library library{};
        ~DestroyOnExit() { if ( library ) FT_Done_FreeType( library ); }
    };
    DestroyOnExit ft{};

    const FT_Error initOK = FT_Init_FreeType( &ft.library );
    exitOnFailed( initOK );

    const FT_Open_Args openArgs{
        .flags = FT_OPEN_MEMORY,
        .memory_base = fontData.data(),
        .memory_size = (FT_Long)fontData.size(),
    };

    FT_Face face{};
    const FT_Error newFaceOK = FT_Open_Face( ft.library, &openArgs, 0, &face );
    exitOnFailed( newFaceOK );

    float height = static_cast<float>( size );
    const FT_UInt pixelSize = static_cast<FT_UInt>( height );

    const FT_Error pixelSizeOK = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    exitOnFailed( pixelSizeOK );

    std::u32string charset{};
    auto genRange = []( auto& charset, char32_t begin, char32_t end )
    {
        auto range = end - begin;
        auto size = charset.size();
        charset.resize( size + range );
        std::iota( charset.begin() + (uint32_t)size, charset.end(), begin );
    };

    genRange( charset, 0x20, 0x7F ); // latin basic
    genRange( charset, 0xA1, 0x100 ); // latin supplement
    genRange( charset, 0x100, 0x180 ); // latin extended-A

    auto renderSlot = [&face, size]( char32_t ch ) -> Slot
    {
        const FT_UInt glyphIndex = FT_Get_Char_Index( face, ch );
        if ( glyphIndex == 0 ) {
            std::cout << WARN << "charcode not present in font: 0x" << std::hex << (uint32_t)ch << std::endl;
        }
        const FT_Error loadOK = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT );
        exitOnFailed( loadOK );

        FT_GlyphSlot slot = face->glyph;
        const FT_Error renderOK = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
        exitOnFailed( renderOK );
        if ( slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY ) {
            std::cout << FAIL << "pixel mode not gray" << std::endl;
            std::exit( 1 );
        }

        Slot ret{};
        ret.ch = ch;
        ret.glyph = getGlyphMetrics( slot, size );
        ret.pitch = (uint32_t)slot->bitmap.pitch;
        ret.rows = slot->bitmap.rows;
        auto* begin = reinterpret_cast<const uint8_t*>( slot->bitmap.buffer );
        auto* end = begin + ret.pitch * ret.rows;
        ret.bitmap = std::vector( begin, end );

        return ret;
    };
    std::deque<Slot> slots( charset.size() );
    std::transform( charset.begin(), charset.end(), slots.begin(), renderSlot );

    std::sort( slots.begin(), slots.end(), []( const auto& lhs, const auto& rhs )
    {
        if ( lhs.rows != rhs.rows ) return lhs.rows > rhs.rows;
        if ( lhs.pitch != rhs.pitch ) return lhs.pitch > rhs.pitch;
        return lhs.ch < rhs.ch;
    });

    uint32_t surfaceSizeNeeded = 0u;
    for ( auto&& slot : slots ) {
        surfaceSizeNeeded += ( slot.pitch + AtlasComposer::TILE_PADDING ) * ( slot.rows + AtlasComposer::TILE_PADDING );
    }
    float extraSlack = 1.0f; // TODO: if crash, extraSlack = (max of glyph area) / (average of glyph area)
    surfaceSizeNeeded = (uint32_t)((double)surfaceSizeNeeded * extraSlack );
    uint32_t surfExtent = 64;
    while ( surfExtent * surfExtent < surfaceSizeNeeded ) surfExtent <<= 1;

    std::deque<Slot> tightSlots{};
    while ( !slots.empty() ) {
        uint32_t width = surfExtent;
        tightSlots.push_back( slots.front() );
        slots.pop_front();
        width -= tightSlots.back().pitch + AtlasComposer::TILE_PADDING;
    retry:
        auto cmp = [width]( const Slot& g ) { return width >= g.pitch + AtlasComposer::TILE_PADDING; };
        auto it = std::find_if( slots.begin(), slots.end(), cmp );
        if ( it == slots.end() ) continue;

        width -= it->pitch + AtlasComposer::TILE_PADDING;
        tightSlots.push_back( *it );
        slots.erase( it );
        goto retry;
    }
    slots = std::move( tightSlots );

    std::vector<uint8_t> pixels( surfExtent * surfExtent );
    AtlasComposer atlas{
        .m_begin = pixels.data(),
        .m_end = pixels.data() + pixels.size(),
        .m_width = surfExtent,
        .m_height = surfExtent,
    };

    for ( auto& slot : slots ) {
        auto [ dst, x, y ] = atlas.findPlace( slot.pitch, slot.rows );
        slot.glyph.position[ 0 ] = static_cast<uint16_t>( x );
        slot.glyph.position[ 1 ] = static_cast<uint16_t>( y );
        if ( slot.pitch == 0 ) {
            continue;
        }
        std::copy( slot.bitmap.begin(), slot.bitmap.end(), dst );
    }

    std::vector<BC4unorm> texture( pixels.size() / 16 );
    {
        std::vector<Swizzler<>::BlockType> blocks( texture.size() );
        Swizzler<> swizzler{ pixels, surfExtent / 4 };
        std::generate( blocks.begin(), blocks.end(), swizzler );

        [[maybe_unused]]
        auto compress = []( const Swizzler<>::BlockType& block ) -> BC4unorm
        {
            BC4unorm ret{};
            auto [ min, max ] = std::minmax_element( block.begin(), block.end() );
            ret.alpha0 = *max;
            ret.alpha1 = *min;
            const std::array<uint8_t, 8> arr1{ ret.alpha0, ret.alpha1,
                lerp( ret.alpha0, ret.alpha1, 1.0f / 7.0f),
                lerp( ret.alpha0, ret.alpha1, 2.0f / 7.0f),
                lerp( ret.alpha0, ret.alpha1, 3.0f / 7.0f),
                lerp( ret.alpha0, ret.alpha1, 4.0f / 7.0f),
                lerp( ret.alpha0, ret.alpha1, 5.0f / 7.0f),
                lerp( ret.alpha0, ret.alpha1, 6.0f / 7.0f),
            };
            auto min_element = []( std::span<const uint8_t> alphas, uint8_t ref ) -> uint8_t
            {
                int dist = 0xFFFF;
                uint8_t indice = 0;
                for ( uint8_t i = 0; i < alphas.size(); ++i ) {
                    int d = std::abs( (int)ref - (int)alphas[ i ] );
                    if ( d >= dist ) continue;
                    dist = d;
                    indice = i;
                }
                return indice;
            };
            uint64_t indices = 0;
            // computing in reverse to preserve 3-bit indice ordering
            for ( auto it = block.rbegin(); it != block.rend(); ++it ) {
                indices <<= 3;
                indices |= min_element( arr1, *it );
            }
            ret.aindexes[ 0 ] = (indices >> 0) & 0xFF;
            ret.aindexes[ 1 ] = (indices >> 8) & 0xFF;
            ret.aindexes[ 2 ] = (indices >> 16) & 0xFF;
            ret.aindexes[ 3 ] = (indices >> 24) & 0xFF;
            ret.aindexes[ 4 ] = (indices >> 32) & 0xFF;
            ret.aindexes[ 5 ] = (indices >> 40) & 0xFF;
            return ret;
        };
        std::transform( blocks.begin(), blocks.end(), texture.begin(), compress );
    }


    std::sort( slots.begin(), slots.end(), []( const auto& lhs, const auto& rhs ) { return lhs.ch < rhs.ch; });

    using Flags = dds::Header::Flags;
    using Caps = dds::Header::Caps;
    dds::Header ddsHeader{
        .flags = static_cast<Flags>( Flags::fCaps | Flags::fWidth | Flags::fHeight | Flags::fPixelFormat ),
        .height = surfExtent,
        .width = surfExtent,
        .pitchOrLinearSize = (uint32_t)texture.size() * (uint32_t)sizeof(BC4unorm),
        .mipMapCount = 1,
        .pixelFormat{
            .flags = dds::PixelFormat::Flags::fFourCC,
            .fourCC = dds::c_dxgi,
        },
        .caps = Caps::fTexture,
    };
    dds::dxgi::Header dxgiHeader{
        .format = dds::dxgi::Format::BC4_UNORM,
        .dimension = dds::dxgi::Dimension::eTexture2D,
        .arraySize = 1,
    };
    std::ofstream ofs{ (std::string)argsDstDDS, std::ios::binary };
    if ( !ofs.is_open() ) {
        std::cout << FAIL << "Cannot open file for write: " << argsDstDDS << std::endl;
        return 1;
    }

    ofs.write( (const char*)&ddsHeader, sizeof( ddsHeader ) );
    ofs.write( (const char*)&dxgiHeader, sizeof( dxgiHeader ) );
    ofs.write( (const char*)texture.data(), static_cast<std::streamsize>( texture.size() * (uint32_t)sizeof(BC4unorm) ) );
    ofs.close();

    fnta::Header fntaHeader{
        .count = (uint32_t)charset.size(),
        .width = surfExtent,
        .height = surfExtent,
        .lineHeight = size,
    };
    ofs = std::ofstream( (std::string)argsDstFont, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << FAIL << "Cannot open font atlas to write: " << argsDstFont << std::endl;
        return 1;
    }
    ofs.write( (const char*)&fntaHeader, sizeof( fntaHeader ) );
    ofs.write( (const char*)charset.data(), (std::streamsize)charset.size() * 4 );
    for ( auto&& slot : slots ) {
        ofs.write( (const char*)&slot.glyph, sizeof( slot.glyph ) );
    }
    ofs.close();

    return 0;
}
