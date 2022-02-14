#pragma once

#include <shared/pool.hpp>

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <list>
#include <memory_resource>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <map>

class AsyncIO {
public:
    struct Ticket {
        std::filesystem::path path{};
        std::pmr::vector<uint8_t> data{};
        std::atomic<bool> ready = false;

        Ticket( std::filesystem::path, std::pmr::memory_resource* );
    };

private:
    static constexpr std::size_t c_maxFiles = 64;
    Pool<Ticket, c_maxFiles> m_pool{};

    std::map<std::filesystem::path, Ticket*> m_localFiles{};

    std::thread m_thread;
    std::mutex m_bottleneck;
    std::pmr::list<Ticket*> m_pending{};
    std::atomic<uint16_t> m_pendingCount = 0;
    std::atomic<bool> m_isRunning = true;

    std::condition_variable m_notify;
    std::mutex m_mutex;
    std::unique_lock<std::mutex> m_uniqueLock;

    void run();
    Ticket* next();
    void finish( Ticket* );

public:
    ~AsyncIO() noexcept;
    AsyncIO() noexcept;

    void mount( const std::filesystem::path& );
    void enqueue( const std::filesystem::path&, std::pmr::memory_resource* upstream = std::pmr::get_default_resource() );
    std::optional<std::pmr::vector<uint8_t>> get( const std::filesystem::path& );
    std::pmr::vector<uint8_t> getWait( const std::filesystem::path& );
};
