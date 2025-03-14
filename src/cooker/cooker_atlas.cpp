#include "cooker_common.hpp"

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

    args.read( "-i", argsSrcAtlas ) || cooker::error( "\t-i \"src/atlas.txt\" \u2012 argument not specified" );
    args.read( "-o", argsDstAtlas ) || cooker::error( "\t-o \"dst/atlas.fnta\" \u2012 argument not specified" );
    const bool argsRemap = args.read( "--remap" );

    std::ifstream ifs{ (std::string)argsSrcAtlas, std::ios::binary | std::ios::ate };
    if ( !ifs.is_open() ) {
        cooker::error( "Cannot open source atlas:", argsSrcAtlas );
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

    uint32_t remapOffset = 0;
    Hash hash;
    for ( auto&& it : entry ) {
        Hash::value_type hh = hash( *it );
        switch ( hh ) {
        case "width"_hash: header.width = it.toInt<uint16_t>(); continue;
        case "height"_hash: header.height = it.toInt<uint16_t>(); continue;
        case "lineHeight"_hash: header.lineHeight = it.toInt<uint32_t>(); continue;
        case "remapOffset"_hash: remapOffset = it.toInt<uint32_t>(); continue;
        }

        auto& glyph = glyphs.emplace_back();
        for ( auto&& property : it ) {
            switch ( hash( *property ) ) {
            case "x"_hash: glyph.data.position[ 0 ] = property.toInt<uint16_t>(); continue;
            case "y"_hash: glyph.data.position[ 1 ] = property.toInt<uint16_t>(); continue;
            case "px"_hash: glyph.data.padding[ 0 ] = property.toInt<int16_t>(); continue;
            case "py"_hash: glyph.data.padding[ 1 ] = property.toInt<int16_t>(); continue;
            case "w"_hash: glyph.data.size[ 0 ] = property.toInt<uint16_t>(); continue;
            case "h"_hash: glyph.data.size[ 1 ] = property.toInt<uint16_t>(); continue;
            }
        }
        glyph.data.advance[ 0 ] = static_cast<int16_t>( glyph.data.size[ 0 ] );
        glyph.data.advance[ 1 ] = static_cast<int16_t>( glyph.data.size[ 1 ] );
        glyph.ch = hh;
    }
    if ( argsRemap ) {
        std::for_each( glyphs.begin(), glyphs.end(), [remapOffset]( auto& h )
        {
            switch ( h.ch ) {
            case "x"_hash: h.ch = fnta::Input::X + remapOffset; break;
            case "y"_hash: h.ch = fnta::Input::Y + remapOffset; break;
            case "a"_hash: h.ch = fnta::Input::A + remapOffset; break;
            case "b"_hash: h.ch = fnta::Input::B + remapOffset; break;
            case "start"_hash: h.ch = fnta::Input::Start + remapOffset; break;
            case "select"_hash: h.ch = fnta::Input::Select + remapOffset; break;
            case "touch"_hash: h.ch = fnta::Input::Touch + remapOffset; break;
            case "up"_hash: h.ch = fnta::Input::Up + remapOffset; break;
            case "down"_hash: h.ch = fnta::Input::Down + remapOffset; break;
            case "left"_hash: h.ch = fnta::Input::Left + remapOffset; break;
            case "right"_hash: h.ch = fnta::Input::Right + remapOffset; break;
            case "lb"_hash: h.ch = fnta::Input::LB + remapOffset; break;
            case "lt"_hash: h.ch = fnta::Input::LT + remapOffset; break;
            case "lx"_hash: h.ch = fnta::Input::LX + remapOffset; break;
            case "ly"_hash: h.ch = fnta::Input::LY + remapOffset; break;
            case "l"_hash: h.ch = fnta::Input::R + remapOffset; break;
            case "rb"_hash: h.ch = fnta::Input::RB + remapOffset; break;
            case "rt"_hash: h.ch = fnta::Input::RT + remapOffset; break;
            case "rx"_hash: h.ch = fnta::Input::RX + remapOffset; break;
            case "ry"_hash: h.ch = fnta::Input::RY + remapOffset; break;
            case "r"_hash: h.ch = fnta::Input::R + remapOffset; break;
            default: break;
            }
        } );
    }
    std::sort( glyphs.begin(), glyphs.end(), []( const auto& lhs, const auto& rhs ) { return lhs.ch < rhs.ch; } );
    header.count = (uint32_t)glyphs.size();

    auto ofs = cooker::openWrite( argsDstAtlas );
    cooker::write( ofs, header );
    std::ranges::for_each( glyphs, [&ofs]( const auto& g ) { cooker::write( ofs, g.ch ); } );
    std::ranges::for_each( glyphs, [&ofs]( const auto& g ) { cooker::write( ofs, g.data ); } );
    return 0;
}
