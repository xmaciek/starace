#pragma once

#include "texture.hpp"

#include <memory_resource>

class Renderer {
    static Renderer* s_instance;

public:
    ~Renderer();
    Renderer();

    static Renderer* instance();

    std::pmr::memory_resource* allocator();
    uint32_t createTexture( uint32_t w, uint32_t h, TextureFormat, const uint8_t* );
    void clear();
    void deleteTexture( uint32_t );
    void present();
    void push( void* buffer, void* constant );
    void setViewportSize( uint32_t w, uint32_t h );
};
