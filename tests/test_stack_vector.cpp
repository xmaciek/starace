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

struct TestStruct {
    int* x{};
    ~TestStruct() { if ( x ) --(*x); }
    TestStruct() = default;
    TestStruct( int* a ) : x{ a } { if ( x ) (*x)++; }
    TestStruct( const TestStruct& r ) : x{ r.x } { if ( x ) (*x)++; }
    TestStruct& operator = ( const TestStruct& r ) { if ( x ) --(*x); x = r.x; if ( x ) (*x)++; return *this; }
    TestStruct( TestStruct&& r ) : x{ std::exchange( r.x, {} ) } {}
    TestStruct& operator = ( TestStruct&& r ) { if ( x ) --(*x); x = std::exchange( r.x, {} ); return *this; }
};

TEST( StackVector, move )
{
    StackVector<TestStruct, 2> data1;
    StackVector<TestStruct, 2> data2;

    int counter = 0;
    data1.emplace_back( &counter );
    data1.emplace_back( &counter );
    ASSERT_EQ( data1.size(), 2 );

    data2 = std::move( data1 );
    ASSERT_EQ( data1.size(), 0 );
    ASSERT_EQ( data2.size(), 2 );

    EXPECT_EQ( counter, 2 );
    EXPECT_EQ( *data2[ 0 ].x, 2 );
    EXPECT_EQ( *data2[ 1 ].x, 2 );

    data2 = std::move( data1 );
    EXPECT_EQ( data1.size(), 0 );
    EXPECT_EQ( data2.size(), 0 );
    EXPECT_EQ( counter, 0 );

}

TEST( StackVector, copy )
{
    StackVector<TestStruct, 2> data1;
    StackVector<TestStruct, 2> data2;

    int counter = 0;
    data1.emplace_back( &counter );
    data1.emplace_back( &counter );
    ASSERT_EQ( data1.size(), 2 );

    data2 = data1;
    ASSERT_EQ( data1.size(), 2 );
    ASSERT_EQ( data2.size(), 2 );

    EXPECT_EQ( counter, 4 );
    EXPECT_EQ( *data1[ 0 ].x, 4 );
    EXPECT_EQ( *data1[ 1 ].x, 4 );
    EXPECT_EQ( *data2[ 0 ].x, 4 );
    EXPECT_EQ( *data2[ 1 ].x, 4 );
}

TEST( StackVector, clear )
{
    StackVector<TestStruct, 2> data;

    int counter = 0;
    data.emplace_back( &counter );
    data.emplace_back( &counter );
    ASSERT_EQ( data.size(), 2 );

    EXPECT_EQ( counter, 2 );
    data.clear();
    EXPECT_EQ( data.size(), 0 );
    EXPECT_EQ( counter, 0 );
}
