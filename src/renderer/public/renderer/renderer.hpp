#pragma once

#include <renderer/buffer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <memory_resource>
#include <vector>

enum struct TextureFormat {
    eRGB,
    eRGBA,
};

class Renderer {
protected:
    static Renderer* s_instance;

public:
    virtual ~Renderer() = default;
    Renderer() = default;

    static Renderer* create();
    static Renderer* instance();

    virtual Buffer createBuffer( std::pmr::vector<glm::vec2>&& ) = 0;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec3>&& ) = 0;
    virtual std::pmr::memory_resource* allocator() = 0;
    virtual uint32_t createTexture( uint32_t w, uint32_t h, TextureFormat, const uint8_t* ) = 0;
    virtual void clear() = 0;
    virtual void deleteBuffer( const Buffer& ) = 0;
    virtual void deleteTexture( uint32_t ) = 0;
    virtual void present() = 0;
    virtual void push( void* buffer, void* constant ) = 0;
    virtual void setViewportSize( uint32_t w, uint32_t h ) = 0;
};
