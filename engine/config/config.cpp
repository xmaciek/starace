#include <config/config.hpp>

#include <unicode/unicode.hpp>

#include "cfg.hpp"

#include <bit>
#include <stack>
#include <cstring>

namespace cfg {
std::string_view Entry::toString() const
{
    return { m_value.begin(), m_value.end() };
}

std::pmr::u32string Entry::toString32() const
{
    unicode::Transcoder utf{ m_value };
    std::pmr::u32string ret;
    ret.resize( utf.length() );
    std::ranges::for_each( ret, utf );
    return ret;
}


float Entry::toFloat() const
{
    float f = 0.0f;
    std::from_chars( m_value.c_str(), m_value.c_str() + m_value.size(), f );
    return f;
}

const Entry* Entry::begin() const
{
    return m_data.data();
}

const Entry* Entry::end() const
{
    return m_data.data() + m_data.size();
}

std::string_view Entry::name() const
{
    return m_name;
}

const Entry& Entry::operator [] ( std::string_view name ) const
{
    static Entry e;
    const auto& v = m_data;
    auto cmp = [name]( const Entry& ent ) { return ent.m_name == name; };
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
                stack.top()->m_value = std::pmr::string{ strBegin, strEnd };
                stack.pop();
                expectedToken = eNameOrPop;
                it.advance( (uint32_t)std::distance( it.begin(), strEnd + 1 ) );
                continue;
                }
            }
            [[fallthrough]];

        case eValue:
            if ( token.userEnum != TokenIterator::c_unknown ) { return {}; }
            stack.top()->m_value = std::pmr::string{ token.data, token.data + token.length };
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
            stack.top()->m_data.push_back( { .m_name = std::pmr::string{ token.data, token.data + token.length } } );
            stack.push( &(stack.top()->m_data.back()) );
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

