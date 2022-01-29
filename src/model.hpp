#pragma once

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <renderer/renderer.hpp>

#include <filesystem>
#include <memory_resource>
#include <cstdint>
#include <vector>

class Model {
private:
    Buffer m_vertices{};
    Texture m_texture{};
    std::vector<math::vec3> m_thrusters{};
    std::array<math::vec3, 3> m_weapons{};
    float m_scale = 1.0f;

    void loadOBJ( const std::filesystem::path& filename, Renderer* );
    void destroy();

public:
    ~Model();
    Model() = default;
    Model( const std::filesystem::path&, Texture, Renderer*, float scale = 1.0f );

    // TODO: make defaults when model stops owning resources
    Model( const Model& ) = delete;
    Model& operator = ( const Model& ) = delete;
    Model( Model&& ) noexcept;
    Model& operator = ( Model&& ) noexcept;

    math::vec3 weapon( uint32_t ) const;
    std::vector<math::vec3> thrusters() const;
    void render( RenderContext ) const;
    void scale( float scale );

};
