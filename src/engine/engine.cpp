#include <engine/engine.hpp>

Engine::Engine() noexcept
: m_io{ std::make_unique<AsyncIO>() }
{
}
