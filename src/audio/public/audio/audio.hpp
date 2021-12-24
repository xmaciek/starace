#pragma once

#include <string_view>

class Audio {
public:
    struct Chunk {
        void* data = nullptr;
    };

    virtual ~Audio() = default;
    Audio() = default;

    static Audio* create();
    virtual void play( const Chunk& ) = 0;
    virtual Chunk load( std::string_view ) = 0;
};
