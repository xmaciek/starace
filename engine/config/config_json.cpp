#include <config/config.hpp>

#include "cfg.hpp"

#include <string_view>
#include <array>
#include <utility>
#include <expected>

// JSON RFC 8259

static constexpr std::array SEPARATORS = {
    ' ',
    ':',
    '\n',
    '\r',
    '\t',
    '{',
    '}',
    '[',
    ']',
    ',',
    '"',
};

using std::literals::string_view_literals::operator""sv;
static constexpr auto WHITESPACES = U" \t\n\r"sv;

using namespace cfg;

enum AST : char32_t {
    eBegin = U'{',
    eEnd = U'}',
    eName = U'"',
    eColon = U':',
    eArray = U'[',
    eArrayEnd = U']',
    eComma = U',',
    eObjectArrayValue = 0x80,
    eObjectArrayArrayEndValue,
    eObjectArrayEndValue,
    eNameEnd,
    eCommaEnd,
    eCommaArrayEnd,
    eStringVal,
};

struct Detokenizer {
    enum class ErrorCode : uint32_t {
        none,
        eNotImplemented,
        eToken,
        eEndOfStream,
    };
    using enum ErrorCode;
    using RetVal = std::expected<std::pmr::string, ErrorCode>;
    using ComplexVal = std::expected<Entry, ErrorCode>;

    static ComplexVal fromObject( TokenIterator& );
    static ComplexVal fromArray( TokenIterator& );
    static RetVal fromValue( TokenIterator& );
    static RetVal fromString( TokenIterator& );
};


Detokenizer::RetVal Detokenizer::fromString( TokenIterator& it )
{
    auto token = *it;
    if ( !token ) return std::unexpected( eEndOfStream );
    if ( token.userEnum != '"' ) return std::unexpected( eToken );
    token = *++it;
    if ( !token ) return std::unexpected( eEndOfStream );
    std::pmr::string ret{ *token };
    token = *++it;
    if ( !token ) return std::unexpected( eEndOfStream );
    if ( token.userEnum != '"' ) return std::unexpected( eToken );
    return ret;
}

Detokenizer::ComplexVal Detokenizer::fromObject( TokenIterator& it )
{
    auto token = *it;
    if ( !token ) { return std::unexpected( eEndOfStream ); }

    AST expectedToken = eBegin;
    Entry ret{};

    for ( token = *it; token; token = *(++it) ) {
        if ( WHITESPACES.contains( static_cast<char32_t>( token.userEnum ) ) ) { continue; }
        switch ( expectedToken ) {
        case eBegin:
            if ( token.userEnum != eBegin ) { return std::unexpected( eToken ); }
            expectedToken = eNameEnd;
            continue;

        case eNameEnd:
            switch ( token.userEnum ) {
            case eEnd: return ret;
            case eName: break;
            default: return std::unexpected( eToken );
            } [[fallthrough]];
        case eName:
            if ( auto n = fromString( it ) ) {
                ret.m_data.emplace_back().m_name = std::move( *n );
                expectedToken = eColon;
                continue;
            }
            else {
                return std::unexpected( n.error() );
            }

        case eColon:
            if ( token.userEnum != eColon ) { return std::unexpected( eToken ); }
            expectedToken = eObjectArrayValue;
            continue;

        case eObjectArrayValue:
            expectedToken = eCommaEnd;
            switch ( token.userEnum ) {
            case eBegin:
                if ( auto o = fromObject( it ) ) {
                    ret.m_data.back().m_data = std::move( (*o).m_data );
                    continue;
                }
                else {
                    return std::unexpected( o.error() );
                }
            case eArray:
                if ( auto a = fromArray( it ) ) {
                    ret.m_data.back().m_data = std::move( (*a).m_data );
                    continue;
                }
                else {
                    return std::unexpected( a.error() );
                }
            default:
                if ( auto v = fromValue( it ) ) {
                    ret.m_data.back().m_value = std::move( *v );
                    continue;
                }
                else {
                    return std::unexpected( v.error() );
                }
            }
        case eCommaEnd:
            switch ( token.userEnum ) {
            case eComma:
                expectedToken = eName;
                continue;
            case eEnd:
                return ret;
            default:
                return std::unexpected( eToken );
            }
        default:
            return std::unexpected( eToken );
        }
    }

    return std::unexpected( eEndOfStream );
}

