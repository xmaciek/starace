#include <gtest/gtest.h>

#include <shared/hash.hpp>

#include <string>
#include <string_view>

static constexpr auto TEST = "test"_hash;
static constexpr auto TEXT = "text"_hash;

TEST( Hash, value_equality )
{
    Hash hash{};
    EXPECT_EQ( TEST, "test"_hash );
    EXPECT_EQ( TEST, hash( "test" ) );
    EXPECT_EQ( hash( "test" ), "test"_hash );
    EXPECT_EQ( Hash::v<"test">, TEST );
    EXPECT_EQ( Hash::v<"test">, hash( "test" ) );
    EXPECT_EQ( Hash::v<"test">, "test"_hash );

    volatile Hash::value_type h1 = "test"_hash;
    volatile Hash::value_type h2 = hash( "test" );
    EXPECT_EQ( h1, h2 );
    EXPECT_EQ( h1, Hash::v<"test"> );

    EXPECT_EQ( TEXT, hash( "text" ) );
    EXPECT_NE( TEST, TEXT );

}

TEST( Hash, partial_memory_read )
{
    Hash hash{};
    std::string t = "testx";
    std::string_view sv{ t.begin(), t.begin() + 4 };
    EXPECT_EQ( TEST, hash( sv ) );

    static constexpr auto x = "x"_hash;
    t = "x = 128";
    sv = std::string_view{ t.begin(), t.begin() + 1 };
    EXPECT_EQ( x, hash( sv ) );

}
