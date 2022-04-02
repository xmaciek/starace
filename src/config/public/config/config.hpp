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

    std::pmr::string name{};
    std::pmr::string value{};
    std::pmr::vector<Entry> data{};

    const Entry& operator [] ( std::string_view ) const;
    const Entry* begin() const;
    const Entry* end() const;

    std::string_view toString() const;
    int toInt() const;
    float toFloat() const;

    std::string_view operator * () const;
};

}
