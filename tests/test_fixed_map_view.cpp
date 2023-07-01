#include <gtest/gtest.h>

#include <shared/fixed_map.hpp>

TEST( FixedMapView, find )
{
    int keys[]   = { 0, 1, 2, 3, 4 };
    int values[] = { 5, 6, 7, 8, 9 };
    FixedMapView<int, int> fmv( keys, values );

    int* found38 = fmv.find( 3 );
    ASSERT_NE( found38, nullptr );
    EXPECT_EQ( *found38, 8 );
}

TEST( FixedMapView, notFound )
{
    int keys[]   = { 0, 1, 2, 3, 4 };
    int values[] = { 5, 6, 7, 8, 9 };
    FixedMapView<int, int> fmv( keys, values );

    int* notFound = fmv.find( 5 );
    EXPECT_EQ( notFound, nullptr );
}

TEST( FixedMapView, equalRange )
{
    int keys[]   = { 0, 1, 2, 2, 3, 4 };
    int values[] = { 5, 6, 7, 8, 9, 9 };
    FixedMapView<int, int> fmv( keys, values );

    auto [ begin, end ] = fmv.equalRange( 2 );
    ASSERT_NE( begin, nullptr );
    ASSERT_NE( end, nullptr );
    ASSERT_NE( begin, end );
    ASSERT_EQ( end - begin, 2 );
    EXPECT_EQ( begin[ 0 ], 7 );
    EXPECT_EQ( begin[ 1 ], 8 );

}

TEST( FixedMapView, equalRangeNotFound )
{
    int keys[]   = { 0, 1, 2, 2, 3, 4 };
    int values[] = { 5, 6, 7, 8, 9, 9 };
    FixedMapView<int, int> fmv( keys, values );

    auto [ begin, end ] = fmv.equalRange( 5 );

    EXPECT_EQ( begin, nullptr );
    EXPECT_EQ( end, nullptr );;
}
