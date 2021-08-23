#pragma once

#include "render_context.hpp"
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <renderer/renderer.hpp>

#include <glm/vec3.hpp>

#include <memory_resource>
#include <cstdint>
#include <vector>

class Model {
private:
    std::pmr::vector<float> m_model{};
    mutable Buffer m_vertices{};
    Texture m_textureID{};
    std::vector<glm::vec3> m_thrusters{};
    std::array<glm::vec3, 3> m_weapons{};
    float m_scale = 1.0f;

public:
    ~Model();
    Model() = default;
    Model( const char*, Texture, Renderer*, float scale = 1.0f );

    glm::vec3 weapon( uint32_t ) const;
    std::vector<glm::vec3> thrusters() const;
    void bindTexture( Texture );
    void render( RenderContext ) const;
    void loadOBJ( const char* filename );
    void scale( float scale );
};
