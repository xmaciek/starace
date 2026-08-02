#include <gtest/gtest.h>

#include <shared/unique_add.hpp>

TEST( unique_add, test )
{
    std::vector<int> vec{ 0, 4, 5, 2, 1, 4 };
    auto it = unique_add( vec, 5 );
    ASSERT_NE( it, vec.end() );
    EXPECT_EQ( vec.size(), 6 );
    EXPECT_EQ( *it, 5 );

    it = unique_add( vec, 7 );
    ASSERT_NE( it, vec.end() );
    EXPECT_EQ( vec.size(), 7 );
    EXPECT_EQ( *it, 7 );
}
