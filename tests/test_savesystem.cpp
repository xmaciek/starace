#include <gtest/gtest.h>

#include <engine/savesystem.hpp>

#include <algorithm>
#include <array>

static const std::pmr::vector<uint8_t> TEST_DATA{ 0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE };

TEST( SaveSystem, save )
{
    SaveSystem ss{ "starace_test" };
    EXPECT_EQ( ss.save( 0, TEST_DATA ), SaveSystem::eSuccess );
    std::pmr::vector<uint8_t> data{};
    EXPECT_EQ( ss.load( 0, data ), SaveSystem::ErrorCode::eSuccess );
    EXPECT_EQ( data, TEST_DATA );
}

TEST( SaveSystem, load )
{
    SaveSystem ss{ "starace_test" };
    ASSERT_EQ( ss.save( 0, TEST_DATA ), SaveSystem::eSuccess );
    std::pmr::vector<uint8_t> data{};
    EXPECT_EQ( ss.load( 0, data ), SaveSystem::ErrorCode::eSuccess );
    EXPECT_EQ( data, TEST_DATA );
}

TEST( SaveSystem, list )
{
    uint32_t saveCount = 0;
    uint32_t save0once = false;
    SaveSystem ss{ "starace_test" };
    ss.list( [&saveCount, &save0once]( SaveSystem::Slot s )
    {
        saveCount++;
        save0once += ( s == 0 ) ? 1 : 0;
    } );
    EXPECT_EQ( saveCount, 1 );
    EXPECT_EQ( save0once, 1 );
}

TEST( SaveSystem, save_fail )
{
    SaveSystem ss{ "starace_test" };
    EXPECT_EQ( ss.save( SaveSystem::MAX_SAVES, TEST_DATA ), SaveSystem::eSlotIndexOutOfRange );
}

TEST( SaveSystem, load_fail )
{
    SaveSystem ss{ "starace_test" };
    std::pmr::vector<uint8_t> data{};
    EXPECT_EQ( ss.load( SaveSystem::MAX_SAVES, data ), SaveSystem::eSlotIndexOutOfRange );
    EXPECT_TRUE( data.empty() );
}
