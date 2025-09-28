#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <list>
#include <map>
#include <memory_resource>
#include <mutex>
#include <vector>
#include <span>
#include <functional>
#include <string>
#include <string_view>

struct Asset {
    std::string_view path{};
    std::span<const uint8_t> data{};

    template <typename T>
    requires ( std::is_trivially_copyable_v<T> )
    bool read( T& t )
    {
        if ( data.size() < sizeof( T ) ) return false;
        std::memcpy( &t, data.data(), sizeof( T ) );
        data = data.subspan( sizeof( T ) );
        return true;
    }
};

class Filesystem {
private:
    std::mutex m_bottleneckFs;
    std::mutex m_bottleneckCb;
    struct Mount {
        std::pmr::vector<uint8_t> m_blob{};
        std::pmr::map<std::pmr::string, std::span<uint8_t>> m_toc{};
    };
    std::pmr::list<Mount> m_mounts{};

    using Callback = std::function<void( Asset&& )>;
    std::pmr::list<std::pair<std::pmr::string, Callback>> m_callbacks{};

public:
    ~Filesystem() noexcept;
    Filesystem() noexcept;

    void mount( const std::filesystem::path& );
    void setCallback( std::string_view, Callback&& );
    inline void setCallback( std::string_view ext, auto* ptr, auto&& memFn )
    {
        setCallback( ext, [ptr, memFn]( auto&& data )
        {
            std::invoke( memFn, ptr, std::forward<decltype(data)>( data ) );
        } );
    }
    std::span<const uint8_t> viewWait( std::string_view );

};
