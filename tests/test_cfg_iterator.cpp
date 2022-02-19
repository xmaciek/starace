#include <gtest/gtest.h>

#include <shared/cfg.hpp>

#include <string_view>


TEST( CfgIterator, find )
{
    const std::array text = { '=' };
    const std::span<const char> data{ text };
    const std::span<const char> separators = cfg::c_separators;

    cfg::TokenIterator it{ separators, data };
    cfg::Token token = *it;
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, '=' );
}

TEST( CfgIterator, find2 )
{
    const std::string_view text = "x = 1";
    const std::span<const char> data{ text };
    const std::span<const char> separators = cfg::c_separators;

    cfg::TokenIterator it{ separators, data };
    cfg::Token token = *it;
    ASSERT_TRUE( static_cast<bool>( token ) );
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, cfg::TokenIterator::c_unknown );
    EXPECT_EQ( *token.data, 'x' );

    token = *++it;
    ASSERT_TRUE( static_cast<bool>( token ) );
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, ' ' );

    token = *++it;
    ASSERT_TRUE( static_cast<bool>( token ) );
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, '=' );

    token = *++it;
    ASSERT_TRUE( static_cast<bool>( token ) );
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, ' ' );

    token = *++it;
    ASSERT_TRUE( static_cast<bool>( token ) );
    EXPECT_EQ( token.length, 1 );
    EXPECT_EQ( token.userEnum, cfg::TokenIterator::c_unknown );
    EXPECT_EQ( *token.data, '1' );

    // past the end of data stream
    token = *++it;
    ASSERT_FALSE( static_cast<bool>( token ) );

    token = *++it;
    ASSERT_FALSE( static_cast<bool>( token ) );

}
