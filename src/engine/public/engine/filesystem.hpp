#pragma once

#include <atomic>
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

class Filesystem {
private:
    std::mutex m_bottleneck;

    std::pmr::list<std::pmr::vector<uint8_t>> m_blobs{};
    std::pmr::map<std::filesystem::path, std::span<const uint8_t>> m_blobView{};

    std::atomic<bool> m_isRunning = true;

    using Callback = std::function<void( std::string_view, std::span<const uint8_t> )>;
    std::pmr::list<std::pair<std::pmr::string, Callback>> m_callbacks{};

public:
    ~Filesystem() noexcept;
    Filesystem() noexcept;

    void mount( const std::filesystem::path& );
    void setCallback( std::string_view, Callback&& );
    std::span<const uint8_t> viewWait( const std::filesystem::path& );

};
