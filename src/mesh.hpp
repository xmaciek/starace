#pragma once

#include <engine/math.hpp>
#include <renderer/buffer.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <string>
#include <string_view>
#include <unordered_map>
#include <span>

class Renderer;

class Mesh {
    Renderer* m_renderer = nullptr;
    // TODO revisit with complying map __cpp_lib_generic_unordered_lookup, or drop key altogether
    std::pmr::unordered_map<std::pmr::string, Buffer> m_map{};
public:
    std::array<math::vec3, 3> m_hardpoints{}; // todo
    std::array<math::vec3, 2> m_thrusters{}; // todo
    uint32_t m_thrusterCount = 0;

    ~Mesh() noexcept;
    Mesh() noexcept = default;
    Mesh( std::span<const uint8_t>, Renderer* ) noexcept;

    Mesh( const Mesh& ) = delete;
    Mesh& operator = ( const Mesh& ) = delete;
    Mesh( Mesh&& ) noexcept = default;
    Mesh& operator = ( Mesh&& ) noexcept = default;

    Buffer operator [] ( std::string_view ) const noexcept;
};
