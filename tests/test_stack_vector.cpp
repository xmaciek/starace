#include <gtest/gtest.h>

#include <shared/stack_vector.hpp>

TEST( StackVector, test )
{
    StackVector<int, 10> data;
    EXPECT_EQ( data.size(), 0 );

    data.push_back( 1 );
    data.push_back( 7 );
    data.push_back( 4 );
    EXPECT_EQ( data.size(), 3 );
    EXPECT_EQ( data.front(), 1 );
    EXPECT_EQ( data.back(), 4 );

    StackVector<int, 10> rhs;
    EXPECT_EQ( rhs.size(), 0 );

    std::swap( data, rhs );
    EXPECT_EQ( data.size(), 0 );
    EXPECT_EQ( rhs.size(), 3 );
    EXPECT_EQ( rhs.front(), 1 );
    EXPECT_EQ( rhs.back(), 4 );

}
