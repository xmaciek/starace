#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <type_traits>
#include <utility>

template <typename T>
class UniquePointer {

    template <typename U>
    friend class UniquePointer;

    using pointer = T*;

    pointer m_ptr = nullptr;
    std::pmr::memory_resource* m_allocator = nullptr;
    std::size_t m_bytes = 0;
    std::size_t m_align = 0;

    void destroy()
    {
        if ( m_allocator && m_ptr && m_bytes && m_align ) {
            std::destroy_at<T>( m_ptr );
            m_allocator->deallocate( m_ptr, m_bytes, m_align );
        }
        else if ( m_allocator || m_ptr || m_bytes || m_align ) {
            assert( !"here be dragons" );
        }
    }

public:
    ~UniquePointer() noexcept
    {
        destroy();
    }

    UniquePointer() noexcept = default;

    UniquePointer( std::pmr::memory_resource* alloc, auto&& ... args ) noexcept
    : m_allocator{ alloc }
    , m_bytes{ sizeof( T ) }
    , m_align{ alignof( T ) }
    {
        void* blob = m_allocator->allocate( m_bytes, m_align );
        assert( blob );
        m_ptr = std::construct_at( reinterpret_cast<pointer>( blob )
            , std::forward<decltype(args)>( args )...
        );
    }


    UniquePointer( UniquePointer&& rhs ) noexcept
    {
        std::swap( m_ptr, rhs.m_ptr );
        std::swap( m_allocator, rhs.m_allocator );
        std::swap( m_bytes, rhs.m_bytes );
        std::swap( m_align, rhs.m_align );
    }

    UniquePointer& operator = ( UniquePointer&& rhs ) noexcept
    {
        std::swap( m_ptr, rhs.m_ptr );
        std::swap( m_allocator, rhs.m_allocator );
        std::swap( m_bytes, rhs.m_bytes );
        std::swap( m_align, rhs.m_align );
        return *this;
    }

    template <typename U>
    requires ( std::is_base_of_v<T, U> )
    UniquePointer( UniquePointer<U>&& rhs ) noexcept
    {
        m_ptr = std::exchange( rhs.m_ptr, {} );
        m_allocator = std::exchange( rhs.m_allocator, {} );
        m_bytes = std::exchange( rhs.m_bytes, {} );
        m_align = std::exchange( rhs.m_align, {} );
    }

    template <typename U>
    requires ( std::is_base_of_v<T, U> )
    UniquePointer& operator = ( UniquePointer<U>&& rhs ) noexcept
    {
        destroy();
        m_ptr = std::exchange( rhs.m_ptr, {} );
        m_allocator = std::exchange( rhs.m_allocator, {} );
        m_bytes = std::exchange( rhs.m_bytes, {} );
        m_align = std::exchange( rhs.m_align, {} );
        return *this;
    }


    template <typename U>
    requires ( !std::is_base_of_v<T, U> )
    UniquePointer( UniquePointer<U>&& rhs ) = delete;

    template <typename U>
    requires ( !std::is_base_of_v<T, U> )
    UniquePointer& operator = ( UniquePointer<U>&& rhs ) = delete;

    template <typename U = T>
    UniquePointer( const UniquePointer<U>& ) = delete;
    UniquePointer( const UniquePointer& ) = delete;

    template <typename U = T>
    UniquePointer& operator = ( const UniquePointer<U>& ) = delete;
    UniquePointer& operator = ( const UniquePointer& ) = delete;


    operator bool () const noexcept
    {
        return m_ptr;
    }

    pointer operator -> () const noexcept
    {
        return m_ptr;
    }

    T& operator * () noexcept
    {
        return *m_ptr;
    }
    const T& operator * () const noexcept
    {
        return *m_ptr;
    }
    pointer get() const noexcept
    {
        return m_ptr;
    }
};
