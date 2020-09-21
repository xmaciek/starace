#pragma once

#include <memory_resource>

class Renderer {
public:
    std::pmr::memory_resource* allocator();
    void push( void* buffer, void* constant );
};
