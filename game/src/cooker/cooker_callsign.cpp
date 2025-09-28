#include <cooker/common.hpp>

#include <extra/csg.hpp>
#include <extra/args.hpp>
#include <unicode/unicode.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <string>
#include <type_traits>
#include <vector>

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--src \"<file/path.txt>\" \u2012 specifies source file\n"
            "\t--dst \"<file/path.csg>\" \u2012 specifies output\n"
            "\nOptional arguments:\n"
            "\t-h --help \u2012 prints this message and exits\n"
            ;
        return !args;
    }
    std::string_view argSrc{};
    std::string_view argDst{};
    args.read( "--src", argSrc ) || cooker::error( "--src <file/path.txt> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || cooker::error( "--dst <file/path.csg> \u2012 argument not specified" );

    std::ifstream ifs( (std::string)argSrc );
    if ( !ifs.is_open() ) cooker::error( "cannot open src file:", argSrc );

    std::pmr::vector<csg::Callsign> callsigns;
    callsigns.reserve( 400 );
    for ( std::string line; std::getline( ifs, line ); ) {
        unicode::Transcoder transcoder{ line };
        uint32_t length = transcoder.length();
        if ( length >= csg::Callsign::CAPACITY ) cooker::error( "callsign too long", line );
        auto& cs = callsigns.emplace_back();
        std::generate_n( std::begin( cs.str ), length, transcoder );
    }
    ifs.close();

    std::sort( callsigns.begin(), callsigns.end(), []( auto& lhs, auto& rhs ) { return (std::u32string_view)lhs < (std::u32string_view)rhs; } );
    csg::Header header{ .count = static_cast<uint32_t>( callsigns.size() ), };
    auto ofs = cooker::openWrite( argDst );
    cooker::write( ofs, header );
    cooker::write( ofs, callsigns );
    return 0;
}
