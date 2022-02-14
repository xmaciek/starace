#pragma once

#include <shared/pool.hpp>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <list>
#include <map>
#include <memory_resource>
#include <mutex>
#include <optional>
#include <semaphore>
#include <thread>
#include <vector>

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
    std::atomic<bool> m_isRunning = true;

    std::counting_semaphore<0xFFFFu> m_notify{ 0 };

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
