#pragma once

#include "pool.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <filesystem>
#include <memory_resource>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace asyncio {

struct Ticket {
    std::filesystem::path path{};
    std::pmr::vector<uint8_t> data{};

    Ticket( std::filesystem::path&&, std::pmr::memory_resource* );
};

class Service {
    std::thread m_thread{};
    std::atomic<bool> m_isRunning = true;
    std::array<std::atomic<Ticket*>, 16> m_pending{};
    std::array<std::atomic<Ticket*>, 16> m_ready{};
    Pool<Ticket, 16> m_pool{};
    std::mutex m_bottleneck{};

    std::condition_variable m_notify;
    std::mutex m_mutex;
    std::unique_lock<std::mutex> m_uniqueLock;

    void run();
    Ticket* next();
    void finish( Ticket* );

public:
    ~Service() noexcept;
    Service() noexcept;

    void enqueue( const std::filesystem::path&, std::pmr::memory_resource* upstream = std::pmr::get_default_resource() );
    std::optional<std::pmr::vector<uint8_t>> get( const std::filesystem::path& );
    std::pmr::vector<uint8_t> getWait( const std::filesystem::path& );
};


} // namespace asyncio
