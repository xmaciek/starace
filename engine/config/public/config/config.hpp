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
    static Entry fromJSON( std::string_view );
    static Entry fromJSON( std::span<const char> );
    static Entry fromJSON( std::span<const uint8_t> );

    std::pmr::string m_name{};
    std::pmr::string m_value{};
    std::pmr::vector<Entry> m_data{};

    const Entry& operator [] ( std::string_view ) const;
    const Entry* begin() const;
    const Entry* end() const;

    inline operator bool () const { return !m_name.empty() || !m_value.empty() || !m_data.empty(); }

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

    bool toValue( std::pmr::string& ) const;
    bool toValue( int32_t& ) const;
    bool toValue( uint32_t& ) const;
    bool toValue( float& ) const;

    template <typename T>
    requires ( std::is_enum_v<T> )
    bool toValue( T& t ) const
    {
        std::underlying_type_t<T> v{};
        if ( !toValue( v ) ) return false;
        t = static_cast<T>( v );
        return true;
    }

    float toFloat() const;

    inline std::span<const Entry> toArray() const { return m_data; };
    inline auto size() const { return m_data.size(); }
    std::string_view name() const;
};

}
