#include <config/config.hpp>

#include "cfg.hpp"

#include <cassert>
#include <stack>
#include <cstring>

namespace cfg {
std::string_view Entry::toString() const
{
    return { value.begin(), value.end() };
}

int Entry::toInt() const
{
    char** e = nullptr;
    return std::strtol( value.c_str(), e, 10 );
}

float Entry::toFloat() const
{
    char** e = nullptr;
    return std::strtof( value.c_str(), e );
}

const Entry* Entry::begin() const
{
    return &*data.cbegin();
}

const Entry* Entry::end() const
{
    return &*data.cend();
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
            if ( token.userEnum == '{' ) {
                expectedToken = eName;
                continue;
            }
            [[fallthrough]];

        case eValue:
            assert( token.userEnum == TokenIterator::c_unknown );
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
            assert( token.userEnum == TokenIterator::c_unknown );
            stack.top()->data.push_back( { .name = std::pmr::string{ token.data, token.data + token.length } } );
            stack.push( &(stack.top()->data.back()) );
            expectedToken = eAssign;
            continue;

        case eAssign:
            assert( token.userEnum == '=' );
            expectedToken = eValueOrPush;
            continue;

        }
    }
    return ret;
}



}

