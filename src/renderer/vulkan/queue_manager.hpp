#pragma once

#include "vk.hpp"

#include <cstdint>
#include <tuple>
#include <array>
#include <mutex>

class QueueManager {
public:
    struct alignas( 8 ) Family {
        uint32_t index = 0;
        uint32_t count = 0;
        bool operator == ( const Family& ) const = default;
        bool operator < ( const Family& ) const;
    };

private:
    std::tuple<VkQueue, uint32_t> m_graphics{};
    std::tuple<VkQueue, uint32_t> m_present{};
    std::tuple<VkQueue, uint32_t> m_transfer{};

    std::array<std::mutex, 3> m_mutexes{};

    Family m_graphicsFamily{};
    Family m_presentFamily{};
    Family m_transferFamily{};

public:
    ~QueueManager();
    QueueManager() = default;
    QueueManager( VkPhysicalDevice, VkSurfaceKHR );
    QueueManager( const QueueManager& ) = delete;
    QueueManager( QueueManager&& );
    QueueManager& operator = ( const QueueManager& ) = delete;
    QueueManager& operator = ( QueueManager&& );

    inline uint32_t graphicsFamily() const { return m_graphicsFamily.index; }
    inline uint32_t presentFamily() const { return m_presentFamily.index; }
    inline uint32_t transferFamily() const { return m_transferFamily.index; }

    std::tuple<VkQueue, std::mutex*> graphics();
    std::tuple<VkQueue, std::mutex*> present();
    std::tuple<VkQueue, std::mutex*> transfer();

    std::tuple<std::array<VkDeviceQueueCreateInfo, 3>, uint32_t> createInfo() const;
    void acquire( VkDevice );

};
