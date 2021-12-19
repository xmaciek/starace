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
        EXPECT_NE( begin, end );
        EXPECT_EQ( end - begin, 3 );
        int f[ 4 ]{};
        auto idx = []( int i ) { switch ( i ) { case 7: return 0; case 9: return 1; case 10: return 2; default: return 3; } };
        f[ idx( *begin++ ) ]++;
        f[ idx( *begin++ ) ]++;
        f[ idx( *begin++ ) ]++;
        EXPECT_EQ( f[ 0 ], 1 );
        EXPECT_EQ( f[ 1 ], 1 );
        EXPECT_EQ( f[ 2 ], 1 );
        EXPECT_EQ( f[ 3 ], 0 );
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
