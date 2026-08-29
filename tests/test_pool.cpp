#include <gtest/gtest.h>

#include <shared/pool.hpp>

TEST( Pool, pool )
{
    Pool<int, 32> pool;

    EXPECT_EQ( pool.size(), 0 );

    auto idx = pool.emplace( 42 );
    EXPECT_EQ( idx, 0 );
    EXPECT_EQ( pool[ idx ], 42 );
    EXPECT_EQ( pool.size(), 1 );

    auto idx2 = pool.emplace( 76 );
    EXPECT_EQ( idx2, 1 );
    EXPECT_EQ( pool[ idx2 ], 76 );
    EXPECT_EQ( pool.size(), 2 );

    pool.release( idx );
    EXPECT_EQ( pool.size(), 1 );
    auto idx3 = pool.emplace( 88 );
    EXPECT_EQ( pool.size(), 2 );
    EXPECT_EQ( idx3, 0 );
    EXPECT_EQ( pool[ idx3 ], 88 );

}
