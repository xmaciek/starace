#pragma once

#include "vk.hpp"
#include "device.hpp"
#include "image.hpp"

class RenderPass {
public:
    bool m_depthOnly : 1 = false;
    bool m_hasVRS : 1 = false;
    bool m_vrs : 1 = false;
    enum Purpose {
        eColor,
        eDepth,
    };

    RenderPass() noexcept = default;
    RenderPass( const Device&, Purpose ) noexcept;
    ~RenderPass() noexcept = default;

    RenderPass( const RenderPass& ) = delete;
    RenderPass& operator = ( const RenderPass& ) = delete;
    RenderPass( RenderPass&& ) noexcept = default;
    RenderPass& operator = ( RenderPass&& ) noexcept = default;

    void begin( VkCommandBuffer, const Image& depth, const Image& color ) noexcept;
    void resume( VkCommandBuffer, const Image& depth, const Image& color ) noexcept;
    void end( VkCommandBuffer ) noexcept;

    inline void enableVRS( bool b ) noexcept { m_vrs = b; }
};
