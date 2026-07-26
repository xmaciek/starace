#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <tuple>
#include <span>

namespace {
namespace detail {

template <typename TKey, typename TValue>
inline TValue* find( std::span<TKey> keys, std::span<TValue> values, auto k )
{
    assert( std::ranges::is_sorted( keys ) );
    auto it = std::ranges::lower_bound( keys, k );
    if ( it == keys.end() ) [[unlikely]] { return nullptr; }
    if ( *it != k ) [[unlikely]] { return nullptr; }
    auto dist = it - keys.begin();
    auto ptr = &*values.begin();
    return ptr + dist;
}

template <typename TKey, typename TValue>
inline std::tuple<TValue*, TValue*> equalRange( std::span<TKey> keys, std::span<TValue> values, auto k  )
{
    assert( std::ranges::is_sorted( keys ) );
    auto [ b, e ] = std::ranges::equal_range( keys, k );
    if ( b == e || b == keys.end() ) [[unlikely]] { return {}; }
    auto diffBegin = b - keys.begin();
    auto diffEnd = e - keys.begin();
    auto ptr = &*values.begin();
    return std::make_tuple<TValue*, TValue*>( ptr + diffBegin, ptr + diffEnd );
}

}
}

template <typename TKey, typename TValue>
class FixedMapView {
public:
    using difference_type = std::intptr_t;

    std::span<TKey> m_keys{};
    std::span<TValue> m_values{};

    ~FixedMapView() noexcept = default;
    FixedMapView() noexcept = default;
    FixedMapView( std::span<TKey> keys, std::span<TValue> values ) noexcept
    : m_keys{ keys }
    , m_values{ values }
    {
        assert( std::is_sorted( m_keys.begin(), m_keys.end() ) );
        assert( m_keys.size() == m_values.size() );
    }

    const TValue* find( const TKey& k ) const noexcept
    {
        return detail::find( m_keys, m_values, k );
    }

    TValue* find( const TKey& k ) noexcept
    {
        return detail::find( m_keys, m_values, k );
    }

    std::tuple<const TValue*, const TValue*> equalRange( const TKey& k ) const noexcept
    {
        return detail::equalRange( m_keys, m_values, k );
    }


    std::tuple<TValue*, TValue*> equalRange( const TKey& k ) noexcept
    {
        return detail::equalRange( m_keys, m_values, k );
    }

};

template <typename TKey, typename TValue, size_t TCapacity>
class FixedMap {
    template <typename T>
    static constexpr bool isTrivial() noexcept {
        return std::is_trivially_constructible_v<T>
        && std::is_trivially_copyable_v<T>
        && std::is_trivially_destructible_v<T>
        ;
    }

    template <typename T>
    struct AlignedStorage {
        alignas( alignof( T ) ) std::byte data[ sizeof( T ) ];
    };
    using size_type = std::size_t;
    using difference_type = std::intptr_t;
    using TKeyStorage = std::conditional_t<isTrivial<TKey>(), TKey, AlignedStorage<TKey>>;
    using TValueStorage = std::conditional_t<isTrivial<TValue>(), TValue, AlignedStorage<TValue>>;
    std::array<TKeyStorage, TCapacity> m_keyList{};
    std::array<TValueStorage, TCapacity> m_valueList{};
    size_type m_currentSize = 0;

    std::span<TKey> keys() noexcept { return std::span{ reinterpret_cast<TKey*>( &m_keyList[ 0 ] ), m_currentSize }; }
    std::span<const TKey> keys() const noexcept { return std::span{ reinterpret_cast<const TKey*>( &m_keyList[ 0 ] ), m_currentSize }; }

    std::span<TValue> values() noexcept { return std::span{ reinterpret_cast<TValue*>( &m_valueList[ 0 ] ), m_currentSize }; }
    std::span<const TValue> values() const noexcept { return std::span{ reinterpret_cast<const TValue*>( &m_valueList[ 0 ] ), m_currentSize }; }

