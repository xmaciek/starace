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

    EXPECT_EQ( entry[ "a" ][ "b" ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a" ][ "d" ].toString(), "e"sv );
    EXPECT_EQ( entry[ "a" ][ "f" ][ "g" ].toString(), "h"sv );
    EXPECT_EQ( entry[ "a" ][ "i" ][ "j" ][ "k" ].toString(), "l"sv );
    EXPECT_EQ( entry[ "m" ][ "n" ].toString(), "o"sv );
}

TEST( Config, numbers )
{
    auto sv = "a = 1024 b = 1.024"_span;
    cfg::Entry entry = cfg::Entry::fromData( sv );

    EXPECT_EQ( entry[ "a" ].toInt(), 1024 );
    EXPECT_EQ( entry[ "b" ].toFloat(), 1.024f );
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

