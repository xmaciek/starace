#include <gtest/gtest.h>

#include <shared/max_score_element.hpp>

TEST( Shared, maxScoreElement )
{
    std::array data{ 0u, 4u, 123u, 45u, 423u, 54u, 123u, 12u, 45u, 77u, 4u, 23u, 0u, 45342u, 3u, 8u };
    auto it = maxScoreElement( data, []( auto r ) -> size_t { return r; } );

    EXPECT_EQ( *it, 45342u );
}
