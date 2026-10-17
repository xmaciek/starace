#include <gtest/gtest.h>

#include <config/config.hpp>

using std::literals::string_view_literals::operator""sv;

TEST( ConfigJSON, simple )
{
    auto sv = "{}"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );

    EXPECT_EQ( entry.name(), ""sv );
    EXPECT_EQ( entry.toString(), ""sv );
}

TEST( ConfigJSON, oneval )
{
    auto sv = R"({ "a" : "b" })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
}

TEST( ConfigJSON, oneval2 )
{
    auto sv = R"({ "a" : -2 })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "-2"sv );
}

TEST( ConfigJSON, threeval )
{
    auto sv = R"({ "a" : "b", "c" : "d", "e" : "f" })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "b"sv );
    EXPECT_EQ( entry[ "c"sv ].name(), "c"sv );
    EXPECT_EQ( entry[ "c"sv ].toString(), "d"sv );
    EXPECT_EQ( entry[ "e"sv ].name(), "e"sv );
    EXPECT_EQ( entry[ "e"sv ].toString(), "f"sv );
}

TEST( ConfigJSON, nestedObject )
{
    auto sv = R"({"a":{"b":"c"}})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].name(), "b"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );
}

TEST( ConfigJSON, nestedObject2 )
{
    auto sv = R"(
    {
        "a":{
            "b":"c"
        },
        "d":{
            "e":{
                "f" : "g"
            }
        }
    })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].name(), "b"sv );
    EXPECT_EQ( entry[ "a"sv ][ "b"sv ].toString(), "c"sv );

    EXPECT_EQ( entry[ "d"sv ].name(), "d"sv );
    EXPECT_EQ( entry[ "d"sv ][ "e"sv ].name(), "e"sv );
    EXPECT_EQ( entry[ "d"sv ][ "e"sv ][ "f"sv ].name(), "f"sv );
    EXPECT_EQ( entry[ "d"sv ][ "e"sv ][ "f"sv ].toString(), "g"sv );
}

TEST( ConfigJSON, array )
{
    auto sv = R"({"a":[]})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].size(), 0 );
}

TEST( ConfigJSON, array2 )
{
    auto sv = R"({"a":[{ "b" : "c" }, 4]})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    ASSERT_EQ( entry[ "a"sv ].size(), 2 );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 0 ][ "b"sv ].name(), "b"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 0 ][ "b"sv ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 1 ].toInt<int>(), 4 );
}

TEST( ConfigJSON, arrayofArray )
{
    auto sv = R"({"a":[[]]})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    ASSERT_EQ( entry[ "a"sv ].size(), 1 );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 0 ].size(), 0 );
}

TEST( ConfigJSON, arrayEntry )
{
    auto sv = R"({"a":[ "b", "c", "d" ]})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    ASSERT_EQ( entry[ "a"sv ].size(), 3 );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 0 ].toString(), "b"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 1 ].toString(), "c"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 2 ].toString(), "d"sv );
}

TEST( ConfigJSON, arrayNumber )
{
    auto sv = R"({"a":[ 1, 2, 3, 4 ]})"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    ASSERT_EQ( entry[ "a"sv ].size(), 4 );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 0 ].toString(), "1"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 1 ].toString(), "2"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 2 ].toString(), "3"sv );
    EXPECT_EQ( entry[ "a"sv ].toArray()[ 3 ].toString(), "4"sv );
}

TEST( ConfigJSON, bools )
{
    auto sv = R"({"a": true, "b" : false })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_TRUE( entry[ "a"sv ].toInt<bool>() );
    EXPECT_EQ( entry[ "b"sv ].name(), "b"sv );
    EXPECT_FALSE( entry[ "b"sv ].toInt<bool>() );
}

TEST( ConfigJSON, parseError )
{
    EXPECT_FALSE( cfg::Entry::fromJSON( R"(a)"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({)"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"(})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"(")"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"("a")"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"("a",)"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"("a":)"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":a})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":0.0.0})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":-0.0.0})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":][})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":[a]})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":"b" "c":"d"})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":"b", "c":"d",})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":[1,2,3,]})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":True})"sv ) );
    EXPECT_FALSE( cfg::Entry::fromJSON( R"({"a":False})"sv ) );
}
