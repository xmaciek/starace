#include <gtest/gtest.h>

#include <ccmd/ccmd.hpp>

constexpr std::string_view TEST_CODE =
    "cmd1 2 3\n"
    "cmd4 5 6\n"
    "cmdF 1.2 3.4\n"
    "cmdS aa bb wink wink\n"
    ;

static uint32_t cmd1( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 2 ) return 1;
    int i = 0xFF;
    if ( !ccmd::argv( vm, 0, i ) ) return 2;
    if ( i != 2 ) return 3;
    if ( !ccmd::argv( vm, 1, i ) ) return 4;
    if ( i != 3 ) return 5;
    return 0;
}

static uint32_t cmd4( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 2 ) return 1;
    int i = 0xFF;
    if ( !ccmd::argv( vm, 0, i ) ) return 2;
    if ( i != 5 ) return 3;
    if ( !ccmd::argv( vm, 1, i ) ) return 4;
    if ( i != 6 ) return 5;
    return 0;
}

static uint32_t cmdF( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 2 ) return 1;
    float f{};
    if ( !ccmd::argv( vm, 0, f ) ) return 2;
    if ( f != 1.2f ) return 3;
    if ( !ccmd::argv( vm, 1, f ) ) return 4;
    if ( f != 3.4f ) return 5;
    return 0;
}

static uint32_t cmdS( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 4 ) return 1;
    std::pmr::string s{};
    if ( !ccmd::argv( vm, 0, s ) ) return 2;
    if ( s != "aa" ) return 3;
    if ( !ccmd::argv( vm, 1, s ) ) return 4;
    if ( s != "bb" ) return 5;
    if ( !ccmd::argv( vm, 2, s ) ) return 6;
    if ( s != "wink" ) return 7;
    if ( !ccmd::argv( vm, 3, s ) ) return 8;
    if ( s != "wink" ) return 9;
    return 0;
}

static uint32_t fail( ccmd::Vm* vm, void* )
{
    ccmd::setError( vm, "custom error message" );
    return 42;
}

static uint32_t unresolved( ccmd::Vm*, void* )
{
    return 0;
}

static uint32_t testArgc( ccmd::Vm* vm, void* )
{
    return ccmd::argc( vm ) == 2 ? 0 : 1;
}

static uint32_t testArgvOutOfRange( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 2 ) return 1;
    int i = 77;
    return ccmd::argv( vm, 12, i ) ? 0 : 2;
}

static uint32_t testArgvConvert( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 2 ) return 1;
    std::pmr::string s;
    if ( !ccmd::argv( vm, 0, s ) ) return 2;
    if ( s != "2" ) return 3;
    return 0;
}

static uint32_t testArgvFailConvert( ccmd::Vm* vm, void* )
{
    if ( ccmd::argc( vm ) != 4 ) return 1;
    int i = 77;
    return ccmd::argv( vm, 0, i ) ? 0 : 2;
}

TEST( ccmd, compile )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    EXPECT_FALSE( bytecode.empty() );
}

TEST( ccmd, run )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    std::array commands = {
        ccmd::Command{ "cmd1", &cmd1 },
        ccmd::Command{ "cmd4", &cmd4 },
        ccmd::Command{ "cmdF", &cmdF },
        ccmd::Command{ "cmdS", &cmdS },
    };
    auto ret = ccmd::run( commands, bytecode );
    EXPECT_EQ( ret, ccmd::eSuccess );
}

TEST( ccmd, missingCommands )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    std::array commands = {
        ccmd::Command{ "cmd1", &cmd1 },
        ccmd::Command{ "cmd4", &cmd4 },
        ccmd::Command{ "cmdF", &cmdF },
    };
    auto ret = ccmd::run( commands, bytecode );
    EXPECT_EQ( ret, ccmd::eUnresolvedCommand );
}

TEST( ccmd, failCommand )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    ccmd::RunContext rctx{};
    std::array commands = {
        ccmd::Command{ "cmd1", &fail },
        ccmd::Command{ "cmd4", &cmd4 },
        ccmd::Command{ "cmdF", &cmdF },
        ccmd::Command{ "cmdS", &cmdS },
    };
    auto ret = ccmd::run( commands, bytecode, &rctx );
    EXPECT_EQ( ret, ccmd::eFunctionFail );
    EXPECT_EQ( rctx.commandExitCode, 42 );
    EXPECT_EQ( rctx.commandName, "cmd1" );
    EXPECT_EQ( rctx.commandErrorMessage, "custom error message" );
}

TEST( ccmd, unresolvedCommand )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    ccmd::RunContext rctx{};
    std::array commands = {
        ccmd::Command{ "cmd1", &cmd1 },
        ccmd::Command{ &unresolved },
    };
    auto ret = ccmd::run( commands, bytecode );
    EXPECT_EQ( ret, ccmd::eSuccess );
}

TEST( ccmd, argc )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    std::array commands = {
        ccmd::Command{ "cmd1", &testArgc },
        ccmd::Command{ &unresolved },
    };
    auto ret = ccmd::run( commands, bytecode );
    EXPECT_EQ( ret, ccmd::eSuccess );
}

TEST( ccmd, argvOutOfRange )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    ccmd::RunContext rctx{};
    std::array commands = {
        ccmd::Command{ "cmd1", &testArgvOutOfRange },
        ccmd::Command{ &unresolved },
    };
    auto ret = ccmd::run( commands, bytecode, &rctx );
    EXPECT_EQ( ret, ccmd::eFunctionFail );
    EXPECT_EQ( rctx.commandExitCode, 2 );
}

TEST( ccmd, argvConvert )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    std::array commands = {
        ccmd::Command{ "cmd1", &testArgvConvert },
        ccmd::Command{ &unresolved },
    };
    auto ret = ccmd::run( commands, bytecode );
    EXPECT_EQ( ret, ccmd::eSuccess );
}

TEST( ccmd, argvFailConvert )
{
    auto bytecode = ccmd::compile( TEST_CODE );
    ASSERT_FALSE( bytecode.empty() );

    ccmd::RunContext rctx{};
    std::array commands = {
        ccmd::Command{ "cmdS", &testArgvFailConvert },
        ccmd::Command{ &unresolved },
    };
    auto ret = ccmd::run( commands, bytecode, &rctx );
    EXPECT_EQ( ret, ccmd::eFunctionFail );
    EXPECT_EQ( rctx.commandExitCode, 2 );
}
