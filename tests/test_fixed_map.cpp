#include <gtest/gtest.h>

#include <shared/fixed_map.hpp>

TEST( FixedMap, pushback )
{
    FixedMap<int,int,10> fm{};
    EXPECT_EQ( fm.size(), 0 );
    fm.pushBack( 0, 0 );
    EXPECT_EQ( fm.size(), 1 );
}

TEST( FixedMap, insert )
{
    FixedMap<int,int,10> fm{};
    EXPECT_EQ( fm.size(), 0 );
    fm.insert( 0, 0 );
    fm.insert( 2, 0 );
    fm.insert( 1, 0 );
    EXPECT_EQ( fm.size(), 3 );
}

TEST( FixedMap, find )
{
    FixedMap<int,int,10> fm{};
    EXPECT_EQ( fm.size(), 0 );
    fm.insert( 0, 8 );
    fm.insert( 2, 6 );
    fm.insert( 1, 7 );
    EXPECT_EQ( fm.size(), 3 );
    {
        int* f = fm[ 2 ];
        ASSERT_NE( f, nullptr );
        EXPECT_EQ( *f, 6 );
    }
    {
        int* f = fm[ 1 ];
        ASSERT_NE( f, nullptr );
        EXPECT_EQ( *f, 7 );
    }

    fm.pushBack( 4, 3 );
    {
        int* f = fm[ 4 ];
        ASSERT_NE( f, nullptr );
        EXPECT_EQ( *f, 3 );
    }
    fm.insert( 3, 4 );
    {
        int* f = fm[ 3 ];
        ASSERT_NE( f, nullptr );
        EXPECT_EQ( *f, 4 );
    }
}

TEST( FixedMap, notfind )
{
    FixedMap<int,int,10> fm{};
    EXPECT_EQ( fm.size(), 0 );
    fm.insert( 0, 8 );
    fm.insert( 2, 6 );
    fm.insert( 1, 7 );
    EXPECT_EQ( fm.size(), 3 );
    int* f = fm[ 3 ];
    EXPECT_EQ( f, nullptr );
}

TEST( FixedMap, equalRange )
{
    FixedMap<int,int,10> fm{};
    EXPECT_EQ( fm.size(), 0 );
    fm.insert( 0, 8 );
    fm.insert( 2, 6 );
    fm.insert( 1, 7 );
    fm.insert( 1, 9 );
    fm.insert( 1, 10 );
    EXPECT_EQ( fm.size(), 5 );

    {
        auto [ begin, end ] = fm.equalRange( 1 );
        ASSERT_NE( begin, nullptr );
        ASSERT_NE( end, nullptr );
        EXPECT_EQ( end - begin, 3 );
        EXPECT_EQ( begin[ 0 ], 10 );
        EXPECT_EQ( begin[ 1 ], 9 );
        EXPECT_EQ( begin[ 2 ], 7 );
    }
    {
        auto [ begin, end ] = fm.equalRange( 2 );
        ASSERT_NE( begin, nullptr );
        ASSERT_NE( end, nullptr );
        EXPECT_NE( begin, end );
        EXPECT_EQ( end - begin, 1 );
        EXPECT_EQ( *begin, 6 );
    }
    {
        auto [ begin, end ] = fm.equalRange( 3 );
        ASSERT_EQ( begin, nullptr );
        ASSERT_EQ( end, nullptr );
    }
}

TEST( FixedMap, copy )
{
    FixedMap<int, int, 2> fm{};
    fm.insert( 1, 1 );
    fm.insert( 2, 2 );

    FixedMap<int, int, 2> copied = fm;
    int* a1 = fm[ 1 ];
    int* b1 = fm[ 2 ];
    int* a2 = copied[ 1 ];
    int* b2 = copied[ 2 ];
    ASSERT_NE( a1, nullptr );
    ASSERT_NE( a2, nullptr );
    ASSERT_NE( b1, nullptr );
    ASSERT_NE( b2, nullptr );
    EXPECT_EQ( *a1, 1 );
    EXPECT_EQ( *a1, *a2 );
    EXPECT_EQ( *b1, 2 );
    EXPECT_EQ( *b1, *b2 );
}
