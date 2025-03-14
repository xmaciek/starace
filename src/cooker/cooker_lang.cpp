#include "cooker_common.hpp"

#include <extra/args.hpp>
#include <extra/lang.hpp>
#include <extra/unicode.hpp>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <span>
#include <string_view>
#include <tuple>
#include <memory_resource>
#include <vector>

static bool getLine( std::span<const char>& stream, std::span<const char>& out )
{
    if ( stream.empty() ) return false;
    auto it = std::ranges::find( stream, '\n' );
    out = { stream.begin(), it };
    auto delimSkip = it == stream.end() ? it : it + 1;
    stream = stream.subspan( (size_t)std::distance( stream.begin(), delimSkip ) );
    return true;
};

struct ParserCSV {
    using key_type = lang::KeyType;
    using value_type = char32_t;

    std::span<const char> m_stream;
    std::pmr::vector<key_type> m_keys;
    std::pmr::vector<value_type> m_string;

    ~ParserCSV() = default;
    ParserCSV( std::span<const char> data )
    : m_stream{ data }
    {
    }
    void process()
    {
        std::span<const char> line;
        uint32_t lineCount = 0;
        while ( getLine( m_stream, line ) ) {
            lineCount++;
            if ( line.empty() ) continue;
            if ( line[ 0 ] != '"' ) cooker::error( "key must begin with quotes, at line", lineCount );

            auto it = line.begin() + 1;
            const auto keyBegin = it;

            auto keyEnd = std::find( it, line.end(), '"' );
            if ( keyEnd == line.end() ) cooker::error( "key must end with quotes, at line", lineCount );
            it = keyEnd;

            const auto separator = std::find( it, line.end(), ',' );
            if ( separator == line.end() ) cooker::error( "key value must be comma separated, at line", lineCount );
            it = separator + 1;

            auto valueBegin = std::find( it, line.end(), '"' );
            if ( valueBegin == line.end() ) cooker::error( "value must begin with quotes, at line", lineCount );
            valueBegin++;
            it = valueBegin;

            const auto valueEnd = std::find( it, line.end(), '"' );
            if ( valueEnd == line.end() ) cooker::error( "value must end with quotes, at line", lineCount );

            unicode::Transcoder utf{ std::string_view{ valueBegin, valueEnd } };
            const auto offset = m_string.size();
            const auto size = utf.length();
            m_string.resize( offset + size + 1u );

            auto begin = m_string.begin();
            std::advance( begin, offset );

            auto end =  m_string.begin();
            std::advance( end, offset + size );

            std::generate( begin, end, std::move( utf ) );

            m_keys.emplace_back( Hash{}( std::string_view{ keyBegin, keyEnd } ), offset, size );
        }
    }
    void sort()
    {
        std::sort( m_keys.begin(), m_keys.end() );
    }
};

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--src \"src/file/path.csv\" \u2012 source localization file\n"
            "\t--dst \"dst/file/path.lang\" \u2012 destination of cooked localization\n"
            "\nOptional Arguments:\n"
            "\t-h --help \u2012 prints this message and exit\n"
            ;
        return !args;
    }
    std::string_view argsSrc{};
    std::string_view argsDst{};

    args.read( "--src", argsSrc ) || cooker::error( "--src \"src/file/path.csv\" \u2012 argument not specified" );
    args.read( "--dst", argsDst ) || cooker::error( "--dst \"dst/file/path.lang\" \u2012 argument not specified" );

    std::ifstream ifs( (std::string)argsSrc, std::ios::binary | std::ios::ate );
    ifs.is_open() || cooker::error( "source file cannot be open", argsSrc );
    std::pmr::vector<char> data( (size_t)ifs.tellg() );
    ifs.seekg( 0 );
    ifs.read( data.data(), (std::streamsize)data.size() );
    ifs.close();

    ParserCSV parser( data );
    parser.process();
    parser.sort();

    lang::Header header{
        .count = static_cast<uint32_t>( parser.m_keys.size() ),
        .string = static_cast<uint32_t>( parser.m_string.size() ),
    };

    auto ofs = cooker::openWrite( argsDst );
    cooker::write( ofs, header );
    cooker::write( ofs, parser.m_keys );
    cooker::write( ofs, parser.m_string );
    return 0;
}
