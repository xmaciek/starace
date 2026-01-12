#include <gtest/gtest.h>

#include <unicode/unicode.hpp>

#include <algorithm>
#include <string>
#include <string_view>

TEST( Unicode, transcoder_length )
{
    EXPECT_EQ( unicode::Transcoder{ "test\u016B" }.length(), 5 );
}

TEST( Unicode, transcoder_convert_as_generator_utf32 )
{
    unicode::Transcoder transcoder{ "Wololo\u016Blo" };
    std::u32string test;
    test.resize( 9 );
    std::ranges::generate( test, transcoder );
    EXPECT_EQ( test, U"Wololo\u016Blo" );
}

TEST( Unicode, transcoder_convert_as_functor_utf32 )
{
    unicode::Transcoder transcoder{ "Wololo\u016Blo" };
    std::u32string test;
    test.resize( 9 );
    std::ranges::for_each( test, transcoder );
    EXPECT_EQ( test, U"Wololo\u016Blo" );
}

TEST( Unicode, transcoder_convert_as_functor_wchar )
{
    unicode::Transcoder transcoder{ "Wololo\u016Blo" };
    std::wstring test;
    test.resize( 9 );
    std::ranges::for_each( test, transcoder );
    EXPECT_EQ( test, L"Wololo\u016Blo" );
}
