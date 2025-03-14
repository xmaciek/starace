#pragma once

#include <extra/lang.hpp>
#include <shared/hash.hpp>

#include <span>
#include <memory_resource>
#include <string>
#include <string_view>
#include <map>

namespace ui {

class Lockit {

    std::span<const lang::KeyType> m_keys{};
    std::span<const char32_t> m_data{};

public:
    ~Lockit() = default;
    Lockit() = default;
    Lockit( std::span<const uint8_t> );
    Lockit( const Lockit& ) = delete;
    Lockit& operator = ( const Lockit& ) = delete;
    Lockit( Lockit&& ) = default;
    Lockit& operator = ( Lockit&& ) = default;

    std::u32string_view find( std::string_view ) const;
    std::u32string_view find( Hash::value_type ) const;

};

}
