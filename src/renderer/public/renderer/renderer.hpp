#pragma once

#include <cstdint>
#include <memory_resource>

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

    virtual std::pmr::memory_resource* allocator() = 0;
    virtual uint32_t createTexture( uint32_t w, uint32_t h, TextureFormat, const uint8_t* ) = 0;
    virtual void clear() = 0;
    virtual void deleteTexture( uint32_t ) = 0;
    virtual void present() = 0;
    virtual void push( void* buffer, void* constant ) = 0;
    virtual void setViewportSize( uint32_t w, uint32_t h ) = 0;
};
