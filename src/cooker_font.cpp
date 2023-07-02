#include <extra/dds.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <span>
#include <tuple>
#include <iostream>
#include <string_view>
#include <vector>
#include <fstream>
#include <charconv>
#include <numeric>

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

struct FontHeader {
    uint32_t magic = 'ATNF';
    uint32_t version = 1;
    uint32_t count = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

static void exitOnFailed( FT_Error ec )
{
    if ( ec == 0 ) return;
    std::cout << FAIL << FT_Error_String( ec ) << std::endl;
    std::exit( 1 );
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

    static constexpr uint32_t TILE_PADDING = 2u;

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

struct Glyph {
    uint16_t position[ 2 ];
    uint16_t size[ 2 ];
    int16_t advance[ 2 ];
    int16_t padding[ 2 ];
};

struct Slot {
    Glyph glyph{};
    char32_t ch{};
    uint32_t pitch{};
    uint32_t rows{};
    std::vector<uint8_t> bitmap{};
};

static Glyph getGlyphMetrics( FT_GlyphSlot slot )
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
    Glyph glyph{};
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
        return {};
    }
    auto size = ifs.tellg();
    ifs.seekg( 0 );
    std::vector<FT_Byte> data{};
    data.resize( (decltype(data)::size_type)size );
    ifs.read( (char*)data.data(), size );
    return data;
}

int main( int argc, char** argv )
{
    if ( argc < 5 ) {
        std::cout << FAIL <<
            "Insufficient number of arguments, expected: "
            "<size> "
            "<src/font/file/path.ext> "
            "<dst/font/atlas.font> "
            "<dst/font/image.dds>"
            << std::endl;
            return 1;
    }

    std::string_view argsSize = argv[ 1 ];
    std::string_view argsSrcFont = argv[ 2 ];
    std::string_view argsDstFont = argv[ 3 ];
    std::string_view argsDstDDS = argv[ 4 ];

    uint32_t size = 0;
    if ( auto ret = std::from_chars( argsSize.begin(), argsSize.end(), size ); ret.ec != std::errc{} ) {
        std::cout << FAIL << "Failed to convert size value to uint32 from " << argsSize << std::endl;
        return 1;
    }
    if ( size == 0 ) {
        std::cout << FAIL << "Font size cannot be 0" << std::endl;
        return 1;
    }

    auto fontData = loadFontFile( argsSrcFont );
    if ( fontData.empty() ) {
        return 1;
    }

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

    auto renderSlot = [&face]( char32_t ch ) -> Slot
    {
        const FT_UInt glyphIndex = FT_Get_Char_Index( face, ch );

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
        ret.glyph = getGlyphMetrics( slot );
        ret.pitch = (uint32_t)slot->bitmap.pitch;
        ret.rows = slot->bitmap.rows;
        auto* begin = reinterpret_cast<const uint8_t*>( slot->bitmap.buffer );
        auto* end = begin + ret.pitch * ret.rows;
        ret.bitmap = std::vector( begin, end );
        return ret;
    };
    std::vector<Slot> slots( charset.size() );
    std::transform( charset.begin(), charset.end(), slots.begin(), renderSlot );

    std::sort( slots.begin(), slots.end(), []( const auto& lhs, const auto& rhs )
    {
        if ( lhs.pitch != rhs.pitch ) return lhs.pitch > rhs.pitch;
        if ( lhs.rows != rhs.rows ) return lhs.rows > rhs.rows;
        return lhs.ch < rhs.ch;
    });
    uint32_t surfaceSizeNeeded = 0u;
    for ( auto&& slot : slots ) {
        surfaceSizeNeeded += ( slot.pitch + AtlasComposer::TILE_PADDING ) * ( slot.rows + AtlasComposer::TILE_PADDING );
    }
    surfaceSizeNeeded = (uint32_t)((double)surfaceSizeNeeded * 1.618 ); // add extra slack

    uint32_t surfExtent = 4;
    while ( surfExtent * surfExtent < surfaceSizeNeeded ) surfExtent <<= 1;

    std::pmr::vector<uint8_t> texture( surfExtent * surfExtent );
    AtlasComposer atlas{
        .m_begin = texture.data(),
        .m_end = texture.data() + texture.size(),
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

    std::sort( slots.begin(), slots.end(), []( const auto& lhs, const auto& rhs )
    {
        return lhs.ch < rhs.ch;
    });

    using Flags = dds::Header::Flags;
    using Caps = dds::Header::Caps;
    dds::Header ddsHeader{
        .flags = (Flags)( Flags::fCaps | Flags::fWidth | Flags::fHeight | Flags::fPixelFormat | Flags::fMipMapCount ),
        .height = surfExtent,
        .width = surfExtent,
        .pitchOrLinearSize = (uint32_t)texture.size(),
        .mipMapCount = 1,
        .pixelFormat{
            .flags = dds::PixelFormat::Flags::fFourCC,
            .fourCC = dds::c_dxgi,
        },
        .caps = (Caps)( Caps::fTexture | Caps::fMipMap ),
    };
    dds::dxgi::Header dxgiHeader{
        .format = dds::dxgi::Format::R8_UNORM,
        .dimension = dds::dxgi::Dimension::eTexture2D,
    };
    std::ofstream ofs{ (std::string)argsDstDDS, std::ios::binary };
    if ( !ofs.is_open() ) {
        std::cout << FAIL << "Cannot open file for write: " << argsDstDDS << std::endl;
        return 1;
    }

    ofs.write( (const char*)&ddsHeader, sizeof( ddsHeader ) );
    ofs.write( (const char*)&dxgiHeader, sizeof( dxgiHeader ) );
    ofs.write( (const char*)texture.data(), static_cast<std::streamsize>( texture.size() ) );
    ofs.close();

    FontHeader fontHeader{
        .count = (uint32_t)charset.size(),
        .width = surfExtent,
        .height = surfExtent,
    };
    ofs = std::ofstream( (std::string)argsDstFont, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << FAIL << "Cannot open font atlas to write: " << argsDstFont << std::endl;
        return 1;
    }
    ofs.write( (const char*)&fontHeader, sizeof( fontHeader ) );
    ofs.write( (const char*)charset.data(), (std::streamsize)charset.size() * 4 );
    for ( auto&& slot : slots ) {
        ofs.write( (const char*)&slot.glyph, sizeof( slot.glyph ) );
    }
    ofs.close();

    return 0;
}
