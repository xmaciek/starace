#pragma once

#include <charconv>
#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cfg {

class Entry {
public:
    static Entry fromData( std::span<const char> );
    static Entry fromData( std::span<const uint8_t> );
    static Entry fromData( std::pmr::vector<uint8_t>&& );

    std::pmr::string name{};
    std::pmr::string value{};
    std::pmr::vector<Entry> data{};

    const Entry& operator [] ( std::string_view ) const;
    const Entry* begin() const;
    const Entry* end() const;

    std::string_view toString() const;
    std::pmr::u32string toString32() const;

    template <typename T = int>
    requires std::is_integral_v<T>
    T toInt() const
    {
        T t = 0;
        std::from_chars( value.c_str(), value.c_str() + value.size(), t );
        return t;
    }

    template <>
    bool toInt() const
    {
        return toInt<int>() != 0;
    }

    float toFloat() const;

    std::string_view operator * () const;
};

}
