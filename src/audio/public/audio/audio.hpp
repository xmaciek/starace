#pragma once

#include <string_view>

namespace audio {

struct Chunk {
    void* data = nullptr;
};

class Engine {
public:
    virtual ~Engine() = default;
    Engine() = default;

    static Engine* create();
    virtual void play( const Chunk& ) = 0;
    virtual Chunk load( std::string_view ) = 0;
};

} // namespace audio
