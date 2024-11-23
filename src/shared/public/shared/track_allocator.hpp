#pragma once

#include <profiler.hpp>

#include <cstddef>
#include <memory_resource>

template <bool b = true>
class TrackAllocator : std::pmr::memory_resource {

public:
    TrackAllocator() noexcept
    {
        if constexpr ( b ) {
            std::pmr::set_default_resource( this );
        }
    }
    ~TrackAllocator() noexcept
    {
        if constexpr ( b ) {
            std::pmr::set_default_resource( std::pmr::new_delete_resource() );
        }
    };

    virtual void* do_allocate( std::size_t size, std::size_t align ) override
    {
        void* ptr = std::pmr::new_delete_resource()->allocate( size, align );
        TracyAlloc( ptr, size );
        return ptr;
    }

    virtual void do_deallocate( void* ptr, std::size_t size, std::size_t align ) override
    {
        TracyFree( ptr );
        return std::pmr::new_delete_resource()->deallocate( ptr, size, align );
    }

    virtual bool do_is_equal( const std::pmr::memory_resource& rhs ) const noexcept override
    {
        return this == &rhs;
    }
};