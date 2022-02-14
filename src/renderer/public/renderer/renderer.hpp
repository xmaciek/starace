#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <SDL.h>

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

    virtual std::pmr::memory_resource* allocator() = 0;

    virtual void createPipeline( const PipelineCreateInfo& ) = 0;

    virtual Buffer createBuffer( std::pmr::vector<float>&& ) = 0;
    virtual void deleteBuffer( Buffer ) = 0;

    virtual Texture createTexture( const TextureCreateInfo&, std::pmr::vector<uint8_t>&& ) = 0;
    virtual void deleteTexture( Texture ) = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;
    virtual void push( const PushBuffer&, const void* constant ) = 0;
};
