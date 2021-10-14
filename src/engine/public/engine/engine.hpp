#pragma once

#include <engine/async_io.hpp>

class Engine {
protected:
    std::unique_ptr<AsyncIO> m_io{};

public:
    ~Engine() noexcept = default;
    Engine() noexcept;

};
