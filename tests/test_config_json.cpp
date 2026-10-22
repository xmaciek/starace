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

TEST( ConfigJSON, longstring )
{
    auto sv = R"({ "a" : "bcd efg hij" })"sv;
    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );

    EXPECT_EQ( entry[ "a"sv ].name(), "a"sv );
    EXPECT_EQ( entry[ "a"sv ].toString(), "bcd efg hij"sv );
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

TEST( ConfigJSON, complex )
{
    auto sv = R"({"asset":{"generator":"Khronos glTF Blender I/O v5.1.20","version":"2.0"},"scene":0,"scenes":[{"name":"Scene","nodes":[0]}],"nodes":[{"mesh":0,"name":"projectile","rotation":[0.7071068286895752,0,0,0.7071068286895752]}],"meshes":[{"name":"projectile","primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2},"indices":3}]}],"accessors":[{"bufferView":0,"componentType":5126,"count":8,"max":[0.5,13,0.5],"min":[-0.5,-13,-0.5],"type":"VEC3"},{"bufferView":1,"componentType":5126,"count":8,"type":"VEC3"},{"bufferView":2,"componentType":5126,"count":8,"type":"VEC2"},{"bufferView":3,"componentType":5123,"count":12,"type":"SCALAR"}],"bufferViews":[{"buffer":0,"byteLength":96,"byteOffset":0,"target":34962},{"buffer":0,"byteLength":96,"byteOffset":96,"target":34962},{"buffer":0,"byteLength":64,"byteOffset":192,"target":34962},{"buffer":0,"byteLength":24,"byteOffset":256,"target":34963}],"buffers":[{"byteLength":280}]}  )"sv;

    cfg::Entry entry = cfg::Entry::fromJSON( sv );
    ASSERT_TRUE( entry );
    auto&& asset = entry[ "asset"sv ];
    EXPECT_EQ( asset.name(), "asset"sv );
    EXPECT_EQ( asset[ "generator"sv ].toString(), "Khronos glTF Blender I/O v5.1.20"sv );
    EXPECT_EQ( asset[ "version"sv ].toString(), "2.0"sv );

    auto&& bufferViews = entry[ "bufferViews"sv ];
    ASSERT_TRUE( bufferViews );

    auto&& arr = bufferViews.toArray();
    ASSERT_TRUE( arr.size() > 0 );
    EXPECT_EQ( arr[ 0 ][ "buffer"sv ].toInt<>(), 0 );
    EXPECT_EQ( arr[ 0 ][ "byteLength"sv ].toInt<>(), 96 );
    EXPECT_EQ( arr[ 0 ][ "byteOffset"sv ].toInt<>(), 0 );
    EXPECT_EQ( arr[ 0 ][ "target"sv ].toInt<>(), 34962 );
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
