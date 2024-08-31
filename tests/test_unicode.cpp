#include <gtest/gtest.h>

#include <extra/unicode.hpp>

#include <string>
#include <string_view>

TEST( Unicode, transcoder_length )
{
    EXPECT_EQ( unicode::Transcoder{ "test\u016B" }.length(), 5 );
}

TEST( Unicode, transcoder_convert_loop )
{
    unicode::Transcoder transcoder{ "abcd\u016B" };
    std::u32string test;
    test.resize( 5 );
    for ( auto&& it : test ) it = *(transcoder++);
    EXPECT_EQ( test, U"abcd\u016B" );
}


TEST( Unicode, transcoder_convert_as_generator )
{
    unicode::Transcoder transcoder{ "Wololo\u016Blo" };
    std::u32string test;
    test.resize( 9 );
    std::generate_n( test.begin(), transcoder.length(), transcoder );
    EXPECT_EQ( test, U"Wololo\u016Blo" );
}
