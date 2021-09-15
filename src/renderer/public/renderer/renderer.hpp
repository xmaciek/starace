#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <SDL2/SDL.h>

#include <cstdint>
#include <memory_resource>
#include <vector>

class Renderer {
public:
    virtual ~Renderer() = default;
    Renderer() = default;

    static Renderer* create( SDL_Window* );
    static Renderer* instance();
    static SDL_WindowFlags windowFlag();

    virtual Buffer createBuffer( std::pmr::vector<float>&&, Buffer::Lifetime ) = 0;
    virtual std::pmr::memory_resource* allocator() = 0;
    virtual Texture createTexture( uint32_t w, uint32_t h, Texture::Format, bool genMips, const uint8_t* ) = 0;
    virtual void beginFrame() = 0;
    virtual void deleteBuffer( const Buffer& ) = 0;
    virtual void deleteTexture( Texture ) = 0;
    virtual void present() = 0;
    virtual void push( const void* buffer, const void* constant ) = 0;
    virtual void submit() = 0;
};
