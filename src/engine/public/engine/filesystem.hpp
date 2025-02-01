#pragma once

#include <cstdint>
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

    using Callback = std::function<void( const Asset& )>;
    std::pmr::list<std::pair<std::pmr::string, Callback>> m_callbacks{};

public:
    ~Filesystem() noexcept;
    Filesystem() noexcept;

    void mount( const std::filesystem::path& );
    void setCallback( std::string_view, Callback&& );
    std::span<const uint8_t> viewWait( std::string_view );

};
