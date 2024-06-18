#include <config/config.hpp>
#include <extra/fnta.hpp>
#include <extra/args.hpp>
#include <shared/hash.hpp>


#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>
#include <numeric>

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

struct Slot {
    fnta::Glyph glyph{};
    char32_t ch{};
    uint32_t pitch{};
    uint32_t rows{};
    std::vector<uint8_t> bitmap{};
};

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t-i \"src/atlas.txt\" \u2012 source text file describing atlas\n"
            "\t-o \"dst/atlas.fnta\" \u2012 destination of cooked atlas dataset\n"
            "\nOptional Arguments:\n"
            "\t-h --help \u2012 prints this message and exit\n"
            ;
        return !args;
    }

    std::string_view argsSrcAtlas{};
    std::string_view argsDstAtlas{};

    auto err = []( auto&& msg ) -> bool
    {
        std::cout << FAIL << msg << std::endl;
        std::exit( 1 );
    };

    args.read( "-i", argsSrcAtlas ) || err( "\t-i \"src/atlas.txt\" \u2012 argument not specified" );
    args.read( "-o", argsDstAtlas ) || err( "\t-o \"dst/atlas.fnta\" \u2012 argument not specified" );

    std::ifstream ifs{ (std::string)argsSrcAtlas, std::ios::binary | std::ios::ate };
    if ( !ifs.is_open() ) {
        std::cout << FAIL << "Cannot open source atlas to: " << argsSrcAtlas << std::endl;
        return 1;
    }

    auto fileSize = ifs.tellg();
    std::vector<uint8_t> src( static_cast<size_t>( fileSize ) );
    ifs.seekg(0);
    ifs.read( reinterpret_cast<char*>( src.data() ), fileSize );
    ifs.close();

    cfg::Entry entry = cfg::Entry::fromData( src );
    struct GlpyhInfo {
        char32_t ch;
        fnta::Glyph data;
    };
    std::vector<GlpyhInfo> glyphs;
    fnta::Header header{
        .width = 256,
        .height = 256,
    };

    Hash hash;
    for ( auto&& it : entry ) {
        Hash::value_type hh = hash( *it );
        switch ( hh ) {
        case "width"_hash: header.width = it.toInt<uint16_t>(); continue;
        case "height"_hash: header.height = it.toInt<uint16_t>(); continue;
        }
        auto& glyph = glyphs.emplace_back();
        for ( auto&& property : it ) {
            switch ( hash( *property ) ) {
            case "x"_hash: glyph.data.position[ 0 ] = property.toInt<uint16_t>(); continue;
            case "y"_hash: glyph.data.position[ 1 ] = property.toInt<uint16_t>(); continue;
            case "w"_hash: glyph.data.size[ 0 ] = property.toInt<uint16_t>(); continue;
            case "h"_hash: glyph.data.size[ 1 ] = property.toInt<uint16_t>(); continue;
            }
        }
        glyph.data.advance[ 0 ] = static_cast<int16_t>( glyph.data.size[ 0 ] );
        glyph.data.advance[ 1 ] = static_cast<int16_t>( glyph.data.size[ 1 ] );
        glyph.ch = hh;
    }
    std::sort( glyphs.begin(), glyphs.end(), []( const auto& lhs, const auto& rhs ) { return lhs.ch < rhs.ch; } );
    header.count = (uint32_t)glyphs.size();
    header.lineHeight = std::accumulate( glyphs.begin(), glyphs.end(), 0u, []( uint32_t r, const auto& g ) { return r + g.data.size[ 1 ]; } );
    header.lineHeight /= header.count;

    std::ofstream ofs{ (std::string)argsDstAtlas, std::ios::binary };
    if ( !ofs.is_open() ) {
        std::cout << FAIL << "Cannot open destination atlas to write: " << argsDstAtlas << std::endl;
        return 1;
    }
    ofs.write( (const char*)&header, sizeof( header ) );
    for ( auto&& g : glyphs ) {
        ofs.write( (const char*)&g.ch, sizeof( g.ch ) );
    }
    for ( auto&& g : glyphs ) {
        ofs.write( (const char*)&g.data, sizeof( g.data ) );
    }
    ofs.close();

    return 0;
}
