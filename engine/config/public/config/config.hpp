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

    std::pmr::string m_name{};
    std::pmr::string m_value{};
    std::pmr::vector<Entry> m_data{};

    const Entry& operator [] ( std::string_view ) const;
    const Entry* begin() const;
    const Entry* end() const;

    std::string_view toString() const;
    std::pmr::u32string toString32() const;

    template <typename T = int>
    requires std::is_integral_v<T>
    T toInt() const
    {
        if constexpr ( std::is_same_v<T, bool> ) {
            return toInt<int>() != 0;
        }
        else {
            T t{};
            std::from_chars( m_value.c_str(), m_value.c_str() + m_value.size(), t );
            return t;
        }
    }

    float toFloat() const;

    std::string_view name() const;
};

}
