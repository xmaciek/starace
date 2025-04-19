#include <gtest/gtest.h>

#include <shared/hash.hpp>

#include <string>
#include <string_view>

static constexpr auto TEST = "test"_hash;

TEST( Hash, value_equality )
{
    Hash hash{};
    EXPECT_EQ( TEST, "test"_hash );
    EXPECT_EQ( TEST, hash( "test" ) );
    EXPECT_EQ( hash( "test" ), "test"_hash );
    volatile Hash::value_type h1 = "test"_hash;
    volatile Hash::value_type h2 = hash( "test" );
    EXPECT_EQ( h1, h2 );

}

TEST( Hash, weird_msvc_switch_suffix_operator_bug__maybe )
{
    Hash hash{};
    std::string txt = "testx";
    auto volatile h = hash( txt );
    switch ( h ) {
    case "testx"_hash:
        EXPECT_EQ( h, "testx"_hash );
        break;
    default:
        EXPECT_EQ( "bug", "exists" );
        break;
    }
}
