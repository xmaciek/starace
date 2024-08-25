#include <extra/csg.hpp>
#include <extra/args.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <string>
#include <type_traits>
#include <vector>

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

[[noreturn]]
static bool exitOnFailed( std::string_view msg, std::string_view arg )
{
    std::cout << FAIL << msg << " " << arg << std::endl;
    std::exit( 1 );
}

[[noreturn]]
static bool exitOnFailed( std::string_view msg )
{
    std::cout << FAIL << msg << std::endl;
    std::exit( 1 );
}


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
    args.read( "--src", argSrc ) || exitOnFailed( "--src <file/path.txt> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || exitOnFailed( "--dst <file/path.csg> \u2012 argument not specified" );

    std::ifstream ifs( (std::string)argSrc );
    std::pmr::vector<csg::Callsign> callsigns;
    callsigns.reserve( 200 );
    if ( !ifs.is_open() ) exitOnFailed( "cannot open src file:", argSrc );
    for ( std::string line; std::getline( ifs, line ); ) {
        if ( line.size() >= csg::Callsign::CAPACITY ) exitOnFailed( "callsign too long", line );
        auto& cs = callsigns.emplace_back();
        // TODO: utf32
        std::transform( line.begin(), line.end(), std::begin( cs.str ), [](char c ) -> char32_t { return static_cast<char32_t>( c ); } );
    }
    ifs.close();
    std::sort( callsigns.begin(), callsigns.end(), []( auto& lhs, auto& rhs ) { return (std::u32string)lhs < (std::u32string)rhs; } );
    csg::Header header{ .count = static_cast<uint32_t>( callsigns.size() ), };
    std::ofstream ofs( std::string( argDst ), std::ios::binary );
    if ( !ofs.is_open() ) exitOnFailed( "cannot open dst file:", argDst );
    ofs.write( reinterpret_cast<char*>( &header ), sizeof( header ) );
    ofs.write( reinterpret_cast<char*>( callsigns.data() ), static_cast<std::streamsize>( callsigns.size() * sizeof( csg::Callsign ) ) );
    ofs.close();
    return 0;
}
