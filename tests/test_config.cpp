#include <gtest/gtest.h>

#include <config/config.hpp>

using std::literals::string_view_literals::operator""sv;

std::span<const char> operator ""_span ( const char* str, unsigned long len ) noexcept
{
    return { str, str + len };
}

TEST( Config, simple )
{
    auto sv = "a = b"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry.name(), ""sv );
    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
}

TEST( Config, nested1 )
{
    auto sv = "a = { b = c }"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].name(), "b"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
}

TEST( Config, nested2 )
{
    auto sv = "a = { b = { c = d } }"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ][ "c"sv ].toString(), "d"sv );
}

TEST( Config, multi )
{
    auto sv = "a = b c = d"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
    EXPECT_EQ( entry[ "c"sv ].toString(), "d"sv );
}

TEST( Config, multiNested )
{
    auto sv = "a = { b = c d = e } }"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a"sv ][ "d"sv ].toString(), "e"sv );
}

TEST( Config, complex )
{
    auto sv = "a = { b = c d = e f = { g = h } i = { j = { k = l } } } m = { n = o }"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a"sv ][ "d"sv ].toString(), "e"sv );
    EXPECT_EQ( entry[ "a"sv ][ "f"sv ][ "g"sv ].toString(), "h"sv );
    EXPECT_EQ( entry[ "a"sv ][ "i"sv ][ "j"sv ][ "k"sv ].toString(), "l"sv );
    EXPECT_EQ( entry[ "m"sv ][ "n"sv ].toString(), "o"sv );
}

TEST( Config, numbers )
{
    auto sv = "a = 1024 b = 1.024"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a"sv ].toInt(), 1024 );
    EXPECT_EQ( entry[ "b"sv ].toFloat(), 1.024f );
}

TEST( Config, iterator )
{
    auto sv = "a = b c = d"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    const auto* it = entry.begin();
    EXPECT_EQ( it->name(), "a"sv );
    EXPECT_EQ( it->toString(), "b"sv );
    it++;
    EXPECT_EQ( it->name(), "c"sv );
    EXPECT_EQ( it->toString(), "d"sv );
}

TEST( Config, broken )
{
    auto sv = "{ a = { b = c } }"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );
    EXPECT_TRUE( entry.name().empty() );
}

TEST( Config, quotes )
{
    auto sv = "a = \"q u o t e s\""_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    const auto* it = entry.begin();
    EXPECT_EQ( it->name(), "a"sv );
    EXPECT_EQ( it->toString(), "q u o t e s"sv );
}

TEST( Config, quotes2 )
{
    auto sv = "a = \"in q u o t e s\" b = \"c\" d = \"\" e = f"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    const auto* it = entry.begin();
    EXPECT_EQ( it->name(), "a"sv );
    EXPECT_EQ( it->toString(), "in q u o t e s"sv );

    it++;
    EXPECT_EQ( it->name(), "b"sv );
    EXPECT_EQ( it->toString(), "c"sv );

    it++;
    EXPECT_EQ( it->name(), "d"sv );
    EXPECT_EQ( it->toString(), ""sv );

    it++;
    EXPECT_EQ( it->name(), "e"sv );
    EXPECT_EQ( it->toString(), "f"sv );
}

TEST( Config, toValue )
{
    {
        cfg::Entry entry{};

        int32_t v{};
        EXPECT_FALSE( entry.toValue( v ) );
        EXPECT_EQ( v, 0 );

        entry.m_value = "-42";
        EXPECT_TRUE( entry.toValue( v ) );
        EXPECT_EQ( v, -42 );

        entry.m_value = "2.0";
        EXPECT_TRUE( entry.toValue( v ) );
        EXPECT_EQ( v, 2 );
    }

    {
        cfg::Entry entry{};
        uint32_t u{};
        entry.m_value = "99";
        EXPECT_TRUE( entry.toValue( u ) );
        EXPECT_EQ( u, 99 );

        entry.m_value = "88.4";
        EXPECT_TRUE( entry.toValue( u ) );
        EXPECT_EQ( u, 88 );
    }

    {
        cfg::Entry entry{};
        float f{};
        entry.m_value = "12";
        EXPECT_TRUE( entry.toValue( f ) );
        EXPECT_EQ( f, 12.0f );
    }

    {
        enum E : uint32_t {
            eX = 4,
            eY = 8,
        };
        E e = eX;
        cfg::Entry entry{};
        entry.m_value = "8";
        EXPECT_TRUE( entry.toValue( e ) );
        EXPECT_EQ( e, eY );
    }
}
