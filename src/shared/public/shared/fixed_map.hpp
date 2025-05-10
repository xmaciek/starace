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

template <typename TKey, typename TValue>
class FixedMapView {
    using difference_type = std::intptr_t;

    std::span<TKey> m_keys{};
    std::span<TValue> m_values{};

public:
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
        auto begin = m_keys.begin();
        auto end = m_keys.end();
        assert( std::is_sorted( begin, end ) );
        auto it = std::lower_bound( begin, end, k );
        if ( it == end ) [[unlikely]] { return nullptr; }
        if ( *it != k ) [[unlikely]] { return nullptr; }
        const difference_type dist = it - begin;
        auto ptr = &*m_values.begin();
        return ptr + dist;
    }

    TValue* find( const TKey& k ) noexcept
    {
        auto begin = m_keys.begin();
        auto end = m_keys.end();
        assert( std::is_sorted( begin, end ) );
        auto it = std::lower_bound( begin, end, k );
        if ( it == end ) [[unlikely]] { return nullptr; }
        if ( *it != k ) [[unlikely]] { return nullptr; }
        const difference_type dist = it - begin;
        auto ptr = &*m_values.begin();
        return ptr + dist;
    }

    std::tuple<const TValue*, const TValue*> equalRange( const TKey& k ) const noexcept
    {
        auto begin = m_keys.begin();
        auto end = m_keys.end();
        assert( std::is_sorted( begin, end ) );
        auto [ b, e ] = std::equal_range( begin, end, k );
        if ( b == e || b == end ) [[unlikely]] { return {}; }
        const difference_type diffBegin = b - begin;
        const difference_type diffEnd = e - begin;
        auto ptr = &*m_values.begin();
        return std::make_tuple<const TValue*, const TValue*>( ptr + diffBegin, ptr + diffEnd );
    }


    std::tuple<TValue*, TValue*> equalRange( const TKey& k ) noexcept
    {
        auto begin = m_keys.begin();
        auto end = m_keys.end();
        assert( std::is_sorted( begin, end ) );
        auto [ b, e ] = std::equal_range( begin, end, k );
        if ( b == e || b == end ) [[unlikely]] { return {}; }
        const difference_type diffBegin = b - begin;
        const difference_type diffEnd = e - begin;
        auto ptr = &*m_values.begin();
        return std::make_tuple<TValue*, TValue*>( ptr + diffBegin, ptr + diffEnd );
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

    using size_type = std::size_t;
    using difference_type = std::intptr_t;
    using TKeyStorage = std::conditional_t<isTrivial<TKey>(), TKey, std::aligned_storage_t<sizeof(TKey), alignof(TKey)>>;
    using TValueStorage = std::conditional_t<isTrivial<TValue>(), TValue, std::aligned_storage_t<sizeof(TValue), alignof(TValue)>>;
    std::array<TKeyStorage, TCapacity> m_keyList{};
    std::array<TValueStorage, TCapacity> m_valueList{};
    size_type m_currentSize = 0;

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
        if constexpr ( !isTrivial<TValue>() ) { std::destroy( valueBegin(), valueEnd() ); }
        if constexpr ( !isTrivial<TKey>() ) { std::destroy( keyBegin(), keyEnd() ); }
    }

    FixedMap() noexcept = default;

    FixedMap( const FixedMap& ) noexcept requires ( isTrivial<TKey>() && isTrivial<TValue>() ) = default;
    FixedMap& operator = ( const FixedMap& ) noexcept requires ( isTrivial<TKey>() && isTrivial<TValue>() ) = default;


    FixedMapView<TKey, TValue> makeView()
    {
        return { std::span<TKey>{ keyBegin(), keyEnd() }, std::span<TValue>{ valueBegin(), valueEnd() } };
    }

    FixedMapView<const TKey, const TValue> makeView() const
    {
        return { std::span<TKey>{ keyBegin(), keyEnd() }, std::span<TValue>{ valueBegin(), valueEnd() } };
    }

    size_type size() const noexcept
    {
        return m_currentSize;
    }

    const TValue* find( const TKey& k ) const noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        const difference_type dist = it - keyBegin();
        return valueBegin() + dist;
    }

    TValue* find( const TKey& k ) noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        const difference_type dist = it - keyBegin();
        return valueBegin() + dist;
    }

    std::tuple<TValue*, TValue*> equalRange( const TKey& k ) noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto [ begin, end ] = std::equal_range( keyBegin(), keyEnd(), k );
        if ( begin == end || begin == keyEnd() ) { return {}; }
        const difference_type diffBegin = begin - keyBegin();
        const difference_type diffEnd = end - keyBegin();
        return { valueBegin() + diffBegin, valueBegin() + diffEnd };
    }

    std::tuple<const TValue*, const TValue*> equalRange( const TKey& k ) const noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto [ begin, end ] = std::equal_range( keyBegin(), keyEnd(), k );
        if ( begin == end || begin == keyEnd() ) { return {}; }
        const difference_type diffBegin = begin - keyBegin();
        const difference_type diffEnd = end - keyBegin();
        return { valueBegin() + diffBegin, valueBegin() + diffEnd };
    }

    void pushBack( const TKey& k, const TValue& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        assert( m_currentSize == 0 || *reinterpret_cast<TKey*>(&m_keyList[ m_currentSize - 1 ]) < k );
        new ( &m_keyList[ m_currentSize ] ) TKey ( k );
        new ( &m_valueList[ m_currentSize ] ) TValue ( v );
        m_currentSize += 1;
    }

    void pushBack( TKey&& k, TValue&& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        assert( m_currentSize == 0 || *reinterpret_cast<TKey*>(&m_keyList[ m_currentSize - 1 ]) < k );
        new ( &m_keyList[ m_currentSize ] ) TKey ( k );
        new ( &m_valueList[ m_currentSize ] ) TValue ( v );
        m_currentSize += 1;
    }

    void insert( TKey&& k, TValue&& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        const auto oldEnd = keyEnd();
        auto it = std::lower_bound( keyBegin(), oldEnd, k );
        new ( &m_keyList[ m_currentSize ] ) TKey( k );
        new ( &m_valueList[ m_currentSize ] ) TValue( v );
        m_currentSize += 1;
        if ( it == oldEnd ) {
            return;
        }

        const difference_type dist = it - keyBegin();
        std::rotate( keyBegin() + dist, keyEnd() - 1, keyEnd() );
        std::rotate( valueBegin() + dist, valueEnd() - 1, valueEnd() );
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
    }

    void insert( const TKey& k, const TValue& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        const auto oldEnd = keyEnd();
        auto it = std::lower_bound( keyBegin(), oldEnd, k );
        new ( &m_keyList[ m_currentSize ] ) TKey( k );
        new ( &m_valueList[ m_currentSize ] ) TValue( v );
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
