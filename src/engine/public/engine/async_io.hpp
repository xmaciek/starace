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

// TODO Not much of an async left, rename to something more fitting new desing
class AsyncIO {
private:
    std::mutex m_bottleneck;

    std::pmr::list<std::pmr::vector<uint8_t>> m_blobs{};
    std::pmr::map<std::filesystem::path, std::span<const uint8_t>> m_blobView{};

    std::atomic<bool> m_isRunning = true;

public:
    ~AsyncIO() noexcept;
    AsyncIO() noexcept;

    void mount( const std::filesystem::path& );
    std::span<const uint8_t> viewWait( const std::filesystem::path& );

};
