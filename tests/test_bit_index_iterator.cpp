#include <gtest/gtest.h>

#include <shared/bit_index_iterator.hpp>

TEST( BitIndexIterator, bool_operator )
{
    BitIndexIterator bii{};
    EXPECT_EQ( bii, false );

    bii = 1;
    EXPECT_EQ( bii, true );
}

TEST( BitIndexIterator, deref_operator )
{
    BitIndexIterator bii = 0b1;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 0 );

    bii = 0b10;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 1 );

    bii = 0b100;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 2 );

    bii = 0b1000;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 3 );
}

TEST( BitIndexIterator, increment_operator )
{
    BitIndexIterator bii = 0b1010101;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 0 );

    bii++;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 2 );

    bii++;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 4 );

    bii++;
    ASSERT_EQ( bii, true );
    EXPECT_EQ( *bii, 6 );

    bii++;
    EXPECT_EQ( bii, false );
}


TEST( BitIndexIterator, count )
{
    BitIndexIterator bii = 0b1010101;
    EXPECT_EQ( bii.count(), 4 );
}
