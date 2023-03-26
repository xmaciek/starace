#pragma once

#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cfg {

class Entry {
public:
    static Entry fromData( std::span<const char> );
    static Entry fromData( std::pmr::vector<uint8_t>&& );

    std::pmr::string name{};
    std::pmr::string value{};
    std::pmr::vector<Entry> data{};

    const Entry& operator [] ( std::string_view ) const;
    const Entry* begin() const;
    const Entry* end() const;

    std::string_view toString() const;
    std::pmr::u32string toString32() const;
    int toInt() const;

    template <typename T>
    requires std::is_integral_v<T>
    T toInt() const
    {
        return static_cast<T>( toInt() );
    }

    float toFloat() const;

    std::string_view operator * () const;
};

}
