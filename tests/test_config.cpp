#include <gtest/gtest.h>

#include <config/config.hpp>

using std::literals::string_view_literals::operator""sv;

[[maybe_unused]]
std::ostream& operator << ( std::ostream& cout, const cfg::Entry& e )
{
    static int level = 0;
    cout << level;
    for ( int i = 0; i < level; ++i ) cout << "\t";
    level++;
    cout << " " << e.name << " = " << e.value << "\n";
    for ( const auto& it : e.data ) cout << it;
    level--;
    return cout;
}

TEST( Config, simple )
{
    std::string_view sv = "a = b"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( *entry, ""sv );
    EXPECT_EQ( *(entry[ "a"sv ]), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
}

TEST( Config, nested1 )
{
    std::string_view sv = "a = { b = c }"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( *(entry[ "a"sv ][ "b"sv ]), "b"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
}

TEST( Config, nested2 )
{
    std::string_view sv = "a = { b = { c = d } }"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ][ "c"sv ].toString(), "d"sv );
}

TEST( Config, multi )
{
    std::string_view sv = "a = b c = d"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
    EXPECT_EQ( entry[ "c"sv ].toString(), "d"sv );
}

TEST( Config, multiNested )
{
    std::string_view sv = "a = { b = c d = e } }"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a"sv ][ "d"sv ].toString(), "e"sv );
}

TEST( Config, complex )
{
    std::string_view sv = "a = { b = c d = e f = { g = h } i = { j = { k = l } } } m = { n = o }"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( entry[ "a" ][ "b" ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a" ][ "d" ].toString(), "e"sv );
    EXPECT_EQ( entry[ "a" ][ "f" ][ "g" ].toString(), "h"sv );
    EXPECT_EQ( entry[ "a" ][ "i" ][ "j" ][ "k" ].toString(), "l"sv );
    EXPECT_EQ( entry[ "m" ][ "n" ].toString(), "o"sv );
}

TEST( Config, numbers )
{
    std::string_view sv = "a = 1024 b = 1.024"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    EXPECT_EQ( entry[ "a" ].toInt(), 1024 );
    EXPECT_EQ( entry[ "b" ].toFloat(), 1.024f );
}

TEST( Config, iterator )
{
    std::string_view sv = "a = b c = d"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    const auto* it = entry.begin();
    EXPECT_EQ( **it, "a"sv );
    EXPECT_EQ( it->toString(), "b"sv );
    it++;
    EXPECT_EQ( **it, "c"sv );
    EXPECT_EQ( it->toString(), "d"sv );
}

TEST( Config, quotes )
{
    std::string_view sv = "a = \"q u o t e s\""sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    const auto* it = entry.begin();
    EXPECT_EQ( **it, "a"sv );
    EXPECT_EQ( it->toString(), "q u o t e s"sv );
}

TEST( Config, quotes2 )
{
    std::string_view sv = "a = \"in q u o t e s\" b = \"c\" d = \"\" e = f"sv;
    cfg::Entry entry = cfg::Entry::fromData( { sv.data(), sv.data() + sv.size() } );

    const auto* it = entry.begin();
    EXPECT_EQ( **it, "a"sv );
    EXPECT_EQ( it->toString(), "in q u o t e s"sv );

    it++;
    EXPECT_EQ( **it, "b"sv );
    EXPECT_EQ( it->toString(), "c"sv );

    it++;
    EXPECT_EQ( **it, "d"sv );
    EXPECT_EQ( it->toString(), ""sv );

    it++;
    EXPECT_EQ( **it, "e"sv );
    EXPECT_EQ( it->toString(), "f"sv );
}

