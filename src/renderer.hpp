#pragma once

#include <memory_resource>

class Renderer {
public:
    Renderer();

    std::pmr::memory_resource* allocator();
    void push( void* buffer, void* constant );

    void clear();
    void present();
    void setViewportSize( uint32_t w, uint32_t h );

};