Detokenizer::ComplexVal Detokenizer::fromArray( TokenIterator& it )
{
    auto token = *it;
    if ( !token ) { return std::unexpected( eEndOfStream ); }
    if ( token.userEnum != eArray ) { return std::unexpected( eToken ); }

    [[maybe_unused]]
    AST expectedToken = eObjectArrayArrayEndValue;
    Entry ret;
    for ( token = *(++it); token; token = *(++it) ) {
        if ( WHITESPACES.contains( static_cast<char32_t>( token.userEnum ) ) ) { continue; }
        switch ( expectedToken ) {
        case eObjectArrayArrayEndValue:
            if ( token.userEnum == eArrayEnd ) return ret;
            [[fallthrough]];

        case eObjectArrayValue:
            expectedToken = eCommaArrayEnd;
            switch (token.userEnum) {
            case eBegin:
                if ( auto o = fromObject( it ) ) {
                    ret.m_data.emplace_back().m_data = std::move( (*o).m_data );
                    continue;
                }
                else {
                    return std::unexpected( o.error() );
                }
            case eArray:
                if ( auto a = fromArray( it ) ) {
                    ret.m_data.emplace_back().m_data = std::move( (*a).m_data );
                    continue;
                }
                else {
                    return std::unexpected( a.error() );
                }
            case eName:
                if ( auto n = fromString( it ) ) {
                    ret.m_data.emplace_back().m_value = std::move( *n );
                    continue;
                }
                else {
                    return std::unexpected( n.error() );
                }
            default:
                if ( auto v = fromValue( it ) ) {
                    ret.m_data.emplace_back().m_value = std::move( *v );
                    continue;
                }
                else {
                    return std::unexpected( v.error() );
                }
            }

        case eCommaArrayEnd:
            switch (token.userEnum) {
            case eArrayEnd:
                return ret;
            case eComma:
                expectedToken = eObjectArrayValue;
                continue;
            default:
                return std::unexpected( eToken );
            }
        default:
            return std::unexpected( eToken );
        }
    }
    return std::unexpected( eToken );
}

Detokenizer::RetVal Detokenizer::fromValue( TokenIterator& it )
{
    auto token = *it;
    if ( !token ) return std::unexpected( eEndOfStream );
    if ( token.userEnum == '"' ) return fromString( it );

    std::from_chars_result ret{};

    uint32_t ru{};
    ret = std::from_chars( token.begin(), token.end(), ru, 10 );
    if ( ret && ret.ptr == token.end() ) return std::pmr::string{ *token };

    int32_t ri{};
    ret = std::from_chars( token.begin(), token.end(), ri, 10 );
    if ( ret && ret.ptr == token.end() ) return std::pmr::string{ *token };

    float rf{};
    ret = std::from_chars( token.begin(), token.end(), rf );
    if ( ret && ret.ptr == token.end() ) return std::pmr::string{ *token };

    if ( *token == "true"sv ) return std::pmr::string{ "1"sv };
    if ( *token == "false"sv ) return std::pmr::string{ "0"sv };
    if ( *token == "null"sv ) return std::pmr::string{ "0"sv };
    return std::unexpected( eToken );
}

Entry cfg::Entry::fromJSON( std::span<const char> data )
{
    TokenIterator it{ SEPARATORS, data };
    auto o = Detokenizer::fromObject( it );
    if ( o ) return *o;
    return {};
}

Entry cfg::Entry::fromJSON( std::string_view data )
{
    std::span<const char> span{ data.data(), data.size() };
    return fromJSON( span );
}

