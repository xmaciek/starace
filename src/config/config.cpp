#include <config/config.hpp>

#include "cfg.hpp"

#include <bit>
#include <stack>
#include <cstring>

namespace cfg {
std::string_view Entry::toString() const
{
    return { value.begin(), value.end() };
}

std::pmr::u32string Entry::toString32() const
{
    auto utf8length = []( auto begin, auto end ) noexcept -> uint32_t
    {
        auto countCharsUTF8 = []( char c )
        {
            assert( c != '\0' );
            switch ( std::countl_one( (unsigned char)c ) ) {
            [[likely]]
            case 0: return 1;
            case 1: return 0;
            case 2: return 1;
            case 3: return 1;
            case 4: return 1;
            [[unlikely]]
            default:
                assert( !"broken UTF-8 encoding" );
                return 0;
            }
        };

        uint32_t ret = 0;
        while ( begin != end ) ret += countCharsUTF8( *(begin++) );
        return ret;
    };

    auto gibUTF32 = [ptr = value.begin()]() mutable noexcept -> char32_t
    {
        auto countBytesUTF8 = []( char c )
        {
            assert( c != '\0' );
            switch ( std::countl_one( (unsigned char)c ) ) {
            [[likely]]
            case 0: return 1;
            case 1: return 0;
            case 2: return 2;
            case 3: return 3;
            case 4: return 4;
            [[unlikely]]
            default:
                assert( !"broken UTF-8 encoding" );
                return 0;
            }
        };

        uint32_t count = countBytesUTF8( *ptr );
        char32_t ret = '\0';
        auto extract = []( char32_t& v, auto& ptr, char mask )
        {
            v = ( v << 6 ) | ( *(ptr++) & mask );
        };
        switch ( count ) {
        [[likely]]
        case 1:
            ret = *(ptr++);
            break;
        case 2:
            extract( ret, ptr, 0b00011111 );
            extract( ret, ptr, 0b00111111 );
            break;
        case 3:
            extract( ret, ptr, 0b00001111 );
            extract( ret, ptr, 0b00111111 );
            extract( ret, ptr, 0b00111111 );
            break;
        case 4:
            extract( ret, ptr, 0b00000111 );
            extract( ret, ptr, 0b00111111 );
            extract( ret, ptr, 0b00111111 );
            extract( ret, ptr, 0b00111111 );
            break;
        [[unlikely]]
        default:
            assert( !"broken UTF-8 encoding" );
            return 0;
        }
        return ret;
    };

    auto count = utf8length( value.begin(), value.end() );
    std::pmr::u32string ret;
    ret.resize( count );
    std::generate( ret.begin(), ret.end(), gibUTF32 );
    return ret;
}


float Entry::toFloat() const
{
    float f = 0.0f;
    std::from_chars( value.c_str(), value.c_str() + value.size(), f );
    return f;
}

const Entry* Entry::begin() const
{
    return data.data();
}

const Entry* Entry::end() const
{
    return data.data() + data.size();
}

std::string_view Entry::operator * () const
{
    return name;
}

const Entry& Entry::operator [] ( std::string_view name ) const
{
    static Entry e;
    const auto& v = data;
    auto cmp = [name]( const Entry& ent ) { return *ent == name; };
    auto it = std::find_if( v.begin(), v.end(), cmp );
    return ( it == v.cend() ) ? e : *it;
}

Entry Entry::fromData( std::pmr::vector<uint8_t>&& data )
{
    const char* begin = reinterpret_cast<const char*>( data.data() );
    return fromData( std::span<const char>{ begin, begin + data.size() } );
}

Entry Entry::fromData( std::span<const uint8_t> data )
{
    const char* begin = reinterpret_cast<const char*>( data.data() );
    return fromData( std::span<const char>{ begin, begin + data.size() } );
}

Entry Entry::fromData( std::span<const char> data )
{
    using std::literals::string_view_literals::operator""sv;
    auto whitespace = U" \t\n\r"sv;

    enum AST {
        eName,
        eAssign,
        eValueOrPush,
        eValue,
        eNameOrPop,
    };
    AST expectedToken = eName;

    Entry ret{};
    std::stack<Entry*> stack;
    stack.push( &ret );

    TokenIterator it{ c_separators, data };
    for ( auto token = *it; ( token = *it ); ++it ) {
        if ( whitespace.find( static_cast<char32_t>( token.userEnum ) ) != std::u32string_view::npos ) { continue; }
        switch ( expectedToken ) {
        case eValueOrPush:
            switch ( token.userEnum ) {
            case U'{':
                expectedToken = eName;
                continue;
            case U'"': {
                auto strBegin = it.begin();
                auto strEnd = std::find_if( strBegin, it.end(), EndQuote{} );

                if ( strEnd == it.end() ) { return {}; }

                if ( *strEnd != '"' ) { return {}; }
                stack.top()->value = std::pmr::string{ strBegin, strEnd };
                stack.pop();
                expectedToken = eNameOrPop;
                it.advance( std::distance( it.begin(), strEnd + 1 ) );
                continue;
                }
            }
            [[fallthrough]];

        case eValue:
            if ( token.userEnum != TokenIterator::c_unknown ) { return {}; }
            stack.top()->value = std::pmr::string{ token.data, token.data + token.length };
            stack.pop();
            expectedToken = eNameOrPop;
            continue;

        case eNameOrPop:
            if ( token.userEnum == '}' ) {
                stack.pop();
                continue;
            }
            expectedToken = eName;
            [[fallthrough]];

        case eName:
            if ( token.userEnum != TokenIterator::c_unknown ) { return {}; }
            stack.top()->data.push_back( { .name = std::pmr::string{ token.data, token.data + token.length } } );
            stack.push( &(stack.top()->data.back()) );
            expectedToken = eAssign;
            continue;

        case eAssign:
            if ( token.userEnum != '=' ) { return {}; }
            expectedToken = eValueOrPush;
            continue;

        }
    }
    return ret;
}



}