    auto keyBegin() const noexcept { return reinterpret_cast<const TKey*>( &m_keyList[ 0 ] ); }
    auto keyBegin() noexcept { return reinterpret_cast<TKey*>( &m_keyList[ 0 ] ); }
    auto keyEnd() const noexcept { return keyBegin() + m_currentSize; }
    auto keyEnd() noexcept { return keyBegin() + m_currentSize ; }
    auto valueBegin() const noexcept { return reinterpret_cast<const TValue*>( &m_valueList[ 0 ] ); }
    auto valueBegin() noexcept { return reinterpret_cast<TValue*>( &m_valueList[ 0 ] ); }
    auto valueEnd() const noexcept { return valueBegin() + m_currentSize; }
    auto valueEnd() noexcept { return valueBegin() + m_currentSize; }

public:
    static constexpr inline auto CAPACITY = TCapacity;

    ~FixedMap() noexcept
    {
        std::ranges::destroy( keys() );
        std::ranges::destroy( values() );
    }

    FixedMap() noexcept = default;

    FixedMap( const FixedMap& ) noexcept requires ( isTrivial<TKey>() && isTrivial<TValue>() ) = default;
    FixedMap& operator = ( const FixedMap& ) noexcept requires ( isTrivial<TKey>() && isTrivial<TValue>() ) = default;

    size_type size() const noexcept
    {
        return m_currentSize;
    }

    const TValue* find( const TKey& k ) const noexcept
    {
        return detail::find( keys(), values(), k );
    }

    TValue* find( const TKey& k ) noexcept
    {
        return detail::find( keys(), values(), k );
    }

    std::tuple<TValue*, TValue*> equalRange( const TKey& k ) noexcept
    {
        return detail::equalRange( keys(), values(), k );
    }

    std::tuple<const TValue*, const TValue*> equalRange( const TKey& k ) const noexcept
    {
        return detail::equalRange( keys(), values(), k );
    }

    void pushBack( const TKey& k, const TValue& v ) noexcept
    {
        pushBack2( std::forward<decltype(k)>( k ), std::forward<decltype(v)>( v ) );
    }

    void pushBack( TKey&& k, TValue&& v ) noexcept
    {
        pushBack2( std::forward<decltype(k)>( k ), std::forward<decltype(v)>( v ) );
    }

    void insert( TKey&& k, TValue&& v ) noexcept
    {
        insert2( std::forward<decltype(k)>( k ), std::forward<decltype(v)>( v ) );
    }

    void insert( const TKey& k, const TValue& v ) noexcept
    {
        insert2( std::forward<decltype(k)>( k ), std::forward<decltype(v)>( v ) );
    }

private:
    inline void pushBack2( auto k, auto v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        assert( m_currentSize == 0 || *reinterpret_cast<TKey*>(&m_keyList[ m_currentSize - 1 ]) < k );
        new ( &m_keyList[ m_currentSize ] ) TKey( std::forward<decltype(k)>( k ) );
        new ( &m_valueList[ m_currentSize ] ) TValue( std::forward<decltype(v)>( v ) );
        m_currentSize += 1;
    }

    inline void insert2( auto k, auto v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        const auto oldEnd = keyEnd();
        auto it = std::lower_bound( keyBegin(), oldEnd, k );
        new ( &m_keyList[ m_currentSize ] ) TKey( std::forward<decltype(k)>( k ) );
        new ( &m_valueList[ m_currentSize ] ) TValue( std::forward<decltype(v)>( v ) );
        m_currentSize += 1;
        if ( it == oldEnd ) {
            return;
        }

        const difference_type dist = it - keyBegin();
        std::rotate( keyBegin() + dist, keyEnd() - 1, keyEnd() );
        std::rotate( valueBegin() + dist, valueEnd() - 1, valueEnd() );
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
    }
};
