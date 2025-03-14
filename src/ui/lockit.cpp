#include <ui/lockit.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

Lockit::Lockit( std::span<const uint8_t> data )
{
    lang::Header header{};
    assert( data.size() >= sizeof( header ) );
    std::memcpy( &header, data.data(), sizeof( header ) );
    data = data.subspan( sizeof( header ) );
    assert( header.magic == header.MAGIC );
    assert( header.version == header.VERSION );

    assert( data.size() >= header.count * sizeof( lang::KeyType ) );
    m_keys = { reinterpret_cast<const lang::KeyType*>( data.data() ), header.count };
    data = data.subspan( header.count * sizeof( lang::KeyType ) );

    assert( data.size() >= header.string * sizeof( char32_t ) );
    m_data = { reinterpret_cast<const char32_t*>( data.data() ), header.string };
    data = data.subspan( header.string * sizeof( char32_t ) );

    assert( data.size() == 0 );
}

std::u32string_view Lockit::find( std::string_view sv ) const
{
    return find( Hash{}( sv ) );
}

std::u32string_view Lockit::find( Hash::value_type h ) const
{
    auto it = std::lower_bound( m_keys.begin(), m_keys.end(), lang::KeyType{ h, 0, 0 } );
    if ( it == m_keys.end() || it->hash != h ) {
        assert( !"missing lockit key" );
        return U"<missing lockit key>";
    }
    assert( m_data.size() >= ( (size_t)it->offset + it->size ) );
    return std::u32string_view{ m_data.data() + it->offset, it->size };
}

}
