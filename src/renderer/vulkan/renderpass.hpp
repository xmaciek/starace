#pragma once

#include "vk.hpp"
#include "image.hpp"

class RenderPass {
    bool m_depthOnly = false;

public:
    enum Purpose {
        eColor,
        eDepth,
    };

    RenderPass() noexcept = default;
    RenderPass( Purpose ) noexcept;
    ~RenderPass() noexcept = default;

    RenderPass( const RenderPass& ) = delete;
    RenderPass& operator = ( const RenderPass& ) = delete;
    RenderPass( RenderPass&& ) noexcept = default;
    RenderPass& operator = ( RenderPass&& ) noexcept = default;

    void begin( VkCommandBuffer, const Image& depth, const Image& color ) noexcept;
    void resume( VkCommandBuffer, const Image& depth, const Image& color ) noexcept;
    void end( VkCommandBuffer ) noexcept;

};
