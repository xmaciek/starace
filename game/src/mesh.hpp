#pragma once

#include <math.hpp>
#include <renderer/buffer.hpp>
#include <shared/stack_vector.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <string>
#include <string_view>
#include <unordered_map>
#include <span>

class Renderer;

struct Hardpoints {
    StackVector<math::vec3, 2> primary{};
    StackVector<math::vec3, 1> secondary{};
};

class Mesh {
    Renderer* m_renderer = nullptr;
    // TODO revisit with complying map __cpp_lib_generic_unordered_lookup, or drop key altogether
    std::pmr::unordered_map<std::pmr::string, Buffer> m_map{};
public:
    Hardpoints hardpoints{};
    StackVector<math::vec3, 2> thrusterAfterglow{};

    ~Mesh() noexcept;
    Mesh() noexcept = default;
    Mesh( std::span<const uint8_t>, Renderer* ) noexcept;

    Mesh( const Mesh& ) = delete;
    Mesh& operator = ( const Mesh& ) = delete;
    Mesh( Mesh&& ) noexcept = default;
    Mesh& operator = ( Mesh&& ) noexcept = default;

    Buffer operator [] ( std::string_view ) const noexcept;
    inline auto begin() const { return m_map.begin(); }
    inline auto end() const { return m_map.end(); }
};
